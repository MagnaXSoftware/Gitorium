INSTALLATION
============

If you have the source distribution of Gitorium, you need to compile it.

Requirements
------------

To compile Gitorium, you must have

 *  gcc
 *  cmake >= 2.6
 *  libconfig >= 1.3.2
 *  libgit2 >= 0.17.0

To run it, you must have

 *  git (due to imcomplete libgit2 implementation of git)
 *  libconfig >= 1.3.2
 *  libgit2 >= 0.17.0

Compiling
---------

Run the following commands in the main directory:

    mkdir build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make

Installing
----------

From within the `build` directory, issue the following command (as root, if need be):

    make install

Binaries & Packages
-------------------

A Ubuntu deb is maintained by xavier-l [on launchpad](https://launchpad.net/~xav0989/+archive/gitorium).