#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <bits/stdc++.h>

using namespace std;

void mul(int *a,int *b,int *c,int n)
{

	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			c[i*n + j] = 0;
			for(int k=0;k<n;k++)
				c[i*n + j] += a[i*n + k ]* b [k*n + j];
		}
	}
}

void sum(int *a,int *b,int n)
{
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			a[i*n + j] = a[i*n +j] + b [i*n + j];
		}
	}
}

void process(int rank)
{
	printf("\nProcess[%d] ",rank);
}

void print_cord(int *arr,int n)
{
	printf(" ");
	for(int i=0;i<n;i++)
	{
		printf("[%d]",arr[i]);
	}
	printf(" ");
}

void copy_(int *from,int *to,int n)
{
	for(int i=0;i<n;i++)
		to[i] = from[i];
}

void print_mat(int *arr,int n)
{
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			printf("%d\t",arr[i*n + j]);
		}
		printf("\n");
	}
}

void print_arr(int *arr,int n)
{
	for(int i=0;i<n;i++)
		printf("%d ",arr[i]);
}

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
		printf("\nInput\n");
		for(i = 0;i<n*n;i++)
		{
			printf("%d ",A[i]);
		}
		printf("\n");

		printf("\nGive Matrix B\n");
		for(i = 0;i<n;i++)
		{
			for(j=0;j<n;j++)
			{
				printf("\nEnter B[%d][%d] : ",i+1,j+1);
				scanf("%d",&B[i*n+j]);
			}
		}


		// finding sizes to scatter
		for(i=0;i<dim_size;i++)
		{
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
			MPI_Send(B+offsets[i],sizes[i],MPI_INT,(dim_size)*i,0,comm);
		}

		// set own matrices
		N = sizes[0];

	}

	if(rank_2d % dim_size == 0 && rank_2d !=0 )
	{
		MPI_Recv(&N,1,MPI_INT,0,0,comm,&status);
		A = (int*)(malloc(sizeof(int)*N));
		MPI_Recv(A,N,MPI_INT,0,0,comm,&status);
		B = (int*)(malloc(sizeof(int)*N));
		MPI_Recv(B,N,MPI_INT,0,0,comm,&status);
		printf("\nReceived A\n");
		print_arr(A,N);
		printf("\nRecevied B\n");
		print_arr(B,N);
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
				sizes[i] = N/dim_size ;
		}
		// set new A and B
		int *AA = (int*)(malloc(sizeof(int)*N));
		int *BB = (int*)(malloc(sizeof(int)*N));
		//	printf("\nDims size = %d\n",dim_size);
		for(int i=0,j=0;j < N; j += dim_size ,i = i + dim_size* dim_size)
		{
			int index = (i) % N  + ( i / N * dim_size);
			copy_(A+index,AA+j,dim_size);
			copy_(B+index,BB+j,dim_size);
		}
		// for(int i=0;i<N;i += dim_size)
		// {
		// 	int index = ( i * dim_size * dim_size ) % ( N / dim_size) + ( i * dim_size * dim_size ) / ( N / dim_size);
		// 	copy_(A+index,AA+i*dim_size,dim_size);
		// 	copy_(B+index,BB+i*dim_size,dim_size);
		// }

		printf("\nAA is \n");
		print_arr(AA,N);
		printf("\nBB is \n");
		print_arr(BB,N);

		A = AA;
		B = BB;
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
			MPI_Send(B+offsets[i],sizes[i],MPI_INT,rank_2d+i,0,comm);
		}
		// set own matrices
		N = sizes[0];
	}
	else
	{
		MPI_Recv(&N,1,MPI_INT,(rank_2d/dim_size)*dim_size,0,comm,&status);
		A = (int*)(malloc(sizeof(int)*N));
		MPI_Recv(A,N,MPI_INT,(rank_2d/dim_size)*dim_size,0,comm,&status);
		B = (int*)(malloc(sizeof(int)*N));
		MPI_Recv(B,N,MPI_INT,(rank_2d/dim_size)*dim_size,0,comm,&status);
	}

	// data is sent to all processes

	printf("\nProcess[%d] have received\n",rank_2d);
	printf("A\n");
	print_mat(A,(int)(sqrt(N)));
	printf("\n");
	printf("B\n");
	print_mat(B,(int)(sqrt(N)));
	printf("\n");

	int my_right,my_left,my_up,my_down;

	MPI_Cart_shift(comm,1,-1,&my_right,&my_left);		// my_left will send me if I am destination
	MPI_Cart_shift(comm,0,-1,&my_down,&my_up);			// my_up will send me if I am destination

	int cords[2];
	MPI_Cart_coords(comm,my_right,2,cords);
	printf("\nMy right is : ");
	print_cord(cords,2);
	printf("\nMy left is : ");
	MPI_Cart_coords(comm,my_left,2,cords);
	print_cord(cords,2);
	printf("\nMy Up is : ");
	MPI_Cart_coords(comm,my_up,2,cords);
	print_cord(cords,2);
	printf("\nMy down is : ");
	MPI_Cart_coords(comm,my_down,2,cords);
	print_cord(cords,2);
	printf("\nMy coords ");
	print_cord(my_coords,2);
	printf("\n");

	// initial shift

	// for mat A

	int to,from;
	MPI_Cart_shift(comm,1,my_coords[0],&to,&from);
	printf("\nSending A to : ");
	MPI_Cart_coords(comm,to,2,cords);
	print_cord(cords,2);
	printf("\nReceiving A from : ");
	MPI_Cart_coords(comm,from,2,cords);
	print_cord(cords,2);
	MPI_Sendrecv_replace(A,N,MPI_INT,to,0,from,0,comm,&status);

	//for mat B
	MPI_Cart_shift(comm,0,my_coords[1],&to,&from);
	printf("\nSending A to : ");
	MPI_Cart_coords(comm,to,2,cords);
	print_cord(cords,2);
	printf("\nReceiving A from : ");
	MPI_Cart_coords(comm,from,2,cords);
	print_cord(cords,2);
	MPI_Sendrecv_replace(B,N,MPI_INT,to,0,from,0,comm,&status);

	// printing data after initial shifts

	printf("\nMy data After initial shifts\n");
	printf("A\n");
	print_mat(A,(int)(sqrt(N)));
	printf("\n");
	printf("B\n");
	print_mat(B,(int)(sqrt(N)));
	printf("\n");

	int *temp_c = (int*)(malloc(sizeof(int)*N));
	int *C = (int*)(malloc(sizeof(int)*N));
	for(int i=0;i<(int)(sqrt(N));i++)
	{
		for(int j=0;j<sqrt(N);j++)
		{
			temp_c[i*((int)sqrt(N)) + j] = C [ i*((int)sqrt(N)) + j ]= 0; 
		}
	}


	for(int i=0;i<dim_size;i++)
	{
		mul(A,B,temp_c,(int)(sqrt(N)));
		printf("\nC in iteration Before %d is \n",i);
		print_mat(C,(int)(sqrt(N)));
		sum(C,temp_c,(int)(sqrt(N)));
		printf("\nC in iteration %d is \n",i);
		print_mat(C,(int)(sqrt(N)));
		MPI_Sendrecv_replace(A,N,MPI_INT,my_left,0,my_right,0,comm,&status);
		printf("\nReceived A in iteration int %d \n",i);
		print_mat(A,(int)(sqrt(N)));
		MPI_Sendrecv_replace(B,N,MPI_INT,my_up,0,my_down,0,comm,&status);
		printf("\nReceived B in iteration int %d \n",i);
		print_mat(B,(int)(sqrt(N)));
	}


	MPI_Finalize();

	return 0;

}
