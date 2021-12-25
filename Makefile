CXX = g++
SERIALCLAGS = -g -O3
CFLAGS	= -g -O3 -fopenmp
PTHREAD_FLAGS = -g -O3 -pthread

all: kmp_serial kmp_omp kmp_pthread kmp_cuda kmp_mpi

kmp_serial: kmp_serial.cpp
	$(CXX) $(SERIALCLAGS) kmp_serial.cpp -o kmp_serial

kmp_omp: kmp_omp.cpp
	$(CXX) $(CFLAGS) kmp_omp.cpp -o kmp_omp

kmp_pthread: kmp_pthread.cpp
	$(CXX) $(PTHREAD_FLAGS) kmp_pthread.cpp -o kmp_pthread -Wall

kmp_cuda: kmp_cuda.cu
	nvcc kmp_cuda.cu -o kmp_cuda

kmp_mpi: kmp_mpi.cpp
	mpicxx kmp_mpi.cpp -o kmp_mpi

clean:
	rm -f kmp_serial kmp_omp kmp_pthread kmp_cuda kmp_mpi