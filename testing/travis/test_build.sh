#! /bin/bash

# get GCC compiler
source /src/env.sh

# create build dir
cd /src/VizAly-CBench
mkdir build
cd build

# build
cmake ../src
make -j

# run example
./CBench ../inputs/blosc.json

# view output
cat runlog_0.log
