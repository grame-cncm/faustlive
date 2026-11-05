// Regression test for negative integer constants with LLVM 23 (fixed in Faust 2.92.3,
// commit e2b2737f7): libfaust built them with ConstantInt::get(i32, int), which no longer
// truncates the value, giving invalid constants in LLVM builds without assertions.
// At optimisation level >= 1, the x86 code generator crashed on this program (trap after
// 'renamable $r8d = MOV32ri i32'), and on arm64 the slider was ignored (delay always 1).
// Same pattern as ef.echo1s (Examples/echo.dsp, quadEcho.dsp, stereoEcho.dsp).
// The default delay must not be 0, so that a wrong delay changes the output: the
// bitcode round trip of faustlive_dsp_check (which rebuilds valid constants) then fails.
declare name "llvm_x86_feedback_delay";

import("stdfaust.lib");

process = +~(de.delay(65536, int(hslider("d", 10, 0, 1000, 1)) - 1));
