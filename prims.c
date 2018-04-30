#include <stdio.h>
#include <mpi.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

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
	int **a;
	int n;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&nps);

	sprintf(file_name,"output%d.txt",rank);

	fd = open(file_name,O_WRONLY | O_CREAT | O_TRUNC, 0666);

	if(rank == 0)
		temp_fd = dup(STDOUT_FILENO);

	dup2(fd,STDOUT_FILENO);

	// get input
	
	if(rank == 0)
	{
		printf("\nEnter : ");
		scanf("%d",&n);
		for(int i=0;i<)
	}

	return 0;

}
