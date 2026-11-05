// Minimal case of a crash of the LLVM x86 code generator (LLVM 23.1.3 and 23.1.4,
// Faust 2.92.2, any x86_64 CPU, optimisation level >= 1; fine on arm64 and at -O0):
// the compilation stops on a trap after printing 'renamable $r8d = MOV32ri i32'.
// Same pattern as ef.echo1s (Examples/echo.dsp, quadEcho.dsp, stereoEcho.dsp).
// The test is disabled on x86_64 in tests/CMakeLists.txt until it is fixed.
declare name "llvm_x86_feedback_delay";

import("stdfaust.lib");

process = +~(de.delay(65536, int(hslider("d", 0, 0, 1000, 1)) - 1));
