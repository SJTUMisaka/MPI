#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <string.h>


int My_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
    int rank, size, i;
    MPI_Status status;
    int tag = 66;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == root){
        for (i = 0; i < size; i++){
            if (i != root)
                MPI_Send(buffer, count, datatype, i, tag, comm);
        }
    }
    else
        MPI_Recv(buffer, count, datatype, root, tag, comm, &status);
    
    return 0;
}



int main(int argc, char **argv){
    int size, rank;
    double elapsed_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int mode=0;
    char message[100];

    if (argc != 2) {
        if (!rank) printf("Command line: %s <m>\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    mode = atoi(argv[1]);
    
    if (!rank)
        strcpy(message, "Message from root");

    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = - MPI_Wtime();
    if(mode == 0){
	if (!rank)
            printf("My Bcast\n");
        My_Bcast(message, 100, MPI_CHAR, 0, MPI_COMM_WORLD);
    }
    else{
	if (!rank)
	    printf("Original MPI_Bcast\n");
        MPI_Bcast(message, 100, MPI_CHAR, 0, MPI_COMM_WORLD);
    }
    printf("Message: %s, process id %d\n", message, rank);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time += MPI_Wtime();
    if (!rank)
        printf("Total elasped time: %10.6f\n", elapsed_time);
    MPI_Finalize();
    return 0;
}
