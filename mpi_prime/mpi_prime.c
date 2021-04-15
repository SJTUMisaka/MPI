#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int check_prime(int id, int z){
    int yes = 1;
    if (z < 2) yes = 0;
    for (int i = 2; i * i <= z; i++){
        if (z % i == 0){
            yes = 0;
            break;
        }
    }
    return yes;
}
int main(int argc, char *argv[]) {
    double elapsed_time;    /* Parallel execution time */
    int i;
    int j;
    int id;                 /* Processor ID number */
    int n;                  /* Seiving from 2 ... `n` */
    int p;                  /* Number of processes */
    

    int n_primes;           /* Number of primes up to sqrt(n) */
    int sqrt_n;             /* sqrt(n) */
    int *primes;            /* List of primes up to sqrt(n) */
    char *marked;           /* Integer list to be marked */
    char *global_marked;    /* Reduced integer list after marking */
    int global_count;       /* Count of primes */


    MPI_Init(&argc, &argv);
    
    /* Start the timer */
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if (argc != 2) {
        if (!id) printf("Command line: %s <m>\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }
    n = atoi(argv[1]);

    if (n < 2) {
        if (!id) printf("Total number of primes up to %d: 0\n", n);
        MPI_Finalize();
        exit(0);
    }

    sqrt_n = (int)floor(sqrt(n));
    primes = (int *)malloc(sizeof(int) * sqrt_n);
    marked = (char *)malloc(sizeof(char) * (n + 1));    /* marked[i] for integer i */
    
    if (primes == NULL || marked == NULL) {
        if (!id) printf("Memory allocation failed\n");
        MPI_Finalize();
        exit(1);
    }

    if (!id) {
        global_marked = (char *)malloc(sizeof(char) * (n + 1));
        if (global_marked == NULL) {
            printf("Memory allocation failed for `global_marked`\n");
            MPI_Finalize();
            exit(1);
        }
    }

    marked[0] = marked[1] = 1;
    for (i = 2; i <= n; ++ i)
        marked[i] = 0;

    
    n_primes = 0;
    for (i = 2; i <= sqrt_n; ++ i) {
        if (check_prime(id, i)) {
            primes[n_primes] = i;
            ++ n_primes;
        }
    }
    
    for (i = id; i < n_primes; i += p) {
        // sieve multiples of primes[i]
        for (j = 2 * primes[i]; j <= n; j += primes[i])
            marked[j] = 1;
    }

    MPI_Reduce(marked, global_marked, n + 1, MPI_CHAR, MPI_LOR, 0, MPI_COMM_WORLD);

    /* Stop the timer */
    elapsed_time += MPI_Wtime();

    if (!id) {
        global_count = 0;
        for (i = 2; i <= n; ++ i) {
            if (!global_marked[i]) {
                ++ global_count;
            }
        }
    }
    /* Print the results */
    if (!id) {
        printf("Total number of primes up to %d: %d\n", n, global_count);
        printf("Total elapsed time: %10.6f seconds\n", elapsed_time);
    }
    MPI_Finalize();
    
    return 0;
}