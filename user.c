#include "mpi.h"
#include "lamport.c"
#include <stdio.h>

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>


#define MSG_SIZE 2

// as list TO DO making a list
int list[2000];
int i = 0;

void* messanger(void* _arg){
    request recv;

  MPI_Bcast(&recv,   1, mpi_pool_message, sender, MPI_COMM_WORLD);
  printf("Rank %d: Type: %d lamport_clock = %d process_id = %d\n", rank,
           recv.type, recv.lamport_clock, recv.process_id);

  return 0;
} 



int main( int argc, char **argv )
{
	int rank, size;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int namelen;
  int sender;

  int timestamp = 0;


	MPI_Init( &argc, &argv );
	
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	MPI_Get_processor_name(processor_name,&namelen);
	printf( "Jestem %d z %d na %s\n", rank, size, processor_name );

    /* create a type for struct car */
    const int nitems=3;
    int          blocklengths[3] = {1,1,1};
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Datatype mpi_pool_message;
    MPI_Aint     offsets[3];

    offsets[0] = offsetof(request, type);
    offsets[1] = offsetof(request, lamport_clock);
    offsets[2] = offsetof(request, process_id);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_pool_message);
    MPI_Type_commit(&mpi_pool_message);






  int msg[MSG_SIZE];


  pthread_t message_thread;
  pthread_create(&message_thread,NULL,messanger,NULL); 


while(1){
  //chec wejscia do szatni

  //zwiekszenie zegaru lamporta
  timestamp++;
  //dodanie zadania do wlasnej kolejki
  list[i] = timestamp;
  //wyslanie zadania do innych procesow
  request send;
  send.type = 1001;
  send.lamport_clock = timestamp;
  send.process_id = rank;

  MPI_Bcast(&send, 1, mpi_pool_message, rank, MPI_COMM_WORLD);

  //printf("Rank %d: sent structure car\n", rank);
}





  sender = 0;
  if ( rank == sender){
        request send;
        send.type = 1001;
        send.lamport_clock = timestamp;
        send.process_id = 15;

        MPI_Bcast(&send, 1, mpi_pool_message, sender, MPI_COMM_WORLD);

        printf("Rank %d: sent structure car\n", rank);
  }
  else
  {
        request recv;

        MPI_Bcast(&recv,   1, mpi_pool_message, sender, MPI_COMM_WORLD);
        printf("Rank %d: Type: %d lamport_clock = %d process_id = %d\n", rank,
                 recv.type, recv.lamport_clock, recv.process_id);
  }



	MPI_Finalize();
}
