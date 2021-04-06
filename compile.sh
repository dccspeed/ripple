#!/bin/zsh
num_jobs=${SLURM_CPUS_PER_TASK:-1}
build_type=${1:-Release}
build_dir=${2:-build}
echo "Building $build_type (1) in directory $build_dir (2) with $num_jobs cpus"

# Load necessary module on gilbreth
if [[ $(hostname) =~ (gilbreth|snyder)-fe[0-9]+\.rcac\.purdue\.edu ]]
then
    source /etc/profile.d/modules.sh
    module load gcc/7.3.0
    . $(conda info --base)/etc/profile.d/conda.sh
    conda activate ripple
    export CC=$(which gcc)
    export CXX=$(which g++)
else
    # Activate Conda environment
    eval "$(conda shell.zsh hook)"
    conda activate ripple
fi
# Cmake
cmake -DCMAKE_BUILD_TYPE=$build_type -S . -B ./$build_dir
cmake --build ./$build_dir --target all -- -j $num_jobs

# Get executable in desired directory
mv ./$build_dir/matrioska ./
mv ./$build_dir/psrw ./
mv ./$build_dir/rgpm ./