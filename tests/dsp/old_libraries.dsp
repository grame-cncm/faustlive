// Pre-2.0 Faust libraries (Libs/old/*.lib in faustlibraries), still installed by
// Faust and bundled (flattened) in ':/Libs' by FaustLive.
declare name "old_libraries";

import("music.lib");
import("filter.lib");
import("effect.lib");

process = osc(440) * db2linear(-20) : lowpass(2, 2000) : _ <: _,_;
