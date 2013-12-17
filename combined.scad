rotate(a=[0,180,0]) {
import("2mal2plusknob-A.stl");

rotate(a=[0,180,0])
  translate(v=[-60,0,-12])
   import("2mal2plusknob-B.stl");

rotate(a=[0,180,0])
  translate(v=[-90,0,-12])
    import("2mal2plusknob-C.stl");

translate(v=[-40,0,0])
import("2mal2plusknob-D.stl");

translate(v=[-80,0,0])
  import("2mal2plusknob.stl");
}