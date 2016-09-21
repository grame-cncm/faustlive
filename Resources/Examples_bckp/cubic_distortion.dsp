declare name 		"cubic_distortion";

demo = library("demo.lib");

process =   vgroup("[1]",demo.oscr_demo) : 
            vgroup("[2]",demo.cubicnl_demo) : 
            vgroup("[3]",demo.spectral_level_demo) <: _,_;
