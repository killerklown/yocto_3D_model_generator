./bin/model -o in/empty.obj empty
./bin/raytrace -r 720 -s 3 -o out/empty.png in/empty.obj

./bin/model -o in/simple.obj simple
./bin/raytrace -r 720 -s 3 -o out/simple.png in/simple.obj

./bin/model -o in/displace.obj displace
./bin/raytrace -r 720 -s 3 -o out/displace.png in/displace.obj

./bin/model -o in/instances.obj instances
./bin/raytrace -r 720 -s 3 -o out/instances.png in/instances.obj

./bin/model -o in/normals.obj normals
./bin/raytrace -r 720 -s 3 -o out/normals.png in/normals.obj

./bin/model -o in/subdiv.obj subdiv
./bin/raytrace -r 720 -s 3 -o out/subdiv.png in/subdiv.obj
