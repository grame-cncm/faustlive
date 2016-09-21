import("demo.lib");

fx_stack = 
 vgroup("[1]", sawtooth_demo) <:
 vgroup("[2]", flanger_demo) : 
 vgroup("[3]", phaser2_demo);

level_viewer(x,y) = attach(x,vgroup("[4]", spectral_level_demo(x+y))),y;

process = fx_stack : level_viewer;
