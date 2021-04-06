# Sequential Stratified Regeneration: MCMC for Large State Spaces with an Application to Subgraph Counting Estimation


This project contains the framework 
used in the paper "Sequential Stratified Regeneration: MCMC for Large State Spaces with an Application to Subgraph Counting Estimation" 
available [here](https://arxiv.org/abs/2012.03879).

## Datasets

The datasets used in the paper are available in [SNAP](https://snap.stanford.edu/data/).
Please, apply the script "parseEdgesListToGph.py" to put the graph in Ripple's format. 

## Getting Started

For `Cmake` instructions check below

### Prerequisites

Besides g++ compiler (version 5.4), 
we need to install the follow libraries in order to compile Ripple:

```
libboost-all-dev
libtbb-dev
libgsl-dev
libsparsehash-dev
```

### Installing

Run the follow command in the root directory:

```
makefile
```

## Running the tests

The applications are available in the direction *apps*.
For example, the graph pattern mining application 
which uses Ripple to count subgraphs 
is in the directory *apps/ripple*.
To compile a particular application we need to 
execute the follow command in such application directory: 

```
./compile.sh
```

Then, a executable (binary) 
will be generated and we can 
run the application and get a description 
of its parameters
by executing 
the follow command line:

```
./app -h
```

## Cmake Configuration

```bash
#####
# Modify sync.sh to add desired location and then sync
#####
./sync <remote folder name> <1 for full 0 for partial>

#####
# Configure
#####
# On Mac Local
brew install boost 
brew install tbb
brew install gsl
brew install google-sparsehash
# On a linux machine running `GCC 7+` create or update your conda environment using
# First time
conda env create --file ripple.yml
# Checking Environment
conda env update --file ripple.yml  --prune

#####
# Running Live
#####
./compile.sh 
./app -i data/yeastbigcomp-sl.gph -o out.txt -k 6 -t 10 -c ./apps/matrioska/config.txt 

## License

This project is licensed under the GNU LGPL.

