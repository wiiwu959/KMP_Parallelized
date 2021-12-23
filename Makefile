CXX = g++
SERIALCLAGS = -g -O3
CFLAGS	= -g -O3 -fopenmp
PTHREAD_FLAGS = -g -O3 -pthread

all: serial kmp_omp kmp_pthread

serial: serial.cpp
	$(CXX) $(SERIALCLAGS) serial.cpp -o serial

kmp_omp: kmp_omp.cpp
	$(CXX) $(CFLAGS) kmp_omp.cpp -o kmp_omp

kmp_pthread: kmp_pthread.cpp
	$(CXX) $(PTHREAD_FLAGS) kmp_pthread.cpp -o kmp_pthread -Wall

clean:
	rm -f kmp_omp serial kmp_pthread