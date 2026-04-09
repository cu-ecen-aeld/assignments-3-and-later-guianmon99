# aesd-assignments
This repo contains public starter source code, scripts, and documentation for Advanced Embedded Software Development (ECEN-5713) and Advanced Embedded Linux Development assignments University of Colorado, Boulder.

## Setting Up Git

Use the instructions at [Setup Git](https://help.github.com/en/articles/set-up-git) to perform initial git setup steps. For AESD you will want to perform these steps inside your Linux host virtual or physical machine, since this is where you will be doing your development work.

## Setting up SSH keys

See instructions in [Setting-up-SSH-Access-To-your-Repo](https://github.com/cu-ecen-aeld/aesd-assignments/wiki/Setting-up-SSH-Access-To-your-Repo) for details.

## Specific Assignment Instructions

Some assignments require further setup to pull in example code or make other changes to your repository before starting.  In this case, see the github classroom assignment start instructions linked from the assignment document for details about how to use this repository.

## Testing

The basis of the automated test implementation for this repository comes from [https://github.com/cu-ecen-aeld/assignment-autotest/](https://github.com/cu-ecen-aeld/assignment-autotest/)

The assignment-autotest directory contains scripts useful for automated testing  Use
```
git submodule update --init --recursive
```
to synchronize after cloning and before starting each assignment, as discussed in the assignment instructions.

As a part of the assignment instructions, you will setup your assignment repo to perform automated testing using github actions.  See [this page](https://github.com/cu-ecen-aeld/aesd-assignments/wiki/Setting-up-Github-Actions) for details.

Note that the unit tests will fail on this repository, since assignments are not yet implemented.  That's your job :) 
ldd3: Linux Device Drivers 3 examples updated to work with recent kernels

# About
-----

Linux Device Drivers 3 (http://lwn.net/Kernel/LDD3/) book is now a few years
old and most of the example drivers do not compile in recent kernels.

This project aims to keep LDD3 example drivers up-to-date with recent kernels.

The original code can be found at: http://examples.oreilly.com/9780596005900/

# Compiling
----------

The example drivers should compile against latest Linus Torvalds kernel tree:
* git://git.kernel.org/pub/scm/linux/kernel/git/sfr/linux-next.git

To compile the drivers against a specific tree (for example Linus tree):
```
$ git clone git://github.com/martinezjavier/ldd3.git
$ git clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git
$ export KERNELDIR=/path/to/linux
$ cd ldd3
$ make
```

Bugs, comments or patches: See https://github.com/martinezjavier/ldd3/issues

# Latest Tested Kernel Builds
---------
The kernel builds below are the versions most recently tested/supported

* Ubuntu 18.04 kernel as of July 2020: 5.4.0-42-generic
* Ubuntu 20.04 kernel as of July 2021: 5.4.0-73-generic
* Yocto poky warrior branch kernel for qemu aarch64 builds: 5.0.19
* Yocto poky hardknott branch kernel for qemu aarch64 builds: 5.10.46
* Buildroot 2019.05 kernel for qemu builds: 4.9.16
* Buildroot 2021.02 kernel for qemu builds: 5.10
* Alpine 3.13 kernel as of May 2021: 5.10.29-lts, see [here](https://github.com/ericwq/gccIDE/wiki/ldd3-project) for detail.


# Eclipse Integration
---------4
Eclipse CDT integration is provided by symlinking the correct linux source directory with the ./linux_source_cdt symlink.
The .project and .cproject files were setup using instructions in [this link](https://wiki.eclipse.org/HowTo_use_the_CDT_to_navigate_Linux_kernel_source)
and assuming a symlink is setup in the local project directory to point to relevant kernel headers

This can be done on a system with kernel headers installed using:
```
ln -s /usr/src/linux-headers-`uname -r`/ linux_source_cdt
```
