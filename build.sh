#!/bin/bash 

projectPath=$(pwd)

# Get parameter 
if [ "$#" -ne 1 ]; then 
    buildDir=build 
else 
  buildDir=$1 
fi 

# Create build directory 
mkdir $buildDir 
cd $buildDir 
 
# build 
cmake ../src -DBLOSC_INCLUDE_PATH=$projectPath/ExternalDependencies/c-blosc/install/include -DBLOSC_LIBRARY=$projectPath/ExternalDependencies/c-blosc/install/lib/libblosc.so -DSZ_INCLUDE_PATH=$projectPath/ExternalDependencies/SZ/sz/include -DSZ_LIBRARY=$projectPath/ExternalDependencies/SZ/install/lib/libSZ.so -DZLIB_LIBRARY=$projectPath/ExternalDependencies/SZ/install/lib/libzlib.so -DZSTD_LIBRARY=$projectPath/ExternalDependencies/SZ/install/lib/libzstd.so
make -j

