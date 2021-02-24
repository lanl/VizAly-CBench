#!/bin/bash
module purge
module load gcc/6.4.0
module load openmpi/2.1.3-gcc_6.4.0
module load cmake
module load anaconda/Anaconda3
source activate cbench

# Automatically set up conda environment if not found
if [ $? == 1 ]; then
    echo "Conda env not found, onetime setup.."
    conda create --yes --name cbench python=3.6
    source activate cbench

    # install Python packages
    conda install --yes numpy==1.15.4 matplotlib==3.0.2
    python -m pip install apsw==3.9.2.post1
    python -m pip install cv2
    python -m pip install scipy
fi

# hdf5
export LD_LIBRARY_PATH=/projects/exasky/hdf5/install/lib:$LD_LIBRARY_PATH
export PATH=/projects/exasky/hdf5/install/bin:$PATH

# fftw 3.3.8
export LD_LIBRARY_PATH=/projects/exasky/fftw-3.3.8/install/lib:$LD_LIBRARY_PATH
export FFTW_MAJOR_VERSION=3

# HACC
export HACC_PLATFORM="Darwin"
export HACC_OBJDIR="${HACC_PLATFORM}"

export HACC_CFLAGS="-O3 -g -fopenmp -std=c++11"
export HACC_CC="gcc"

export HACC_CXXFLAGS="-O3 -g -fopenmp -std=c++11"
export HACC_CXX="g++"

export HACC_LDFLAGS="-lm -fopenmp"

export HACC_MPI_CFLAGS="-O3 -std=gnu99 -g -fopenmp -std=c++11"
export HACC_MPI_CC="mpicc"


export HACC_MPI_CXXFLAGS="-O3 -g -Wno-deprecated -fopenmp -fPIC -std=c++11"
export HACC_MPI_CXX="mpicxx"

export HACC_MPI_LDFLAGS="-lm -fopenmp"

## Propagate GenericIO env variables
export GIO_MPICXX="${HACC_MPI_CXX}"


# Set python path
if [ $# -eq 0 ]
then

else
	foresight_home=$1
	echo "foresight home is " $foresight_home
    export PYTHONPATH=$PYTHONPATH:$foresight_home
fi
