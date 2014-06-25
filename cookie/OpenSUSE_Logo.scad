
// Module names are of the form poly_<inkscape-path-id>().  As a result,
// you can associate a polygon in this OpenSCAD program with the corresponding
// SVG element in the Inkscape document by looking for the XML element with
// the attribute id="inkscape-path-id".

// fudge value is used to ensure that subtracted solids are a tad taller
// in the z dimension than the polygon being subtracted from.  This helps
// keep the resulting .stl file manifold.
fudge = 0.1;

module poly_eye(h)
{
  scale([25.4/90, -25.4/90, 1]) union()
  {
    linear_extrude(height=h)
      polygon([[77.202967,-29.530546],[78.438488,-34.832412],[81.621047,-39.239535],[86.241745,-42.110681],[91.616207,-42.979329],[97.000282,-41.687267],[101.326746,-38.538583],[104.161998,-34.001275],[105.072437,-28.543346],[103.831291,-23.251946],[100.654357,-18.842546],[96.036035,-15.970929],[90.651197,-15.102556],[85.276809,-16.398288],[80.954408,-19.545676],[78.118344,-24.078500],[77.202967,-29.530546]]);
  }
}

module poly_body(h)
{
  scale([25.4/90, -25.4/90, 1]) union()
  {
    difference()
    {
       linear_extrude(height=h)
         polygon([[-10.266273,-60.536586],[-24.215379,-59.350737],[-40.041230,-55.982036],[-48.694995,-53.183453],[-57.862416,-49.484360],[-67.558317,-44.766493],[-77.797523,-38.911586],[-78.891273,-38.224086],[-91.117612,-29.475040],[-101.286247,-19.771549],[-109.351144,-9.145889],[-115.266273,2.369664],[-116.859746,7.289566],[-118.124433,13.875032],[-118.483477,21.528127],[-117.360023,29.650914],[-114.850127,36.702226],[-111.016273,43.369664],[-105.209081,49.941749],[-98.060500,55.093448],[-89.905431,58.641068],[-81.078773,60.400914],[-74.912509,60.536586],[-69.148307,59.830516],[-63.844380,58.314218],[-59.058939,56.019201],[-54.850196,52.976976],[-51.276363,49.219054],[-48.395651,44.776947],[-46.266273,39.682164],[-45.356587,35.675624],[-45.085839,31.387532],[-45.483822,26.953461],[-46.580332,22.508982],[-48.405162,18.189667],[-50.988107,14.131086],[-54.358963,10.468811],[-58.547523,7.338414],[-62.312129,5.386211],[-66.091656,4.076530],[-73.454160,3.175056],[-80.152408,4.214636],[-85.703773,6.775914],[-89.380736,9.885269],[-92.102195,13.640145],[-93.780693,17.892594],[-94.328773,22.494664],[-94.328773,22.525914],[-93.858258,26.467356],[-92.635817,29.933362],[-90.806728,32.898769],[-88.516273,35.338414],[-83.144627,38.608202],[-77.547523,39.713414],[-71.297523,38.650914],[-71.266323,38.650914],[-68.735073,37.525914],[-68.703873,37.494714],[-68.578873,37.401014],[-68.516373,37.369814],[-68.453873,37.338614],[-68.360173,37.307414],[-68.360173,37.276214],[-66.185112,34.892968],[-65.391423,31.776214],[-65.641423,29.963714],[-65.641423,29.932514],[-66.759097,27.726057],[-68.546250,26.124424],[-70.787240,25.254961],[-73.266423,25.245014],[-73.297623,25.245014],[-73.641373,25.307514],[-73.735073,25.338764],[-73.828773,25.370014],[-74.297523,25.526264],[-74.922523,25.713764],[-75.016223,25.745014],[-77.328723,26.088764],[-77.891223,25.901264],[-79.016223,25.276264],[-79.985700,24.168110],[-80.391223,22.307514],[-80.391223,22.276264],[-79.641223,19.901264],[-77.191285,18.093794],[-75.083704,17.489243],[-72.484973,17.401264],[-68.942355,18.116372],[-65.964105,19.641906],[-63.480664,22.031495],[-61.422473,25.338764],[-60.286476,28.865452],[-60.181784,32.554504],[-61.080249,36.166937],[-62.953723,39.463764],[-65.719214,42.033723],[-69.397250,43.745988],[-73.978523,44.551516],[-79.453723,44.401264],[-85.041824,43.104346],[-90.008369,40.562226],[-94.205416,36.869225],[-97.485023,32.119664],[-99.548126,26.811992],[-100.320140,21.334705],[-99.807408,15.992086],[-98.016273,11.088414],[-94.731414,6.128425],[-90.974850,2.455589],[-86.887435,-0.111772],[-82.610023,-1.755336],[-78.290516,-2.638954],[-74.096484,-2.913698],[-66.766273,-2.286586],[-62.236383,-1.138832],[-57.855509,0.685200],[-53.686786,3.133184],[-49.793343,6.152791],[-46.238313,9.691696],[-43.084829,13.697569],[-40.396021,18.118085],[-38.235023,22.900914],[-36.672523,28.182164],[-36.141273,30.557164],[-35.985023,31.213414],[-35.360023,31.494664],[-20.953773,38.525914],[-20.922573,38.557114],[-20.891373,38.588314],[-20.485023,38.869664],[-19.953773,38.838464],[-19.610023,38.807264],[-18.485023,38.682264],[-18.328773,37.588514],[-18.266273,37.182264],[-18.235073,36.869764],[-18.328773,36.588514],[-18.516273,35.807264],[-18.891273,32.182264],[-18.797573,23.807264],[-17.635760,18.942360],[-15.203823,15.026014],[-12.040317,12.673144],[-8.703823,11.244764],[-1.490734,10.374746],[6.309327,11.497763],[10.312348,13.003679],[14.321563,15.244280],[18.290123,18.298372],[22.171177,22.244764],[31.671177,32.776014],[31.702377,32.776014],[32.171127,33.213514],[32.233627,33.276014],[32.296127,33.307214],[32.858627,33.713464],[32.921127,33.744664],[32.983627,33.807164],[33.358627,33.963414],[33.983627,34.275914],[36.077377,35.244664],[42.296127,38.119664],[51.139877,42.213414],[51.608627,42.432164],[52.077377,42.307164],[52.452377,42.213464],[53.483627,41.963464],[53.514977,40.869664],[53.514977,40.494664],[53.546177,39.932164],[53.171177,39.525914],[53.202377,39.525914],[53.108677,39.400914],[52.764927,38.932164],[51.514927,37.182164],[47.983677,31.088414],[44.879695,22.956364],[44.148521,18.658230],[44.296177,14.494664],[45.059627,11.769952],[46.202427,9.932164],[47.678351,8.746670],[49.608677,8.025914],[55.011257,7.693670],[62.077427,8.338414],[73.389927,9.244664],[81.899966,8.914204],[90.219596,7.871192],[97.866579,6.186790],[104.358677,3.932164],[110.328422,1.203645],[114.389927,-1.442836],[114.421127,-1.474086],[114.452327,-1.505336],[115.594453,-2.976444],[116.233577,-4.505336],[116.483577,-5.161586],[117.483577,-8.786586],[117.577277,-9.380336],[117.702277,-10.380336],[118.014777,-13.161586],[118.452277,-17.099086],[118.483477,-17.286586],[118.452277,-17.474086],[114.921027,-29.317836],[112.031116,-36.563081],[108.733527,-41.849086],[108.702327,-41.849086],[104.827327,-45.192836],[98.704394,-48.145754],[91.473665,-50.731174],[75.921077,-54.817836],[56.483577,-58.442836],[56.171077,-58.474086],[55.889827,-58.411586],[55.608577,-58.317836],[54.983577,-58.130336],[54.702327,-57.536586],[54.608627,-57.286586],[54.483627,-57.036586],[54.483627,-56.755336],[54.389927,-51.130336],[35.747741,-56.177715],[21.362659,-58.864523],[5.608677,-60.349086],[-10.266323,-60.536586]]);
       translate([0, 0, -fudge])
         linear_extrude(height=h+2*fudge)
           polygon([[-10.203773,-57.724086],[5.514977,-57.536586],[22.286147,-55.851733],[37.592768,-52.824630],[55.327477,-47.880336],[57.139977,-47.286586],[57.202477,-49.192836],[57.296177,-55.442836],[75.296177,-52.036586],[90.636746,-47.993591],[97.643606,-45.503382],[103.389927,-42.755336],[106.671177,-39.880336],[109.338557,-35.426403],[112.264927,-28.380336],[115.608677,-17.099086],[115.202427,-13.474086],[114.889927,-10.692836],[114.764927,-9.724086],[114.733727,-9.380336],[113.827477,-6.192836],[113.577477,-5.505336],[112.577477,-3.630336],[112.546277,-3.599086],[109.048178,-1.356274],[103.202527,1.338414],[97.117715,3.435708],[89.773790,5.045774],[81.706733,6.067785],[73.452527,6.400914],[73.421327,6.400914],[62.390077,5.494664],[55.146935,4.821909],[48.952577,5.275914],[46.295721,6.288596],[44.077577,8.057164],[42.470546,10.646556],[41.515077,14.025914],[41.515077,14.057164],[41.321217,18.886298],[42.115820,23.667287],[45.452577,32.369664],[48.733827,37.994664],[43.483827,35.557164],[37.296327,32.682164],[35.171327,31.713414],[34.577577,31.432164],[34.515077,31.400964],[34.390077,31.338464],[34.077577,31.119714],[34.046377,31.088514],[33.952677,31.026014],[33.640177,30.713514],[24.327677,20.401014],[20.180573,16.200160],[15.892512,12.926611],[11.524848,10.504223],[7.138940,8.856853],[2.796142,7.908355],[-1.442189,7.582587],[-9.360023,8.494664],[-9.391223,8.494664],[-13.505088,10.194439],[-17.203723,12.994664],[-19.021893,15.290014],[-20.295035,17.895007],[-21.609973,23.463414],[-21.899333,28.630737],[-21.734973,32.400914],[-21.734973,32.432114],[-21.422473,35.182114],[-33.516223,29.244614],[-33.922473,27.525864],[-35.578723,21.900864],[-37.880089,16.796770],[-40.742065,12.072850],[-44.103572,7.786030],[-47.903528,3.993237],[-52.080852,0.751398],[-56.574463,-1.882562],[-61.323280,-3.851715],[-66.266223,-5.099136],[-74.112762,-5.761304],[-78.656316,-5.451544],[-83.391223,-4.474136],[-88.141209,-2.635305],[-92.709895,0.242971],[-96.900496,4.317381],[-100.516223,9.744614],[-102.575884,15.350623],[-103.162105,21.379199],[-102.292573,27.512357],[-99.984973,33.432114],[-96.313485,38.735325],[-91.593870,42.883847],[-86.015119,45.751815],[-79.766223,47.213364],[-73.751813,47.362253],[-68.494695,46.418391],[-64.107716,44.372015],[-60.703723,41.213364],[-58.432567,37.229706],[-57.352072,32.862852],[-57.494276,28.390004],[-58.891223,24.088364],[-61.324122,20.275247],[-64.382429,17.397829],[-68.031070,15.490679],[-72.234973,14.588364],[-72.266173,14.588364],[-75.553605,14.676511],[-78.347410,15.446249],[-80.553722,16.747982],[-82.078673,18.432114],[-82.078673,18.463364],[-82.109873,18.463364],[-82.792234,19.872658],[-83.234873,22.244614],[-83.234873,22.275864],[-83.234873,22.307114],[-82.463864,25.463613],[-80.734873,27.525864],[-78.891123,28.557114],[-77.484873,28.900864],[-77.391173,28.932064],[-77.328673,28.932064],[-74.234923,28.463314],[-74.172423,28.463314],[-74.109923,28.432114],[-73.422423,28.213364],[-73.078673,28.088364],[-72.922423,28.057164],[-72.766173,28.025964],[-72.734973,28.025964],[-70.020549,28.514618],[-68.359973,30.682214],[-68.359973,30.713414],[-68.203723,31.775914],[-68.657284,33.535661],[-69.922473,34.900914],[-69.953673,34.932114],[-69.984873,34.963314],[-70.016073,34.963314],[-70.235023,35.119664],[-72.110023,35.932164],[-72.141223,35.963364],[-77.516223,36.900864],[-77.547423,36.900864],[-82.101085,35.946267],[-86.641173,33.182114],[-90.093573,28.703939],[-91.117742,25.820967],[-91.516173,22.494614],[-91.516173,22.463364],[-91.053259,18.581333],[-89.635952,14.997887],[-87.315005,11.819055],[-84.141173,9.150864],[-79.355873,6.955463],[-73.432910,6.030098],[-66.832828,6.813554],[-60.016173,9.744614],[-56.246782,12.572572],[-53.219148,15.890732],[-50.903333,19.568973],[-49.269397,23.477172],[-48.287401,27.485207],[-47.927408,31.462958],[-48.159478,35.280301],[-48.953673,38.807114],[-50.886855,43.473270],[-53.479536,47.526850],[-56.694994,50.942928],[-60.496507,53.696578],[-64.847353,55.762873],[-69.710810,57.116887],[-75.050158,57.733693],[-80.828673,57.588364],[-89.054499,55.941460],[-96.659945,52.625581],[-103.318506,47.830155],[-108.703673,41.744614],[-112.281819,35.510803],[-114.609923,28.963364],[-115.625672,21.416511],[-115.268013,14.192997],[-114.089122,7.971853],[-112.641173,3.432114],[-106.955958,-7.637731],[-99.174199,-17.888547],[-89.313427,-27.291533],[-77.391173,-35.817886],[-76.328673,-36.505386],[-76.297473,-36.505386],[-66.225778,-42.269696],[-56.716243,-46.910451],[-47.746356,-50.544862],[-39.293605,-53.290139],[-23.849464,-56.582139],[-10.203723,-57.724136]]);
    }
  }
}

module poly_mouth(h)
{
  scale([25.4/90, -25.4/90, 1]) union()
  {
    linear_extrude(height=h)
      polygon([[114.928977,-6.701556],[113.908977,-6.513556],[106.103602,-2.956556],[100.730024,-1.432493],[95.046977,-0.740556],[91.140415,-1.091499],[86.651665,-2.146727],[77.166227,-5.506931],[64.833977,-11.184556],[64.645977,-11.404556],[60.741977,-20.647556],[70.493024,-14.688649],[79.344352,-10.246806],[87.266243,-7.339087],[94.228977,-5.982556],[101.001883,-6.342868],[106.777477,-8.136056],[111.352321,-10.508993],[114.522977,-12.608556],[115.820977,-13.449556],[114.928977,-6.701556]]);
  }
}

module poly_rand(h)
{
  scale([25.4/90, -25.4/90, 1]) union()
  {
    difference()
    {
       linear_extrude(height=h)
         polygon([[-6.199878,-64.580935],[-13.684272,-64.313128],[-21.146097,-63.624145],[-28.563242,-62.521946],[-35.913597,-61.014492],[-43.175051,-59.109744],[-50.325495,-56.815662],[-57.342817,-54.140208],[-64.204908,-51.091342],[-70.889657,-47.677025],[-77.374954,-43.905217],[-83.638689,-39.783879],[-89.658751,-35.320972],[-95.413030,-30.524456],[-100.879416,-25.402293],[-106.035799,-19.962443],[-110.860068,-14.212866],[-113.218028,-10.724252],[-115.336403,-7.038663],[-117.200716,-3.185012],[-118.796494,0.807783],[-120.109264,4.910807],[-121.124550,9.095145],[-121.827879,13.331881],[-122.204776,17.592099],[-122.240767,21.846884],[-121.921378,26.067321],[-121.232136,30.224493],[-120.158565,34.289485],[-118.686191,38.233382],[-116.800541,42.027268],[-114.487140,45.642228],[-111.731514,49.049345],[-108.788789,52.108388],[-105.554431,54.858839],[-102.065588,57.290096],[-98.359405,59.391559],[-94.473030,61.152626],[-90.443610,62.562698],[-86.308292,63.611174],[-82.104222,64.287453],[-77.868548,64.580935],[-73.638416,64.481019],[-69.450973,63.977105],[-65.343366,63.058591],[-61.352742,61.714878],[-57.516248,59.935365],[-53.871031,57.709450],[-50.454238,55.026535],[-44.932965,44.719617],[-40.691508,37.692255],[-38.182953,36.858046],[-35.811746,36.467756],[-33.573469,36.449505],[-31.463701,36.731412],[-27.612020,37.908170],[-24.221348,39.422982],[-21.256330,40.700797],[-18.681614,41.166564],[-17.529570,40.915227],[-16.461844,40.245233],[-15.474017,39.084703],[-14.561668,37.361755],[-15.023980,30.168864],[-14.911578,26.426760],[-14.351133,22.859603],[-13.178391,19.671431],[-11.229100,17.066283],[-9.911919,16.046106],[-8.339006,15.248199],[-6.489830,14.698068],[-4.343858,14.421217],[0.276612,14.359140],[4.693037,15.200109],[8.876715,16.819877],[12.798944,19.094198],[16.431020,21.898825],[19.744242,25.109513],[22.709907,28.602015],[25.299312,32.252085],[32.328478,37.262573],[36.267544,39.943121],[40.365717,42.355393],[44.530085,44.210532],[48.667736,45.219677],[50.697508,45.316734],[52.685759,45.093971],[54.620875,44.515280],[56.491242,43.544555],[56.759780,41.596883],[56.683597,39.715604],[55.708632,36.113882],[53.989465,32.662703],[51.949215,29.285382],[50.011002,25.905235],[48.597944,22.445576],[48.220574,20.661966],[48.133161,18.829721],[48.388598,16.939256],[49.039772,14.980985],[50.078769,13.718364],[51.223220,12.769089],[53.784029,11.673110],[56.633277,11.418102],[59.682042,11.729122],[66.022443,12.949469],[69.136236,13.308908],[72.093862,13.134599],[77.561820,13.206961],[83.057433,12.965204],[88.539316,12.383838],[93.966085,11.437369],[99.296354,10.100305],[104.488738,8.347156],[109.501852,6.152427],[114.294312,3.490629],[116.131144,1.990411],[117.695809,0.334687],[120.069494,-3.376210],[121.537076,-7.507926],[122.220264,-11.926326],[122.240767,-16.497274],[121.720295,-21.086634],[120.780557,-25.560270],[119.543262,-29.784046],[118.084861,-33.485214],[116.343887,-37.101833],[114.301594,-40.554029],[111.939236,-43.761929],[109.238067,-46.645659],[106.179341,-49.125344],[102.744311,-51.121110],[98.914232,-52.553083],[89.165844,-55.466866],[79.328006,-58.155830],[69.386696,-60.354314],[59.327892,-61.796656],[56.603383,-61.606545],[54.195639,-60.864025],[49.911104,-58.749953],[47.824641,-57.892502],[45.635600,-57.510838],[43.239146,-57.862013],[40.530442,-59.203075],[28.995193,-61.638816],[17.333980,-63.384352],[5.588418,-64.383715],[-6.199878,-64.580935]]);
       translate([0, 0, -fudge])
         linear_extrude(height=h+2*fudge)
           polygon([[-3.824878,-53.799685],[3.976980,-53.685661],[11.741381,-53.130629],[19.465693,-52.185557],[27.147287,-50.901410],[42.371799,-47.519766],[57.393872,-43.393435],[58.816986,-46.606231],[60.003862,-49.272364],[60.819587,-50.106202],[61.949304,-50.450298],[63.517362,-50.186961],[65.648112,-49.198496],[68.921294,-47.106607],[72.971066,-45.191670],[82.069809,-41.493532],[86.453495,-39.510776],[90.283201,-37.305855],[93.226284,-34.778991],[94.261391,-33.363650],[94.950102,-31.830406],[95.561831,-27.835504],[95.807455,-24.093494],[95.713304,-20.603938],[95.305708,-17.366401],[94.610995,-14.380445],[93.655495,-11.645635],[92.465538,-9.161533],[91.067454,-6.927704],[87.752218,-3.209114],[83.920424,-0.486374],[79.782707,1.244009],[75.549702,1.985528],[69.520104,2.454115],[66.507862,2.478058],[57.476942,1.202763],[55.049501,0.677312],[52.700051,0.531545],[50.446172,0.738149],[48.305448,1.269809],[46.295459,2.099209],[44.433787,3.199037],[41.225722,6.100713],[38.821907,9.756320],[37.362994,13.947343],[36.989638,18.455265],[37.253998,20.759776],[37.842492,23.061569],[38.361051,26.589514],[38.138803,27.428517],[37.647480,27.740676],[36.926041,27.593341],[36.013444,27.053862],[33.770607,25.067877],[28.705195,19.353617],[24.944562,14.908347],[20.273438,10.900451],[14.958520,7.668864],[9.184748,5.308305],[3.137061,3.913490],[-2.999600,3.579138],[-6.043503,3.839235],[-9.040297,4.399967],[-11.966865,5.273174],[-14.800089,6.470696],[-17.516853,8.004372],[-20.094038,9.886042],[-28.455633,16.613860],[-41.965188,28.108427],[-55.396712,38.939857],[-60.450091,42.410056],[-62.275389,43.361830],[-63.524218,43.678265],[-65.584989,41.235781],[-67.014649,40.067049],[-68.624774,39.253966],[-69.127304,38.461179],[-69.642858,36.645272],[-71.357168,27.647225],[-72.334280,26.645808],[-72.965424,25.037349],[-73.296885,22.914075],[-73.374945,20.368215],[-72.955999,14.377650],[-72.078859,7.803479],[-70.431078,-4.144369],[-70.400978,-8.042394],[-70.746370,-9.149633],[-71.393768,-9.572717],[-75.154730,-9.368087],[-76.584830,-8.928899],[-77.747759,-8.283736],[-79.371359,-6.431308],[-80.224038,-3.922445],[-80.504301,-0.868789],[-80.410657,2.618017],[-79.895668,10.444512],[-79.718764,19.775069],[-78.866800,30.319365],[-76.629728,50.362115],[-76.876682,52.179941],[-78.100414,53.223475],[-80.159129,53.541259],[-82.911036,53.181840],[-86.214343,52.193761],[-89.927257,50.625567],[-93.907986,48.525804],[-98.014738,45.943015],[-100.957957,43.674243],[-103.609804,41.153399],[-107.896137,35.917060],[-110.587246,31.357122],[-111.245064,29.681782],[-111.396643,28.596715],[-112.143482,24.920329],[-112.510910,21.300276],[-112.520339,17.740233],[-112.193184,14.243881],[-111.550855,10.814899],[-110.614767,7.456967],[-107.946961,0.968970],[-104.361069,-5.190675],[-100.028393,-10.992531],[-95.120235,-16.407163],[-89.807898,-21.405133],[-82.476392,-27.310686],[-74.738404,-32.696643],[-66.637904,-37.538280],[-58.218865,-41.810872],[-49.525258,-45.489697],[-40.601053,-48.550029],[-31.490223,-50.967144],[-22.236738,-52.716319],[-13.053104,-53.639738],[-3.824868,-53.799685]]);
    }
  }
}

poly_rand(0.8);
poly_body(9);
poly_eye(4);
translate([5,1.5,4])
  scale([0.8,0.8,1])
    poly_eye(2);
poly_mouth(5);
