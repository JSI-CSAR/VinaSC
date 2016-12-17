# VinaSC
@Author: Lang Yu  
Email: langyu at uchicago dot edu

## What is VinaSC
VinaSC is a modified version of Autodock Vina, with the ability to scalable docking multiple ligand-receptor pairs onto multiple machines. In addition, VinaSC supports both CPU and MIC platforms and implements an infrastructure to collaborate them, which exploits heterogeneous computing resources.

## Contributions  

Main contributions include:

- Offloading infrastructure using Intel COI to collaborate CPU and MIC
- Modified the searching logics. We implemented a pipeline workflow to perform confromation search. By doing so, overhead incurred by the variance of thread execution 
- We implemented a dynamic scheduler to distribute workloads between CPU and MIC according to the real-time performance data. The dynamic scheduler turns out to be of great help in improving performance.


## Instructions on Compilation and Execution
1. Compilation

	***Prerequest:*** 
	- Intel COI
	- Intel ICC
	- Intel MPI
	- Boost Library (CPU version & MIC version)
	
	First, modify the Makefile under build/linux/release/, point BOOST\_INCLUDE to the location of CPU version of Boost Library, and MIC\_BOOST\_INCLUDE to the MIC version. Specify the correct version in BOOST_VERSION.
	
	Please run the compilation script (compile.sh) on the root directory, executable vina is copied to test/
	
2. Execution
	
	Example input including ligand file, protein file and configuration file is placed under input/
	
	More configuration options can be found in Autodock Vina website: http://vina.scripps.edu/index.html
	
	Example execution scripts are provided in test/. In general, if you want to execute Vina in single machine, run "./vina --config CONFIGURE_FILE --log LOG_FILE". To run it in multiple nodes, try "mpirun -n NUMBER_OF_NODES -machinefile MACHINE_FILE ./vina --config CONF_FILE --log LOG_FILE"