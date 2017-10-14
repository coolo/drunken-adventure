radius=107.890845;

module part() {
  translate([0,0,-0.3])
    cylinder(r=radius, h=6, $fn=150);
}

delta=radius-71/2;

module leaf() {
  intersection() {
   translate([0,delta,0])
    part();
   translate([0,-delta,0])
    part();
  }
}

module fourleafs() {
translate([80,0,0])
  leaf();
translate([-71/2,0,0])
 rotate([0,0,90])
  leaf();
translate([80,80+71/2,0])
 rotate([0,0,90])
  leaf();
translate([-71/2,80+71/2,0])
  leaf();
}


module base() {
 rotate([0,0,20])
  translate([-7,0,0])
   cube([95,93,4]);
}

module quad() {
  difference() {
  base();
  fourleafs();
  }
}

scale(1.0813)
 rotate([0,0,21.5])
  quad();

translate([0,5,3])
 scale([1,1,0.5])
  rotate([0,0,21.5])
   quad();

