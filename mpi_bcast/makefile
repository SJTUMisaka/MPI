EXnode=mpi_bcast
MPICC?=mpicc

all: ${EXnode}

mpi_bcast: mpi_bcast.c
		${MPICC} -o mpi_bcast mpi_bcast.c -lm

clean:
		rm -f ${EXnode}