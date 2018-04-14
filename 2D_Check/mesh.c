#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>


int main(int argc,char* argv[])
{
	int rank,nps,rank_2d,fd,src,dest;
	MPI_Comm comm;
	int dim_sizes[2];
	int periods[2];
	int my_coords[2];
	char file_name[10];

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nps);	

	sprintf(file_name,"output%d.txt",rank);

	fd = open(file_name,O_WRONLY | O_CREAT | O_TRUNC, 0666);

	dup2(fd,STDOUT_FILENO);

	dim_sizes[0] = dim_sizes[1] = sqrt(nps);
	periods[0] = periods[1] = 1;	// wrap around

	MPI_Cart_create(MPI_COMM_WORLD,2,dim_sizes,periods,1,&comm);

	MPI_Comm_rank(comm,&rank_2d);
	printf("\n[Process %d] 2D rank = %d\n",rank,rank_2d);
	MPI_Cart_coords(comm,rank_2d,2,my_coords);
	printf("\n[Process %d] My Coordinates are : %d %d \n",rank,my_coords[0],my_coords[1]);
	MPI_Cart_rank(comm,my_coords,&rank_2d);
	printf("\n[Process %d] 2D rank is : %d\n",rank,rank_2d);

	MPI_Cart_shift(comm,1,1,&src,&dest);
	MPI_Cart_coords(comm,src,2,my_coords);
	printf("\n[Process %d] on left (source) : %d %d\n",rank_2d,my_coords[0],my_coords[1]);
	MPI_Cart_coords(comm,dest,2,my_coords);
	printf("\n[Process %d] on left (dest) : %d %d\n",rank_2d,my_coords[0],my_coords[1]);

	MPI_Cart_shift(comm,1,-1,&src,&dest);
	MPI_Cart_coords(comm,src,2,my_coords);
	printf("\n[Process %d] on right (source) : %d %d\n",rank_2d,my_coords[0],my_coords[1]);
	MPI_Cart_coords(comm,dest,2,my_coords);
	printf("\n[Process %d] on right (dest) : %d %d\n",rank_2d,my_coords[0],my_coords[1]);

	MPI_Cart_shift(comm,0,1,&src,&dest);
	MPI_Cart_coords(comm,src,2,my_coords);
	printf("\n[Process %d] on my up (source) : %d %d\n",rank_2d,my_coords[0],my_coords[1]);
	MPI_Cart_coords(comm,dest,2,my_coords);
	printf("\n[Process %d] on my up (dest) : %d %d\n",rank_2d,my_coords[0],my_coords[1]);

	MPI_Cart_shift(comm,0,-1,&src,&dest);
	MPI_Cart_coords(comm,src,2,my_coords);
	printf("\n[Process %d] on my down (sourc) : %d %d\n",rank_2d,my_coords[0],my_coords[1]);
	MPI_Cart_coords(comm,dest,2,my_coords);
	printf("\n[Process %d] on my down (dest) : %d %d\n",rank_2d,my_coords[0],my_coords[1]);
	MPI_Finalize();

	return 0;

}