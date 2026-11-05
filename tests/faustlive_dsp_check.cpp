/*
  faustlive_dsp_check

  Headless check of a Faust DSP file through the same libfaust/LLVM pipeline
  as FaustLive (see FLSessionManager::createFactory and getFactoryArgv):

    1. compile with createPolyDSPFactoryFromFile() and the same argv layout
       ("-I" <source dir> [options] "-I" <Libs> "-I" <Examples>),
    2. check that every imported library resolves inside the folders FaustLive
       ships (the staged Libs/Examples folders, or the source folder) and not
       from a system-wide Faust installation,
    3. instantiate a monophonic DSP (and optionally a polyphonic one),
       run a few audio blocks and check that the outputs stay finite,
    4. write the factory as LLVM bitcode (FaustLive's SHAFolder cache format)
       and read it back, then check that the reloaded DSP computes the same
       samples.

  This program does not depend on Qt nor on an audio device.
*/

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "faust/dsp/poly-llvm-dsp.h"

// Static members required by the Faust GUI architecture (normally defined by FaustLive)
std::list<GUI*> GUI::fGuiList;
ztimedmap GUI::gTimedZoneMap;

namespace fs = std::filesystem;

namespace {

struct Options {
    std::string fFile;
    std::string fLibs;
    std::string fExamples;
    std::string fCache;
    std::vector<std::string> fFaustOptions;
    int fVoices = 0;
    int fBlocks = 16;
    int fBlockSize = 256;
    int fSampleRate = 44100;
    bool fCheckImports = true;
};

void usage(const char* name)
{
    std::fprintf(stderr,
        "usage: %s --libs <dir> [--examples <dir>] [--cache <dir>] [--voices <n>]\n"
        "          [--blocks <n>] [--no-import-check] [--faust-option <opt>]... <file.dsp>\n",
        name);
}

bool parseArgs(int argc, char* argv[], Options& opts)
{
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        auto next = [&](std::string& dst) {
            if (i + 1 >= argc) return false;
            dst = argv[++i];
            return true;
        };
        std::string val;
        if (arg == "--libs") {
            if (!next(opts.fLibs)) return false;
        } else if (arg == "--examples") {
            if (!next(opts.fExamples)) return false;
        } else if (arg == "--cache") {
            if (!next(opts.fCache)) return false;
        } else if (arg == "--voices") {
            if (!next(val)) return false;
            opts.fVoices = std::atoi(val.c_str());
        } else if (arg == "--blocks") {
            if (!next(val)) return false;
            opts.fBlocks = std::atoi(val.c_str());
        } else if (arg == "--faust-option") {
            if (!next(val)) return false;
            opts.fFaustOptions.push_back(val);
        } else if (arg == "--no-import-check") {
            opts.fCheckImports = false;
        } else if (!arg.empty() && arg[0] == '-') {
            return false;
        } else {
            opts.fFile = arg;
        }
    }
    return !opts.fFile.empty() && !opts.fLibs.empty();
}

std::string canonical(const std::string& path)
{
    std::error_code ec;
    fs::path p = fs::weakly_canonical(fs::path(path), ec);
    return ec ? path : p.string();
}

bool isInside(const std::string& file, const std::string& dir)
{
    if (dir.empty()) return false;
    std::string f = canonical(file);
    std::string d = canonical(dir);
    if (d.empty()) return false;
    if (d.back() != fs::path::preferred_separator) d += fs::path::preferred_separator;
    return f.compare(0, d.size(), d) == 0;
}

// Deterministic white noise, so that two DSP instances get the same inputs.
struct Noise {
    unsigned int fState = 12345;
    float next()
    {
        fState = fState * 1103515245u + 12345u;
        return float(int(fState >> 1)) / 2147483648.f - 0.5f;
    }
};

// Interleaving-free audio buffers for compute()
struct Buffers {
    std::vector<std::vector<FAUSTFLOAT>> fData;
    std::vector<FAUSTFLOAT*> fPtrs;
    Buffers(int channels, int frames) : fData(channels, std::vector<FAUSTFLOAT>(frames, 0)), fPtrs(channels)
    {
        for (int c = 0; c < channels; c++) fPtrs[c] = fData[c].data();
    }
};

// Run 'blocks' blocks of 'frames' samples, store all outputs in 'out'.
// Returns false if a NaN or an infinite value is produced.
bool render(dsp* d, int blocks, int frames, std::vector<FAUSTFLOAT>& out, std::string& why)
{
    Buffers in(d->getNumInputs(), frames);
    Buffers outs(d->getNumOutputs(), frames);
    Noise noise;
    out.clear();
    for (int b = 0; b < blocks; b++) {
        for (auto& chan : in.fData) {
            for (auto& s : chan) s = FAUSTFLOAT(0.25f * noise.next());
        }
        d->compute(frames, in.fPtrs.data(), outs.fPtrs.data());
        for (int c = 0; c < d->getNumOutputs(); c++) {
            for (int f = 0; f < frames; f++) {
                FAUSTFLOAT v = outs.fData[c][f];
                if (!std::isfinite(v)) {
                    why = "non finite sample at block " + std::to_string(b) + ", channel " + std::to_string(c) +
                          ", frame " + std::to_string(f);
                    return false;
                }
                out.push_back(v);
            }
        }
    }
    return true;
}

// Mirrors FLSessionManager::getFactoryArgv() for local processing: createFactory()
// passes settings == NULL there, so no "-lm"/"-poly"/"-voices"/"-group" arguments
// (those are only added for remote processing).
std::vector<std::string> factoryArgs(const Options& opts)
{
    std::vector<std::string> args;
    args.push_back("-I");
    args.push_back(fs::absolute(fs::path(opts.fFile)).parent_path().string());
    for (const auto& o : opts.fFaustOptions) args.push_back(o);
    args.push_back("-I");
    args.push_back(opts.fLibs);
    args.push_back("-I");
    args.push_back(opts.fExamples.empty() ? opts.fLibs : opts.fExamples);
    return args;
}

int fail(const std::string& file, const std::string& msg)
{
    std::fprintf(stderr, "FAIL %s: %s\n", file.c_str(), msg.c_str());
    return 1;
}

}  // namespace

int main(int argc, char* argv[])
{
    Options opts;
    if (!parseArgs(argc, argv, opts)) {
        usage(argv[0]);
        return 2;
    }

    const std::string& file = opts.fFile;
    std::vector<std::string> args = factoryArgs(opts);
    std::vector<const char*> cargs;
    for (const auto& a : args) cargs.push_back(a.c_str());
    cargs.push_back(nullptr);

    // 1. Compile, as FaustLive does
    std::string error;
    std::unique_ptr<llvm_dsp_poly_factory> factory(
        createPolyDSPFactoryFromFile(file, int(args.size()), cargs.data(), "", error, -1));
    if (!factory) {
        return fail(file, "compilation error: " + error);
    }
    dsp_factory* process = factory->fProcessFactory;

    // 2. All imports must come from the folders shipped with FaustLive
    if (opts.fCheckImports) {
        std::string srcDir = fs::absolute(fs::path(file)).parent_path().string();
        for (const auto& lib : process->getLibraryList()) {
            if (!isInside(lib, opts.fLibs) && !isInside(lib, opts.fExamples) && !isInside(lib, srcDir)) {
                return fail(file, "'" + lib + "' is not imported from the bundled libraries folder " + opts.fLibs +
                                      " (is it missing from Resources/application.qrc?)");
            }
        }
    }

    // 3. Monophonic instance (and polyphonic one if asked)
    std::unique_ptr<dsp> mono(process->createDSPInstance());
    if (!mono) return fail(file, "createDSPInstance() returned null");
    mono->init(opts.fSampleRate);
    std::vector<FAUSTFLOAT> reference;
    std::string why;
    if (!render(mono.get(), opts.fBlocks, opts.fBlockSize, reference, why)) {
        return fail(file, "mono instance: " + why);
    }

    if (opts.fVoices > 0) {
        std::unique_ptr<dsp_poly> poly(factory->createPolyDSPInstance(opts.fVoices, true, true));
        if (!poly) return fail(file, "createPolyDSPInstance() returned null");
        poly->init(opts.fSampleRate);
        poly->keyOn(0, 60, 100);
        poly->keyOn(0, 64, 100);
        std::vector<FAUSTFLOAT> polyOut;
        if (!render(poly.get(), opts.fBlocks, opts.fBlockSize, polyOut, why)) {
            return fail(file, "poly instance: " + why);
        }
        poly->keyOff(0, 60, 0);
        poly->keyOff(0, 64, 0);
    }

    // 4. Bitcode cache round trip (FaustLive SHAFolder format)
    if (!opts.fCache.empty()) {
        fs::create_directories(opts.fCache);
        std::string stem = (fs::path(opts.fCache) / fs::path(file).stem()).string();
        writePolyDSPFactoryToBitcodeFile(factory.get(), stem);
        std::string readError;
        std::unique_ptr<dsp_poly_factory> reloaded(readPolyDSPFactoryFromBitcodeFile(stem, "", readError, -1));
        if (!reloaded) return fail(file, "cannot reload bitcode: " + readError);
        std::unique_ptr<dsp> mono2(reloaded->fProcessFactory->createDSPInstance());
        if (!mono2) return fail(file, "reloaded createDSPInstance() returned null");
        mono2->init(opts.fSampleRate);
        std::vector<FAUSTFLOAT> reloadedOut;
        if (!render(mono2.get(), opts.fBlocks, opts.fBlockSize, reloadedOut, why)) {
            return fail(file, "reloaded instance: " + why);
        }
        if (reloadedOut.size() != reference.size()) return fail(file, "reloaded instance has a different size");
        for (size_t i = 0; i < reference.size(); i++) {
            double diff = std::fabs(double(reloadedOut[i]) - double(reference[i]));
            if (diff > 1e-5 * (1.0 + std::fabs(double(reference[i])))) {
                return fail(file, "reloaded instance differs at sample " + std::to_string(i));
            }
        }
    }

    std::printf("OK %s (%d in, %d out, %zu libraries)\n", file.c_str(), mono->getNumInputs(), mono->getNumOutputs(),
                process->getLibraryList().size());
    return 0;
}
