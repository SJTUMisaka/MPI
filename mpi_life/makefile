EXnode=mpi_life
MPICC?=mpicc

all: ${EXnode}

mpi_life: mpi_life.c
		${MPICC} -o mpi_life mpi_life.c -lm

clean:
		rm -f ${EXnode}