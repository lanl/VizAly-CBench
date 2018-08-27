#! /bin/bash

# get GCC compiler
source /src/env.sh

# build
cd /src/VizAly-CBench
projectPath=$(pwd)


source buildAll.sh

# run example
mpirun -np 2 --allow-run-as-root ./CBench ../inputs/all.json

# view output
cat metrics
