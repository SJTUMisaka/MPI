#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_LOW((id)+1,p,n)-BLOCK_LOW(id,p,n))

int main(int argc, char** argv) {
  double elapsed_time; //execution time
  int size, rank, tag, generations, outPoints;
  int m, n;
  int i, j, k;
	MPI_Status Stat;

  MPI_Init(&argc,&argv);

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);  

  if (argc != 3) {
    if (!rank) printf ("Command line: %s <m>\n", argv[0]);
    MPI_Finalize();
    exit (1);
  }
  generations = atoi(argv[1]);
  outPoints = atoi(argv[2]);
  tag = 66;

  FILE* fp = fopen("life.txt", "r");
  fscanf(fp, "m=%d, n=%d\n", &m, &n);  

  int **slice, *slicestorage;
  slicestorage = (int *)malloc(m * n * sizeof(int));
  slice = (int **)malloc(m * sizeof(int *));

  for (i = 0; i < m; i++){
    for (j = 0; j < n; j++){
      fscanf(fp, "%d", &slicestorage[i * n + j]);
    }
    slice[i] = &slicestorage[i * n];
  }
  fclose(fp);
  
  int **myslice, *myslicestorage;
  myslicestorage = (int *) malloc (BLOCK_SIZE(rank, size, m) * n * sizeof(int));
  myslice = (int **)malloc(BLOCK_SIZE(rank, size, m) * sizeof(int *));
  for (i = 0; i < BLOCK_SIZE(rank, size, m); i++){
    for (j = 0; j < n; j++){
      myslicestorage[i * n + j] = slice[i + BLOCK_LOW(rank, size, m)][j];
    }
    myslice[i] = &myslicestorage[i * n];
  }

  int *todown, *fromup, *toup, *fromdown;
  todown = (int *)malloc(n * sizeof(int));
  fromup = (int *)malloc(n * sizeof(int));
  toup = (int *)malloc(n * sizeof(int));
  fromdown = (int *)malloc(n * sizeof(int));
  elapsed_time = -MPI_Wtime();
  for (int turn = 0; turn < generations; turn++){
    if (rank != size - 1){ // not last
      for (j = 0; j < n; j++){
        todown[j] = myslice[BLOCK_SIZE(rank, size, m)-1][j];
      }
      MPI_Send(todown, n, MPI_INT, rank + 1, tag, MPI_COMM_WORLD);
    }
    if (rank != 0){ // not first
      MPI_Recv(fromup, n, MPI_INT, rank - 1, tag, MPI_COMM_WORLD, &Stat);
    }
    else{
      for (j = 0; j < n; j++){
        fromup[j] = 0;
      }
    }
    if (rank != 0){ // not first
      for (j = 0; j < n; j++){
        toup[j] = myslice[0][j];
      }
      MPI_Send(toup, n, MPI_INT, rank - 1, tag, MPI_COMM_WORLD);
    }
    if (rank != size - 1){ // not last
      MPI_Recv(fromdown, n, MPI_INT, rank + 1, tag, MPI_COMM_WORLD, &Stat);
    }
    else{
      for (j = 0; j < n; j++){
        fromdown[j] = 0;
      }
    }
    // Counting
    int sum;
    int **newmyslice, *newmyslicestorage;
    newmyslicestorage = (int *) malloc (BLOCK_SIZE(rank, size, m) * n * sizeof(int));
    newmyslice = (int **)malloc(BLOCK_SIZE(rank, size, m) * sizeof(int *));
    for (i = 0; i < BLOCK_SIZE(rank, size, m); i++){
      newmyslice[i] = &newmyslicestorage[i * n];
    }
    // 1 2 3
    // 4 5 6
    // 7 8 9
    for (i = 0; i < BLOCK_SIZE(rank, size, m); i++){
      for (j = 0; j < n; j++){
        sum = 0;
        // 1 4 7
        if (j != 0){
          if (i == 0)
            sum += fromup[j - 1];
          else
            sum += myslice[i - 1][j - 1];
          sum += myslice[i][j - 1];
          if (i == BLOCK_SIZE(rank, size, m) - 1)
            sum += fromdown[j - 1];
          else
            sum += myslice[i + 1][j - 1];
        }
        // 2 8
        if (i == 0)
          sum += fromup[j];
        else
          sum += myslice[i - 1][j];
        if (i == BLOCK_SIZE(rank, size, m) - 1)
          sum += fromdown[j];
        else
          sum += myslice[i + 1][j];
        // 3 6 9
        if (j != n - 1){
          if (i == 0)
            sum += fromup[j + 1];
          else
            sum += myslice[i - 1][j + 1];
          sum += myslice[i][j + 1];
          if (i == BLOCK_SIZE(rank, size, m) - 1)
            sum += fromdown[j + 1];
          else
            sum += myslice[i + 1][j + 1];
        }
        if (myslice[i][j] == 0 && sum == 3)
            newmyslice[i][j] = 1;
        else if (sum == 2 || sum == 3)
          newmyslice[i][j] = myslice[i][j];
        else
          newmyslice[i][j] = 0;
      }
    }
    for (i = 0; i < BLOCK_SIZE(rank, size, m); i++){
      for (j = 0; j < n; j++){
        myslice[i][j] = newmyslice[i][j];
      }
    }
    if ((turn + 1) % outPoints == 0){
      if (rank){
        MPI_Send(myslicestorage, BLOCK_SIZE(rank, size, m) * n, MPI_INT, 0, tag, MPI_COMM_WORLD);
      }
      else{
        printf("%d\n", turn);
        printf("-------------------------\n");
        for (i = 0; i < BLOCK_SIZE(rank, size, m); i++){
          for (j = 0; j < n; j++){
            printf("%d", myslice[i][j]);
          }
          printf("\n");
        }
        for (int id = 1; id < size; id++){
          int *printstorage;
          printstorage = (int *) malloc (BLOCK_SIZE(id, size, m) * n * sizeof(int));
          MPI_Recv(printstorage, BLOCK_SIZE(id, size, m) * n, MPI_INT, id, tag, MPI_COMM_WORLD, &Stat);
          for (i = 0; i < BLOCK_SIZE(id, size, m); i++){
            for (j = 0; j < n; j++){
              printf("%d", printstorage[i * n + j]);
            }
            printf("\n");
          }
        }
        printf("-------------------------\n");
      }
    }
  }
  elapsed_time += MPI_Wtime();
  if (!rank)
    printf("time: %10.6f\n", elapsed_time);
  MPI_Finalize();
  return 0;
}