FROM ubuntu
RUN apt-get -qq update
RUN apt-get install -y software-properties-common
RUN apt-get -qq update
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y
RUN apt-get -qq update

# Toolchain?
RUN apt-get install -y --fix-missing cmake
RUN apt-get install -y g++-6
RUN apt-get install -y clang++-4.0
# Nuto deps
RUN apt-get install -y libeigen3-dev libiomp-dev swig3.0 doxygen python3-dev python3-numpy libopenblas-dev libmetis-dev libmumps-seq-dev libann-dev libarpack2-dev gmsh 
# boost
RUN apt-get install -y libboost-filesystem-dev libboost-system-dev libboost-mpi-dev libboost-test-dev
# coverage and documentation
RUN apt-get install -y lcov curl texlive-font-utils

#fix missing /dev/fd from https://github.com/jbbarth/docker-ruby/commit/1916309122b7c04be4c01c46910471fc1d8176c6
RUN test -e /dev/fd || ln -s /proc/self/fd /dev/fd

# setup non root user named nuto with sudo rights, following the instructions from
# https://stackoverflow.com/questions/25845538/using-sudo-inside-a-docker-container
RUN apt-get install -y sudo
RUN useradd nuto && echo "nuto:nuto" | chpasswd && adduser nuto sudo
RUN mkdir -p /home/nuto && chown -R nuto:nuto /home/nuto
USER nuto

# create the source directory /home/nuto/source
RUN mkdir /home/nuto/source

# the build directory will be in /home/nuto 
WORKDIR /home/nuto
