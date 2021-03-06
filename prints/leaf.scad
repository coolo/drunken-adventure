radius=107.890845;

delta=107.89-71/2;

module leaf() {
  intersection() {
   translate([0,delta,0])
    cylinder(r=radius, h=1, $fn=150);
   translate([0,-delta,0])
    cylinder(r=radius, h=1, $fn=150);
  }
}

module bigleaf() {
  intersection() {
   translate([0,delta,0])
    cylinder(r=radius+5, h=4, $fn=150);
   translate([0,-delta,0])
    cylinder(r=radius+5, h=4, $fn=150);
  }
}

bigleaf();
translate([0,0,4])
  leaf();
