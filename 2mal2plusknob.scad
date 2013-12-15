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

knob_diameter=5;		//knobs on top of blocks
knob_height=1.6;
knob_spacing=8.0;
wall_thickness=1.45;
roof_thickness=1.05;
block_height=9.5;
pin_diameter=3;		//pin for bottom blocks with width or length of 1
post_diameter=6.5;
reinforcing_width=1.5;
axle_spline_width=2.0;
axle_diameter=5;
cylinder_precision=0.5;

block(2,2,1,axle_hole=true, hollow_knob=false,reinforcement=false);
translate([14.5,post_diameter+1.5,block_height/2-0.3])
  rotate(a=[0,90,0])
    axle(0.6);

translate([21.5, post_diameter+1.5, block_height/2-0.3])
  sphere(r=5.05, $fn=100);

