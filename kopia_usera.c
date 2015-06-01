#include "mpi.h"
#include "lamport.c"
#include <stdio.h>

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "list.h"

#define MSG_SIZE 2
#define MSG_REQUEST 100

struct request_item{
    struct list_head list;
    int type;
    int lamport_clock;
    };


// as list TO DO making a list
int list[2000];
int i = 0;

MPI_Datatype mpi_pool_message;
MPI_Status status;
int sender;
int rank;

struct request_item *tmp;
struct list_head *pos, *q;
struct request_item mylist;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;;
pthread_mutexattr_t attr;

int locker_rooms[3];
int who_in_locker_rooms[3]; // 0 - nikt, 1 - kobieta, 2 - mezczyzna



void* messanger(void* _arg){

//  printf("jestem w messenger");
    request recv;

    //sender +1 aby byla inna liczba niz sender i dzieki temu odbierane byly wiadomosci
    //MPI_Bcast(&recv,   1, mpi_pool_message, sender, MPI_COMM_WORLD);
    while(1){
      MPI_Recv(&recv, 1, mpi_pool_message, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
//      printf("Rank %d: Type: %d lamport_clock = %d process_id = %d\n", rank, recv.type, recv.lamport_clock, recv.process_id);
      pthread_mutex_lock(&mutex);  
      tmp= (struct request_item *)malloc(sizeof(struct request_item));
      tmp->type = recv.type;
      tmp->lamport_clock = recv.lamport_clock;

      list_add(&(tmp->list), &(mylist.list));
      pthread_mutex_unlock(&mutex);

      list_for_each(pos, &mylist.list){
         tmp= list_entry(pos, struct request_item, list);
         printf("type= %d lamport_clock= %d\n", tmp->type, tmp->lamport_clock);
      }



    }
  return 0;
} 



int main( int argc, char **argv )
{
  int size;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen;
  int i;

  int timestamp = 0;

  int provided;
  int M = 5; //ilosc wolnych szafek w kazdej z szatni
  int P = 2*M; // 2/3 ilosci wszystkich szafek
  int my_locker_room; //zmienna do przechowywania szatni
  for(i = 0; i < 3; i++){
    locker_rooms[i] = M;
  }


//semafory
  pthread_mutexattr_init(&attr);    
  pthread_mutex_init(&mutex, &attr);




  MPI_Init_thread( &argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );
  MPI_Get_processor_name(processor_name,&namelen);
//  printf( "Jestem %d z %d na %s\n", rank, size, processor_name );
    INIT_LIST_HEAD(&mylist.list);


    /* create a type for struct car */
    const int nitems=3;
    int          blocklengths[3] = {1,1,1};
    MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[3];

    offsets[0] = offsetof(request, type);
    offsets[1] = offsetof(request, lamport_clock);
    offsets[2] = offsetof(request, process_id);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_pool_message);
    MPI_Type_commit(&mpi_pool_message);






  int msg[MSG_SIZE];


  pthread_t message_thread;
  pthread_create(&message_thread,NULL,messanger,NULL); 


// while(1){
//   //chec wejscia do szatni

//   //zwiekszenie zegaru lamporta
//   timestamp++;
//   //dodanie zadania do wlasnej kolejki
//   list[i] = timestamp;
//   //wyslanie zadania do innych procesow
//   request send;
//   send.type = 1001;
//   send.lamport_clock = timestamp;
//   send.process_id = rank;

//   MPI_Bcast(&send, 1, mpi_pool_message, rank, MPI_COMM_WORLD);

//   //printf("Rank %d: sent structure car\n", rank);
// }

  pthread_mutex_lock(&mutex);
  //zwiekszenie zegaru lamporta
  timestamp++;

  //dodanie zadania do wlasnej kolejki
  tmp= (struct request_item *)malloc(sizeof(struct request_item));
  tmp->type = 1001;
  tmp->lamport_clock = timestamp;
  list_add(&(tmp->list), &(mylist.list));




  int j = 0;
//wyslanie zadania do innych procesow
  sender = rank;
  if ( rank == sender){
        request send;
        send.type = 1001;
        send.lamport_clock = timestamp;
        send.process_id = 15;
  pthread_mutex_unlock(&mutex);
        //MPI_Bcast(&send, 1, mpi_pool_message, sender, MPI_COMM_WORLD);
//MPI_Send( msg, MSG_SIZE, MPI_INT, receiver, MSG_HELLO, MPI_COMM_WORLD );
        for(j=0; j < size; j++){
          if(j != rank){
            MPI_Send(&send, 1, mpi_pool_message, j, MSG_REQUEST, MPI_COMM_WORLD);
          }
//        printf("Rank %d: sent structure car\n", rank);
      }
  }
  else{
    //dla pewnosci, gdyby if sie nie wykonal, chociaz w tej chwili musi bo sender == rank, dla pamieci o tym 
    pthread_mutex_unlock(&mutex);
  }
  // else
  // {
  //       request recv;

  //       MPI_Bcast(&recv,   1, mpi_pool_message, sender, MPI_COMM_WORLD);
  //       printf("Rank %d: Type: %d lamport_clock = %d process_id = %d\n", rank,
  //                recv.type, recv.lamport_clock, recv.process_id);
  // }

  //oczekiwanie na spelnienie warunkow
  




  pthread_join(message_thread,NULL);  
  




  MPI_Finalize();
}
