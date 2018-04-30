#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

void process(int rank)
{
	printf("\nProcess[%d] ",rank);
}

void print_arr(int *arr,int n)
{
	for(int i=0;i<n;i++)
		printf("%d ",arr[i]);
}

int main(int argc,char* argv[])
{
	int rank,nps,temp_fd,fd;
	char file_name[10];
	int *a;
	int *A;
	int *offset;
	int n,N;
	int selected_vertex = 0;
	int *local_prev;
	int *b;
	bool *visited;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nps);

	local_prev = (int*)(malloc(sizeof(int)*2));
	local_prev[0] = INT_MAX;
	local_prev[1] = -1;

	sprintf(file_name,"output%d.txt",rank);

	fd = open(file_name,O_WRONLY | O_CREAT | O_TRUNC, 0666);

	if(rank == 0)
		temp_fd = dup(STDOUT_FILENO);

	dup2(fd,STDOUT_FILENO);

	// get input
	
	if(rank == 0)
	{
		dup2(temp_fd,STDOUT_FILENO);
		
		printf("\nEnter : ");
		scanf("%d",&n);
		a = (int*)(malloc(sizeof(int)*(n*n)));
		for(int i=0;i<n;i++)
		{
			for(int j=0;j<n;j++)
			{
				scanf("%d",&a[j*n + i]);
				printf("%d ",a[j*n+i]);
			}
		}
		b = (int*)(malloc(sizeof(int)*n*2));
		visited = (bool*)(malloc(sizeof(bool)*n));
		memset(visited,0,n);
		visited[0] = 1;		// p0
		offset = (int*)(malloc(sizeof(int)*n));
		for(int i=0;i<n;i++)
		{
			offset[i] = n;
		}
		printf("\nGot input\n");
		dup2(fd,STDOUT_FILENO);
		A = a;
	}
	//MPI_Barrier(MPI_COMM_WORLD);

	printf("Process [%d] waiting for scatter of N\n",rank);
	MPI_Scatter(offset,1,MPI_INT,&N,1,MPI_INT,0,MPI_COMM_WORLD);
	// now scattering
	A = (int*)(malloc(sizeof(int)*N));
	printf("Process [%d] waiting for scatter of A\n",rank);
	MPI_Scatter(a,n,MPI_INT,A,N,MPI_INT,0,MPI_COMM_WORLD);
	if(rank == 0)
		A = a;
	printf("Process [%d] got of size %d\n",rank,N);
	print_arr(A,N);
	printf("\n");
	printf("\n");
	MPI_Barrier(MPI_COMM_WORLD);	

	// p0 is making initial selection by default
	if(rank == 0)
		local_prev[0] = local_prev[1] = 0;
	

	for(int k=0;k<N;k++)
	{
		if(rank == i)
		{
			for(int j = k+1;j<N;j++)
				A[j] = A[j] / A[k];
			
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}

	MPI_Finalize();

	

	return 0;

}
