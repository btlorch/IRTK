IRTK forked from [BioNedIA](https://github.com/BioMedIA/IRTK)
=========================

First time build
----------------
```
mkdir build && cd build/
cmake -DCMAKE_INSTALL_PREFIX=../install ..
make
```

Rebuild only one application
----------------------------
```
cd build/
make shiftRegion
./Applications/shiftRegion <input> <vector field> <output>
```
