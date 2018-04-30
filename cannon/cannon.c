#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>


int main(int argc,char* argv[])
{
	int rank,nps,rank_2d,fd,src,dest,i,j,dim_size;
	int size;
	int n,N,temp_fd;
	int *A = NULL;
	int *B = NULL;
	int *sizes = NULL;
	int *offsets = NULL;
	MPI_Comm comm;
	MPI_Status status;
	int dim_sizes[2];
	int periods[2];
	int my_coords[2];
	char file_name[10];

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nps);	

	sprintf(file_name,"output%d.txt",rank);

	fd = open(file_name,O_WRONLY | O_CREAT | O_TRUNC, 0666);

	if(rank == 0)
		temp_fd = dup(STDOUT_FILENO);

	dup2(fd,STDOUT_FILENO);

	dim_size = dim_sizes[0] = dim_sizes[1] = sqrt(nps);
	periods[0] = periods[1] = 1;	// wrap around

	MPI_Cart_create(MPI_COMM_WORLD,2,dim_sizes,periods,1,&comm);

	MPI_Comm_rank(comm,&rank_2d);
	printf("\n[Process %d] 2D rank = %d\n",rank,rank_2d);
	MPI_Cart_coords(comm,rank_2d,2,my_coords);
	printf("\n[Process %d] My Coordinates are : %d %d \n",rank,my_coords[0],my_coords[1]);
	MPI_Cart_rank(comm,my_coords,&rank_2d);
	printf("\n[Process %d] rank from Coordinates is : %d\n",rank,rank_2d);

	// get input
	if(rank_2d == 0)
	{
		dup2(temp_fd,STDOUT_FILENO);
		printf("\nEnter Size of matrices : ");
		scanf("%d",&n);
		A = (int*)(malloc(sizeof(int)*n*n));
		B = (int*)(malloc(sizeof(int)*n*n));
		sizes = (int*)(malloc(sizeof(int)*dim_size));
		offsets = (int*)(malloc(sizeof(int)*dim_size));

		printf("\nGive Matrix A\n");
		for(i = 0;i<n;i++)
		{
			for(j=0;j<n;j++)
			{
				printf("\nEnter A[%d][%d] : ",i+1,j+1);
				scanf("%d",&A[i*n+j]);
			}
		}
		printf("\n");
		for(i = 0;i<n*n;i++)
		{
			printf("%d ",A[i]);
		}
		printf("\n");

		// printf("\nGive Matrix B\n");
		// for(i = 0;i<n;i++)
		// {
		// 	for(j=0;j<n;j++)
		// 	{
		// 		printf("\nEnter B[%d][%d] : ",i+1,j+1);
		// 		scanf("%d",&B[i+j]);
		// 	}
		// }
		// finding sizes to scatter
		for(i=0;i<dim_size;i++)
		{
			if(i < (n*n)%dim_size)
				sizes[i] = n*n/dim_size + 1;
			else
				sizes[i] = n*n/dim_size ;
			printf("size %d = %d\n",i,sizes[i]);
		}
		// finding offsets
		offsets[0] = 0;
		for(int i=1;i<dim_size;i++)
		{
			offsets[i] = offsets[i-1] + sizes[i-1];
		}

		dup2(fd,STDOUT_FILENO);
	}

	MPI_Barrier(comm);

	// sending and receiving data phase 1
	if(rank_2d == 0)
	{
		// sending data phase 1
		for(i=1;i<dim_size;i++)
		{
			MPI_Send(&sizes[i],1,MPI_INT,(dim_size)*i,0,comm);
			MPI_Send(A+offsets[i],sizes[i],MPI_INT,(dim_size)*i,0,comm);
		}
		
		// set own matrices
		N = sizes[0];

	}

	if(rank_2d % dim_size == 0 && rank_2d !=0 )
	{
		MPI_Recv(&N,1,MPI_INT,0,0,comm,&status);
		printf("Recived size is : %d",N);
		A = (int*)(malloc(sizeof(int)*N));
		MPI_Recv(A,N,MPI_INT,0,0,comm,&status);
		printf("\n");
		for(i=0;i<N;i++)
			printf("%d ",A[i]);
		printf("\n");
	}

	MPI_Barrier(comm);
	// sending and receiving data phase 2

	if(rank_2d % dim_size == 0)
	{
		if(sizes == NULL)
			sizes = (int*)(malloc(sizeof(int) * dim_size));
		if(offsets == NULL)
			offsets = (int*)(malloc(sizeof(int) * dim_size));
		for(i=0;i<dim_size;i++)
		{
			if(i < N % dim_size)
				sizes[i] = N/dim_size + 1;
			else
				sizes[i] = N/dim_size ;
			printf("size %d = %d\n",i,sizes[i]);
		}
		// finding offsets
		offsets[0] = 0;
		for(int i=1;i<dim_size;i++)
		{
			offsets[i] = offsets[i-1] + sizes[i-1];
		}
		for(i = 1;i<dim_size;i++)
		{
			printf("\nSending to %d",rank_2d+i);
			MPI_Send(&sizes[i],1,MPI_INT,rank_2d+i,0,comm);
			MPI_Send(A+offsets[i],sizes[i],MPI_INT,rank_2d+i,0,comm);
		}
	}
	else
	{
		MPI_Recv(&N,1,MPI_INT,(rank_2d/dim_size)*dim_size,0,comm,&status);
		printf("Recived size is : %d",N);
		A = (int*)(malloc(sizeof(int)*N));
		MPI_Recv(A,N,MPI_INT,(rank_2d/dim_size)*dim_size,0,comm,&status);
		printf("\n");
		for(i=0;i<N;i++)
			printf("%d ",A[i]);
		printf("\n");
	}

	// data is sent to all processes

	MPI_Finalize();

	return 0;

}