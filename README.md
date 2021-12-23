<div align="center">
   <h3> Parallel Programming </h3>
   <a href="https://nycu-sslab.github.io/PP-f21">IOC5181, Parallel Programming @ NYCU, Fall 2021</a>
</div>

## About this Repository
* This is the repository for the final Project in NYCU CS Parallel Programming(PP) fall 2021 class.
* We are going to implement parallelization of KMP String Matching Algorithm on different parallel architectures.
* Our Implementation is based on <a href="https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.258.8768&rep=rep1&type=pdf">
Akhtar Rasool and Nilay Khare, "Parallelization of KMP String Matching Algorithm on Different SIMD architectures-Multi-Core and GPGPU", International Journal of Computer Applications, pp. 26-28, 2012. </a>.
* We will also evaluate and compare performance of different parallel architecture shown in our report.


## Features
- [x] Pthreads
- [x] OpenMP
- [x] MPI
- [x] CUDA

## How to Run
- ### Pthreads
- ### OpenMP
- ### MPI
  ```
  # Compile
  mpicxx KMP_MPI.cpp -o KMP_MPI 
  # Run with specified # of processes and hostfile
  mpirun -np {# of processes} --hostfile {hostfile_name} KMP_MPI

  # Example (Two processes and hostfile named hosts)
  # mpirun -np 2 --hostfile hosts KMP_MPI
  ```
- ### CUDA GPU 
