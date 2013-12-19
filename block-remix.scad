// original from: http://www.thingiverse.com/thing:5699
// edited 2/24/11 3:29 PM 

// my derivation (made around 2011-06-21)
// added hollow_knob parameter to make knobs hollow
// added circular_hole parameter to make circular holes like in standard Technic plates
// added flat_top parameter to get rid of the knobs and add a groove to the bottom, so that the blocks could be separated
// 1xN and Nx1 bricks have their no pins when reinforcement==false

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


knob_diameter=5;		//knobs on top of blocks
knob_height=1.7;
knob_spacing=8;
wall_thickness=1.45;
roof_thickness=1.05;
block_height=9.5;
pin_diameter=3;		//pin for bottom blocks with width or length of 1
post_diameter=6.65;
reinforcing_width=0.7;
axle_spline_width=2.0;
axle_diameter=5;
cylinder_precision=0.5;

/* EXAMPLES: remove one slash from the beginning of this line to comment out the examples

block(2,1,1/3,axle_hole=false,circular_hole=true,reinforcement=true,hollow_knob=true,flat_top=true);

translate([50,-10,0])
	block(1,2,1/3,axle_hole=false,circular_hole=true,reinforcement=false,hollow_knob=true,flat_top=true);

translate([10,0,0])
	block(2,2,1/3,axle_hole=false,circular_hole=true,reinforcement=true,hollow_knob=true,flat_top=true);
translate([30,0,0])
	block(2,2,1/3,axle_hole=false,circular_hole=true,reinforcement=true,hollow_knob=false,flat_top=false);
translate([50,0,0])
	block(2,2,1/3,axle_hole=false,circular_hole=true,reinforcement=true,hollow_knob=true,flat_top=false);
translate([0,20,0])
	block(3,2,2/3,axle_hole=false,circular_hole=true,reinforcement=true,hollow_knob=true,flat_top=false);
translate([20,20,0])
	block(3,2,1,axle_hole=true,circular_hole=false,reinforcement=true,hollow_knob=false,flat_top=false);
translate([40,20,0])
	block(3,2,1/3,axle_hole=false,circular_hole=false,reinforcement=false,hollow_knob=false,flat_top=false);
translate([0,-10,0])
	block(1,5,1/3,axle_hole=true,circular_hole=false,reinforcement=true,hollow_knob=false,flat_top=false);
translate([0,-20,0])
	block(1,5,1/3,axle_hole=true,circular_hole=false,reinforcement=true,hollow_knob=true,flat_top=false);
translate([0,-30,0])
	block(1,5,1/3,axle_hole=true,circular_hole=false,reinforcement=true,hollow_knob=true,flat_top=true);
//*/

module block(width,length,height,axle_hole,reinforcement) {
	overall_length=(length-1)*knob_spacing+knob_diameter+wall_thickness*2;
	overall_width=(width-1)*knob_spacing+knob_diameter+wall_thickness*2;
	union() {
		difference() {
			union() {
				// body:
				cube([overall_length,overall_width,height*block_height]);
				// knobs:
				if (flat_top != true)
				translate([knob_diameter/2+wall_thickness,knob_diameter/2+wall_thickness,0]) 
					for (ycount=[0:width-1])
						for (xcount=[0:length-1]) {
							translate([xcount*knob_spacing,ycount*knob_spacing,0])
								union() {
							//translate(v=[0,-0.3,knob_height+wall_thickness+0.+0.2]) rotate(a=[0,0,90]) 8bit_char("C", 0.4, 0.4);
								difference() {
									cylinder(r=knob_diameter/2,h=block_height*height+knob_height,$fs=cylinder_precision);
									if (hollow_knob==true)
										translate([0,0,-roof_thickness])
											cylinder(r=pin_diameter/2,h=block_height*height+knob_height+2*roof_thickness,$fs=cylinder_precision);
								}
					      }
					}
			}
if (true) {
			// hollow bottom:
			translate([wall_thickness,wall_thickness,-roof_thickness]) cube([overall_length-wall_thickness*2,overall_width-wall_thickness*2,block_height*height]);
			// flat_top -> groove around bottom
			if (flat_top == true) {
				translate([-wall_thickness/2,-wall_thickness*2/3,-wall_thickness/2])
					cube([overall_length+wall_thickness,wall_thickness,wall_thickness]);
				translate([-wall_thickness/2,overall_width-wall_thickness/3,-wall_thickness/2])
					cube([overall_length+wall_thickness,wall_thickness,wall_thickness]);
	
				translate([-wall_thickness*2/3,-wall_thickness/2,-wall_thickness/2])
					cube([wall_thickness,overall_width+wall_thickness,wall_thickness]);
				translate([overall_length-wall_thickness/3,0,-wall_thickness/2])
					cube([wall_thickness,overall_width+wall_thickness,wall_thickness]);
			}
			if (axle_hole==true)
				if (width>1 && length>1) for (ycount=[1:width-1])
					for (xcount=[1:length-1])
						translate([xcount*knob_spacing,ycount*knob_spacing,roof_thickness])  axle(height);
			if (circular_hole==true)
				if (width>1 && length>1) for (ycount=[1:width-1])
					for (xcount=[1:length-1])
						translate([xcount*knob_spacing,ycount*knob_spacing,roof_thickness])
							cylinder(r=knob_diameter/2, h=height*block_height+roof_thickness/4,$fs=cylinder_precision);
}
		}
if (true) {

		if (reinforcement==true && width>1 && length>1)
			difference() {
				for (ycount=[1:width-1])
					for (xcount=[1:length-1])
						translate([xcount*knob_spacing,ycount*knob_spacing,0]) reinforcement(height);
				for (ycount=[1:width-1])
					for (xcount=[1:length-1])
						translate([xcount*knob_spacing,ycount*knob_spacing,-roof_thickness/2]) cylinder(r=knob_diameter/2, h=height*block_height+roof_thickness, $fs=cylinder_precision);
			}
		// posts:
		if (width>1 && length>1) for (ycount=[1:width-1])
			for (xcount=[1:length-1])
				translate([xcount*knob_spacing,ycount*knob_spacing,0])
					post(height, axle_hole);

		if (reinforcement == true && width==1 && length!=1)
			for (xcount=[1:length-1])
				translate([xcount*knob_spacing,overall_width/2,0]) cylinder(r=pin_diameter/2,h=block_height*height,$fs=cylinder_precision);

		if (reinforcement == true && length==1 && width!=1)
			for (ycount=[1:width-1])
				translate([overall_length/2,ycount*knob_spacing,0]) cylinder(r=pin_diameter/2,h=block_height*height,$fs=cylinder_precision);
	}
}
}

module post(height, axle_hole=false) {
        axle_hole_height = height*block_height-roof_thickness/2-knob_height;
	difference() {
		cylinder(r=post_diameter/2, h=height*block_height-roof_thickness/2,$fs=cylinder_precision);
		translate([0,0,-roof_thickness/2])
			cylinder(r=knob_diameter/2+0.05, h=height*block_height+roof_thickness/4,$fs=cylinder_precision);
	}
	if (axle_hole == true) {
	   difference() {
	     translate([0,0,knob_height])
  		  cylinder(r=post_diameter/2,h=axle_hole_height,$fs=cylinder_precision);
	     axle(axle_hole_height);
           }
	}
}

module reinforcement(height) {
	union() {
		translate([0,0,height*block_height/2]) union() {
			cube([reinforcing_width,knob_spacing+knob_diameter+wall_thickness/2,height*block_height-roof_thickness/2],center=true);
			rotate(v=[0,0,1],a=90) cube([reinforcing_width,knob_spacing+knob_diameter+wall_thickness/2,height*block_height-roof_thickness/2], center=true);
		}
	}
}

module axle(height) {
	translate([0,0,height*block_height/2]) union() {
		cube([axle_diameter,axle_spline_width,height*block_height],center=true);
		cube([axle_spline_width,axle_diameter,height*block_height],center=true);
	}
}
