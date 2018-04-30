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
	

	for(int i=1;i<N;i++)
	{
		// select local new minimum
		//printf("\n[Process %d] Selected vertes is %d  --- iteration  %d\n",rank,selected_vertex,i);
		if(A[selected_vertex] < local_prev[0] && A[selected_vertex]!=0)
		{
			local_prev[0] = A[selected_vertex];
			local_prev[1] = selected_vertex;
		}
		//printf("\n[Process %d] Before gather with cost %d and parent %d\n",rank,local_prev[0],local_prev[1]);
		MPI_Gather(local_prev,2,MPI_INT,b,2,MPI_INT,0,MPI_COMM_WORLD);
		if(rank == 0)
		{
			//printf("\n master : Received data in iteration %d\n",i);
			//print_arr(b,2*N);
			//printf("\n");
			//printf("\nNow selecting minimum\n");
			int min_ = INT_MAX;
			int select_ = -1;
			for(int i=0;i<N;i++)
			{
				if( !visited[i] && b[2*i] < min_ )
				{
					select_ = i;
					min_ = b[2*i];
				}
			}
			//printf("\nAfter loop exit %d %d\n",select_,min_);
			printf("\n\t%d =====> %d : cost %d",select_,b[2*select_+1],min_);
			visited[select_] = 1;
			//printf("\nSelected : %d\n",select_);
			//
			for(int i=0;i<n;i++)
				offset[i] = select_;
		}
		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Scatter(offset,1,MPI_INT,&selected_vertex,1,MPI_INT,0,MPI_COMM_WORLD);
	}

	MPI_Finalize();

	

	return 0;

}
