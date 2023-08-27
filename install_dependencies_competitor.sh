#!/bin/bash
sudo apt-get install -f -y flex bison libboost-dev libgraphviz-dev mkdocs
pip3 install Flask==2.0.0
pip3 install Jinja2==3.0.0
pip3 install materialx pylogics
cd submodules/cudd
git checkout "v3.0.0"
autoreconf -f -i
PREFIX="/usr/local"
./configure --enable-silent-rules --enable-obj --enable-dddmp --prefix=$PREFIX
make
sudo make install
cd ../mona
./configure && make && sudo make install
cd ../syft
git checkout "v0.1.1"
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
sudo make install
cd ../lydia
git submodule update --init --recursive
mkdir build && cd build
cmake ..
make -j4 lydia lydia-bin
sudo make install lydia lydia-bin
make clean
cd ..
rm -rf build
cd ../syft/build
make clean
cd ..
rm -rf build
cd ../mona
make clean
cd ../cudd
make clean
pip3 install logaut
cd ..
echo "The script generating the graph via logaut and Lydia is root/competitors/lydia.sh"
