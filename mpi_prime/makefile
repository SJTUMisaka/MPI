EXnode=mpi_prime
MPICC?=mpicc

all: ${EXnode}

mpi_prime: mpi_prime.c
		${MPICC} -o mpi_prime mpi_prime.c -lm

clean:
		rm -f ${EXnode}
