
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

sphere_diameter=10.2;
knob_height=0;

block(2,2,1,axle_hole=false, hollow_knob=false,reinforcement=true);

// right arm
difference() {
translate([knob_spacing*2-3,knob_spacing,block_height/2-1])
  rotate(a=[0,55,0])
    axle(1.4);
translate([5,3,0])
  cube([10,10,10]);
}

// left arm
difference() {
translate([-8.5,knob_spacing,block_height/2+7])
  rotate(a=[0,125,0])
    axle(1.4);
translate([6,8,6])
  cube([10,10,10], center=true);
}


// left sphere
translate([-11,knob_spacing, 13.5])
 scale([1,10.09/9.9,1])
  sphere(r=sphere_diameter/2, $fa=5, $fs=0.1);

// right sphere
translate([26,knob_spacing, 13.5])
 scale([1,10.09/9.9,1])
  sphere(r=sphere_diameter/2, $fa=5, $fs=0.1);


// HEAD
translate([knob_spacing,knob_spacing,block_height-wall_thickness/2])
  rotate(a=[0,0,90])
    axle(1.0);

translate([knob_spacing,knob_spacing,block_height+9])
scale([1,10.09/9.9,1])
  sphere(r=sphere_diameter/2, $fa=5, $fs=0.1);
