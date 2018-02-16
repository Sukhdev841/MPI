#include "mpi.h"
#include "stdio.h"
#include "stdlib.h"

int main(int argc,char* argv[])
{
  int rank,nps;
  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD,&nps);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  //** code goes here
  if(rank == 0)
  {
    int arr[]={1,2,-1,3,4,0,3,1,0,-2,3,4,-10},temp[1];
    int len = 13;
    int part_length = len/(nps-1),i,n,j,min=arr[0];
    int *buff;
    printf("\nprocessor %d: Sending data.",rank);
    buff = (int*)(malloc(sizeof(int)*part_length+1));
    for(i=0;i<nps-1;i++)
    {
      if(i!=nps-2)
      {

          n=part_length;
          for(j=0;j<n;j++)
          {
            buff[j]=arr[i*part_length+j];
          }
      }
      else
      {
          n=len-(nps-2)*part_length;
          for(j=0;j<n;j++)
          {
            buff[j]=arr[i*part_length+j];
          }
      }
      printf("\nprocessor %d: Data length is %d.",rank,n);
      MPI_Send(buff,n,MPI_INT,i+1,0,MPI_COMM_WORLD);
      printf("\nprocessor %d: Data sent to processor %d.",rank,i+1);
    }

    printf("\nprocessor %d: Receiving Local Minimums.",rank);

    for(i=0;i<nps-1;i++)
    {
      MPI_Status status;
      MPI_Recv(temp,1,MPI_INT,i+1,0,MPI_COMM_WORLD,&status);
      printf("\nprocessor %d: %d received from processor %d.",rank,*temp,i+1);
      if(min>*temp)
        min = *temp;
    }

    printf("\nprocessor %d: Minimum = %d\n",rank,min);
  }
  else
  {
    int arr[100],n,i,j,min[1];
    MPI_Status status;
    printf("\nprocessor %d: Receiving data.",rank);
    MPI_Recv(arr,100,MPI_INT,0,0,MPI_COMM_WORLD,&status);
    MPI_Get_count(&status,MPI_INT,&n);
    *min = arr[0];
    for(i=0;i<n;i++)
    {
      if(*min>arr[i])
        *min = arr[i];
    }
    printf("\nprocessor %d: Sending Local Minimum %d.\n",rank,*min);
    MPI_Send(min,1,MPI_INT,0,0,MPI_COMM_WORLD);
  }
  // code ends here
  MPI_Finalize();
  return 0;
}
