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
