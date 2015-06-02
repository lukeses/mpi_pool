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
    int gender;
    int process_id;
    };


// as list TO DO making a list
int list[2000];
int i = 0;

MPI_Datatype mpi_pool_message;
MPI_Status status;
int sender;
int rank;
int timestamp;


struct request_item *tmp;
struct request_item *m_temp, *min;
struct list_head *pos, *q;
struct request_item mylist;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;;
pthread_mutexattr_t attr;

int locker_rooms[3];
int who_in_locker_rooms[3]; // 0 - nikt, 1 - kobieta, 2 - mezczyzna
int counter=0;
int gender;


void* messanger(void* _arg){

//  printf("jestem w messenger");
    request recv;

    //sender +1 aby byla inna liczba niz sender i dzieki temu odbierane byly wiadomosci
    //MPI_Bcast(&recv,   1, mpi_pool_message, sender, MPI_COMM_WORLD);
    while(1){
      MPI_Recv(&recv, 1, mpi_pool_message, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
//      printf("Rank %d: Type: %d lamport_clock = %d process_id = %d\n", rank, recv.type, recv.lamport_clock, recv.process_id);

      if (recv.type == 1001){
        pthread_mutex_lock(&mutex);  
        tmp= (struct request_item *)malloc(sizeof(struct request_item));
        tmp->type = recv.type;
        tmp->lamport_clock = recv.lamport_clock;
        tmp->gender = recv.gender;
        tmp->process_id = recv.process_id;
        list_add(&(tmp->list), &(mylist.list));
        pthread_mutex_unlock(&mutex);
printf("GENDER: %d\n", tmp->gender);


        request send;
        send.type = 2001;
        send.lamport_clock = timestamp;
        send.process_id = rank;
        send.gender = gender; 
        MPI_Send(&send, 1, mpi_pool_message, recv.process_id, MSG_REQUEST, MPI_COMM_WORLD);
        //printf("Wyslalem MSG_REQUEST 1001\n");
      }
      else if(recv.type == 2001){
        //printf("Odebralem MSG_REQUEST 1001\n");
        //TO DO liczenie ile requestow dostalem
        pthread_mutex_lock(&mutex); 
        counter++;
        pthread_mutex_unlock(&mutex);
      }
      else if(recv.type == 3001){
        //printf("Odebralem MSG_REQUEST 3001\n");
        pthread_mutex_lock(&mutex);  

        // list_for_each_safe(pos, q, &mylist.list){
        //      tmp= list_entry(pos, struct request_item, list);
        //      //printf("freeing item to= %d from= %d\n", m_temp->to, m_temp->from);
        //      if(tmp->process_id == recv.process_id){
        //         list_del(pos);
        //         free(tmp);
        //         printf("Usunalem gnoja");
        //     }
        // }
      list_for_each(pos, &mylist.list){
         tmp= list_entry(pos, struct request_item, list);
         //printf("PRZED rank= %d gender= %d process_id= %d\n", rank, tmp->gender, tmp->process_id);
      }

        list_for_each_safe(pos, q, &mylist.list){
           m_temp= list_entry(pos, struct request_item, list);
           //printf("m_temp przed usunieciem - lamport: %d , process_id: %d\n", m_temp->lamport_clock, m_temp->process_id);
           //printf("recv przed usunieciem - lamport: %d , process_id: %d\n", recv.lamport_clock, recv.process_id);
           //printf("freeing item to= %d from= %d\n", m_temp->to, m_temp->from);
           if(m_temp->process_id == recv.process_id && m_temp->lamport_clock == recv.lamport_clock){
              list_del(pos);
              free(m_temp);
              //printf("Usunalem gnoja\n");
          }
        }

        int lock_num;
        for(lock_num = 0; lock_num < 3; lock_num ++){
          if(locker_room_available(lock_num)){
            who_in_locker_rooms[lock_num] = recv.gender;
            locker_rooms[lock_num]--;
            break;
          }   
        }

        pthread_mutex_unlock(&mutex);

      }
      printf("\n");
      // list_for_each(pos, &mylist.list){
      //    tmp= list_entry(pos, struct request_item, list);
      //    printf("rank= %d gender= %d process_id= %d\n", rank, tmp->gender, tmp->process_id);
      // }



    }
  return 0;
} 

int locker_room_available(int num){
  if(locker_rooms[num] > 0 && (who_in_locker_rooms[num] == gender || who_in_locker_rooms[num] == 0))
    return 1;
  else
    return 0;
}


int any_locker_room_available(){
//  printf("%d %d %d", locker_room_available(0), locker_room_available(1), locker_room_available(2));
  if(locker_room_available(0) || locker_room_available(1) || locker_room_available(2))
    return 1;
  else
    return 0;

}

int main( int argc, char **argv )
{
  int size;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int namelen;
  int i;

  timestamp = 0;

  int provided;
  int M = 5; //ilosc wolnych szafek w kazdej z szatni
  int P = 2*M; // 2/3 ilosci wszystkich szafek
  int my_locker_room; //zmienna do przechowywania szatni
  int where_i_am = 0; //0 - przed wejsciem do szatni, 1 - szatnia, 2 - prysznic, 3 - basen
  for(i = 0; i < 3; i++){
    locker_rooms[i] = M;
    who_in_locker_rooms[i] = 1;
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
    const int nitems=4;
    int          blocklengths[4] = {1,1,1,1};
    MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[4];

    offsets[0] = offsetof(request, type);
    offsets[1] = offsetof(request, lamport_clock);
    offsets[2] = offsetof(request, process_id);
    offsets[3] = offsetof(request, gender);
    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_pool_message);
    MPI_Type_commit(&mpi_pool_message);

//ustawienie plci
  gender = 1;//rank % 2 + 1;



  int msg[MSG_SIZE];


  pthread_t message_thread;
  pthread_create(&message_thread,NULL,messanger,NULL); 

  pthread_mutex_lock(&mutex);
  //zwiekszenie zegaru lamporta
  timestamp++;

  //dodanie zadania do wlasnej kolejki
  tmp= (struct request_item *)malloc(sizeof(struct request_item));
  tmp->type = 1001;
  tmp->lamport_clock = timestamp;
  tmp->gender = gender;
  tmp->process_id = rank;
  list_add(&(tmp->list), &(mylist.list));


  int j = 0;
//wyslanie zadania do innych procesow
  sender = rank;
  if ( rank == sender){
        request send;
        send.type = 1001;
        send.lamport_clock = timestamp;
        send.process_id = rank;
        send.gender = gender;
  pthread_mutex_unlock(&mutex);
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


  //oczekiwanie na spelnienie warunkow
  min= (struct request_item *)malloc(sizeof(struct request_item));

  min->lamport_clock = 9999999;

  min->process_id = 99999999;
  printf("Gender: %d\n", gender);
  while(!(counter == size-1 && any_locker_room_available() == 1 && min->process_id == rank)){
  sleep(rank);
  min= (struct request_item *)malloc(sizeof(struct request_item));
  min->lamport_clock = 9999999;
  min->process_id = 99999999;

    //printf("%d %d %d %d \n", gender, who_in_locker_rooms[0], who_in_locker_rooms[0], who_in_locker_rooms[0]);
      pthread_mutex_lock(&mutex);
        list_for_each(pos, &mylist.list){
          m_temp= list_entry(pos, struct request_item, list);
          if(m_temp->lamport_clock < min->lamport_clock){
            min = m_temp;
          }
          else if(m_temp->lamport_clock == min->lamport_clock && m_temp->process_id < min->process_id){
            min = m_temp;
          }
          //printf("type= %d lamport_clock= %d\n", tmp->type, tmp->lamport_clock);
      }
        pthread_mutex_unlock(&mutex);
  }
  printf("WCHODZE!! %d %d \n", rank, gender);
  pthread_mutex_lock(&mutex);

  int lock_num;
  for(lock_num = 0; lock_num < 3; lock_num ++){
    if(locker_room_available(lock_num)){
      my_locker_room = lock_num;
      who_in_locker_rooms[lock_num] = gender;
      locker_rooms[lock_num]--;
      break;
    }   
  }

  pthread_mutex_unlock(&mutex);

  pthread_mutex_lock(&mutex);
  list_for_each_safe(pos, q, &mylist.list){
       m_temp= list_entry(pos, struct request_item, list);
       //printf("freeing item to= %d from= %d\n", m_temp->to, m_temp->from);
       if(m_temp->process_id == min->process_id && m_temp->lamport_clock == min->lamport_clock){
          list_del(pos);
          free(m_temp);
          //printf("a a tez usunalem co mialem\n");
      }
  }
  pthread_mutex_unlock(&mutex);


        request send;
        send.type = 3001;
        send.process_id = rank;
        send.gender = gender;
        for(j=0; j < size; j++){
          if(j != rank){
            MPI_Send(&send, 1, mpi_pool_message, j, MSG_REQUEST, MPI_COMM_WORLD);
          }
      }

while(1){
    sleep(2);
}


  pthread_join(message_thread,NULL);  
  




  MPI_Finalize();
}
