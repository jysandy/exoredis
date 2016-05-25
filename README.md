# ExoRedis
By J. Y. Sandilya

### Dependencies
To build ExoRedis you will need:  
* A C++14 compliant C++ compiler (I suggest g++4.9 or better). I myself use
g++ 5.2.  
* CMake. CMake is a C++ build system. It should be available through your distro's
package manager.
* The Boost C++ libraries. These should be available through your distro's package
manager.  

To install CMake and Boost on Ubuntu, simply run:  
``` bash
sudo apt-get install cmake libboost-all-dev
```

There are different ways to install the right C++ compiler. If you are using
Ubuntu 14.04 then the default g++ package gives you version 4.8.4 which is not good
enough. You will have to use a PPA to install a more recent version. On the other
hand if you are using a more recent version of Ubuntu then the default package
should do. I myself use Ubuntu 15.10, which gives me g++5.2.

##### Optional: Integration tests
The integration tests are written in Python. To run the integration tests you
will need:  
* Python 3.5.1. I suggest using Anaconda/Miniconda to install it.
* pytest

To install Miniconda, follow the instructions
[here](http://conda.pydata.org/docs/install/quick.html#linux-miniconda-install).
I suggest you add the path to your .bashrc when prompted.  
Once Miniconda is installed, create a new Python 3 environment for exoredis,
activate it and install pytest:  
``` bash
conda create exoredis python=3
source activate exoredis
pip install pytest
```

### Building ExoRedis
First make sure you have the dependencies installed. Then, from the exoredis
root directory:
``` bash
mkdir build
cd build
cmake ..
make
```
**Note:** This requires that the symlink ``` /usr/bin/c++``` points to your
C++14 compiler (if you have installed it using a PPA on Ubuntu, you can use
```update-alternatives``` to change this if you want). If this is not the case, use the following commands instead:  
``` bash
mkdir build
cd build
cmake -D CMAKE_CXX_COMPILER=</path/to/your/compiler> ..
make
```

### Running ExoRedis
All commands are to be run from the ``` build/``` directory.
##### Running the tests
``` bash
unit_tests/tests    # Unit tests
activate exoredis
py.test             # Integration tests
```
There should be zero failures. Please get back to me if any test fails.
##### Running ExoRedis
It is as simple as:
``` bash
./exoredis <file_name>
```
Ctrl-C will stop the server.
