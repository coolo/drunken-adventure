// This file is placed under the public domain

// from: http://www.thingiverse.com/thing:9512


// EXAMPLES:
//   standard LEGO 2x1 tile has no pin
//      block(1,2,1/3,reinforcement=false,flat_top=true);
//   standard LEGO 2x1 flat has pin
//      block(1,2,1/3,reinforcement=true);
//   standard LEGO 2x1 brick has pin
//      block(1,2,1,reinforcement=true);
//   standard LEGO 2x1 brick without pin
//      block(1,2,1,reinforcement=false);
//   standard LEGO 2x1x5 brick has no pin and has hollow knobs
//      block(1,2,5,reinforcement=false,hollow_knob=true);


include <block-remix.scad>
include <bitmap.scad>

sphere_diameter=10.2;

//block(2,2,1,axle_hole=true, hollow_knob=false,reinforcement=true);
block(2,2,0.2); 

if (false) {
translate([knob_spacing*2,knob_spacing,block_height/2])
  rotate(a=[0,90,0])
    axle(0.6);

translate([knob_spacing*3,knob_spacing, block_height/2])
  sphere(r=sphere_diameter/2, $fa=5, $fs=0.1);

translate(v=[knob_spacing,-0.3,5])
  rotate(a=[0,90,90])
    8bit_char("A", 1, 0.3);
}