// DSP used by the 'app.smoke' test (tests/app_smoke.cmake): silent by default
// (gain = 0) since the application really opens the audio device.
// It imports an old library (Libs/old/music.lib, flattened as Libs/music.lib)
// and stdfaust.lib to check the session Libs folder created by FaustLive.
declare name "app_smoke";

import("music.lib");
import("stdfaust.lib");

gain = hslider("gain", 0, 0, 1, 0.01) : si.smoo;

process = osc(440) * gain <: _,_;
