// Polyphonic DX7 from stdfaust.lib's 'dx.' prefix: needs the dx7/*.lib files
// (dx7, operator, env, lfo, pitchenv) to be bundled (flattened) in ':/Libs'.
declare name "dx7_algorithms";
declare nvoices "4";

import("stdfaust.lib");

process = dx.algorithms <: _,_;
