#include "mpi.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

void bubble_sort(int *arr,int size);
void compare_split(int *,int*,int*,int,int,int);

int main(int argc,char* argv[])
{
	int n,npes,rank,i,even_phase_neigh,odd_phase_neigh,j,k,fd,old_out ,N;
	int local_size,recvd_size;
	int *array;
	int *local_array;
	int *recvd_array;
	int *temp_array;
	int *sizes;
	int *start_position;
	char file_name[20];
	MPI_Status status;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD,&npes);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);

	sprintf(file_name,"output%d.txt",rank);
	fd = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 0666);
	printf("Process %d's output will go to file %s\n",rank,file_name);
	MPI_Barrier(MPI_COMM_WORLD);


	if(rank == 0)
	{
		// get input in array
		printf("\n[ Process %d ]\tEnter size of array : ",rank);
		scanf("%d",&n);
		N = n;
		printf("\n[ Process %d ]\tEnter elements of array : ",rank);
		array = (int*)(malloc(sizeof(int)*n));
		sizes = (int*)(malloc(sizeof(int)*npes));
		start_position = (int*)(malloc(sizeof(int)*npes));
		for(i=0;i<n;i++)
		{
			scanf("%d",&array[i]);
		}
		k = 0;
		for(i=0;i<n%npes;i++)
		{
			sizes[i] = n/npes + 1;
			start_position[i] = (n/npes + 1)*i;
		}
		if(i!=0)
			k = start_position[i-1] + (n/npes+1);
		for(i = n%npes;i <npes ;i++)
		{
			sizes[i] = n/npes;
			start_position[i] = k+((i-n%npes)*n/npes);
		}
		old_out = dup(STDOUT_FILENO);
	}

	dup2(fd,STDOUT_FILENO);

	MPI_Barrier(MPI_COMM_WORLD);		// each process waiting to get input done

	MPI_Scatter(sizes,1,MPI_INT,&n,1,MPI_INT,0,MPI_COMM_WORLD);	// scatter local size

	local_array = (int*)(malloc(sizeof(int)*(n)));
	recvd_array = (int*)(malloc(sizeof(int)*(n+1)));
	temp_array = (int*)(malloc(sizeof(int)*(n)));

	MPI_Scatterv(array,sizes,start_position,MPI_INT,local_array,n,MPI_INT,0,MPI_COMM_WORLD);

	// now do odd-even sorting
	if(rank % 2 == 0)
	{
		even_phase_neigh = rank + 1;
		odd_phase_neigh = rank -1;
	}
	else
	{
		odd_phase_neigh = rank + 1;
		even_phase_neigh = rank - 1;
	}

	if(even_phase_neigh == -1 || even_phase_neigh == npes)
		even_phase_neigh = MPI_PROC_NULL;
	if(odd_phase_neigh == -1 || odd_phase_neigh == npes)
		odd_phase_neigh = MPI_PROC_NULL;

	// sort local_array
	printf("\nProcess[%d]\t local array size = %d\n",rank,n);
	printf("Process [%d]  Before Sort : ",rank);
	for(i=0;i<n;i++)
	{
		printf("%d ",local_array[i]);
	}
	printf("\n");
	bubble_sort(local_array,n);
	printf("Process[%d] After Sort : ",rank);
	for(i=0;i<n;i++)
	{
		printf("%d ",local_array[i]);
	}
	printf("\n");

	for(i=1;i<=npes;i++)
	{
		if(i%2==0)
		{
			printf("\nProcessor[%d] (Even Phase) After compare split with processor [%d] : ",rank,even_phase_neigh);
			MPI_Sendrecv(local_array,n,MPI_INT,even_phase_neigh,1,recvd_array,n+1,MPI_INT,even_phase_neigh,1,MPI_COMM_WORLD,&status);
		}
		else
		{
			printf("\nProcessor[%d] (Odd Phase) After compare split with processor [%d] : ",rank,odd_phase_neigh);
			MPI_Sendrecv(local_array,n,MPI_INT,odd_phase_neigh,1,recvd_array,n+1,MPI_INT,odd_phase_neigh,1,MPI_COMM_WORLD,&status);	
		}
		MPI_Get_count(&status,MPI_INT,&recvd_size);
		if( rank < status.MPI_SOURCE)
			j = 1;
		else 
			j = 0;
		if(status.MPI_SOURCE != -1)
			compare_split(local_array,recvd_array,temp_array,n,recvd_size,j);

		for(k=0;k<n;k++)
		{
			printf("%d ",local_array[k]);
		}
		printf("\n");
	}

	MPI_Gatherv(local_array,n,MPI_INT,array,sizes,start_position,MPI_INT,0,MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);

	if( rank == 0 )
	{
		dup2(old_out,STDOUT_FILENO);
		printf("\n");
		for(i=0;i<N;i++)
		{
			printf("%d ",array[i]);
		}
		printf("\n");
	}

	free(local_array);
	free(temp_array);
	free(recvd_array);

	MPI_Finalize();

	return 0;

}

void compare_split(int *local,int *recvd,int *temp,int l_size,int r_size,int flag)
{
	// flag = 1 => save small elements
	// flag = 0 => save large elements
	int i,j,k;

	for(i=0;i<l_size;i++)
		temp[i] = local[i];
	if(flag == 1)
	{
		for(i=0,k=0,j=0;i<l_size;i++)
		{
			if( k == r_size || ( j < l_size && (temp[j] < recvd[k])) )
				local[i] = temp[j++];
			else
				local[i] = recvd[k++];
		}
	}
	else
	{
		for(i=l_size-1,k=r_size-1,j=l_size-1;i>=0;i--)
		{
			if( k == -1 || ( j  >= 0 && (temp[j] > recvd[k])) )
				local[i] = temp[j--];
			else
				local[i] = recvd[k--];
		}
	}

}

void bubble_sort(int *arr,int size)
{
	int i,j;
	for(i = 0;i<size;i++)
	{
		for(j=1;j<size;j++)
		{
			if(arr[j-1] > arr[j])
			{
				int temp = arr[j-1];
				arr[j-1] = arr[j];
				arr[j] = temp;
			}
		}
	}
}