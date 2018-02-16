#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc,char* argv[])
{
  int npes;
  int myrank;
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&npes);
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);

  const char *msg="hello";

  if(myrank == 0)
  {
    MPI_Send(msg,strlen(msg),MPI_CHAR,1,0,MPI_COMM_WORLD);
    printf("Message sent by processor 0\n");
  }
  else
  {
    MPI_Status status ;
    char* buff = (char*)(malloc(sizeof(char)*100));
    MPI_Recv(buff,100,MPI_CHAR,0,0,MPI_COMM_WORLD,&status);
    printf("%s is message received by processor 1 \n",buff);
  }

  MPI_Finalize();

  return 0;
}
