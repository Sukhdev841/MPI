#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"

int main(int argc,char* argv[])
{
  int rank,np;
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&np);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  // start of program

  if(rank == 0)
  {
    int count,i,n=4;
    int arr[]={1,2,3,4};
    float ans[1];
    MPI_Status status;
    // int *arr;
    // printf("\nprocessor %d: Enter n : ",rank);
    // scanf("%d",&n);
    // arr = (int*)malloc(sizeof(int)*n);
    // for(i=0;i<n;i++)
    // {
    //   printf("\nprocessor %d: Enter value of y%d : ",rank,i+1);
    //   scanf("%d",&arr[i]);
    // }

    MPI_Send(arr,4,MPI_INT,1,0,MPI_COMM_WORLD);
    printf("processor %d: data sent.\n",rank);

    // now receive result from processor with rank 1
    n=1;
    printf("processor %d: Going to receive.\n",rank);
    MPI_Recv(ans,1,MPI_FLOAT,1,0,MPI_COMM_WORLD,&status);
    printf("processor %d: ans = %f\n",rank,*ans);
  }
  else
  {
    printf("processor %d: receving data.\n",rank);
    int arr[10];
    int n=10,i;
    MPI_Status status;
    MPI_Recv(arr,10,MPI_INT,0,0,MPI_COMM_WORLD,&status);
    MPI_Get_count(&status,MPI_FLOAT,&n);
    printf("\nprocessor %d: %d is n.\n",rank,n);
    float *ans = (float*)(malloc(sizeof(float)));
    *ans = 0;
    for(i=0;i<n;i++)
    {
      (*ans) += arr[i];
    }
    (*ans) /= n;
    MPI_Send(ans,1,MPI_FLOAT,0,0,MPI_COMM_WORLD);
    printf("\nprocessor %d: %f sent to calling processor.\n",rank,*ans);
  }



  // end of program
  MPI_Finalize();
  return 0;
}
