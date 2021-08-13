/*********************************************************		

file: ex4b1.c

              Bingo numbers holder
====================================================  
Written by:
David Sapir, id = 208917351, login = davidsa
Shimshon Polak, id = 315605642, login = shimshonpo

The program recive numbers with the messege queue from N programs 
and send  them is the apeared in the array of number 
that was stored it and print in the end how many number
were correct , how many number have been recived by the player ,
and how much time it took to guess all the numbers.

 Input: none
 
 Output:
 -number been sent to ex4b1.c program.
 -number of right guesses.
 -time to solve the bingo.

****************************************************************/
//---------------- include section ---------------------
#include <stdio.h>       // for input, output
#include <stdlib.h>      // for exit.
#include <sys/types.h>
#include <sys/wait.h>	 // for wait()
#include <unistd.h>       
#include <signal.h>       // for signals.
#include <stdbool.h>      // for bool
#include <errno.h>        // for error.
#include <time.h>         // for time.

#include <sys/types.h>	  //
#include <sys/ipc.h>	
#include <sys/msg.h> 	// messege queues


		  
struct msg_data{
	int m_pid;
	int m_rnd_num;
	int good_guess;
	int stop;
};



struct my_msgbuf
{
long mtype ;
struct msg_data data;
} ;

//--------------- prototype section ------------------
void makeArrayOfRandomNums(int arr[]);

void create_public_key( key_t *key);

void create_private_key(int *shm_id,key_t key);

void makeArrayOfRandomNums(int arr[]);

void wait_to_start(struct my_msgbuf *m_msg,int pid_arr[],int msgid);

void start_bingo(struct my_msgbuf *m_msg,int msgid ,int arr_nums[]);

bool number_is_found(int arr[],const int num);

void delete_msg_queue(int num_signal);

void check_in_arr(int *arr,struct my_msgbuf my_data, int *new_size, 
										int * timer);
//------------------ consts section --------------------

const int NUMS_RANGE = 200000,       // range of the numbers.
          NUM_OF_PROGRAMS = 3,          // num of sons.
          ARRAY_SIZE = 50000,        // the array size
		  MSG_SIZE = 1000;
const long TYPE = 4;		  
		  

//--------------- global variables -------------------
bool is_sigterm = false;

//---------------------- main section ---------------------------
int main (int argc, char **argv){	     
   	key_t key;  
   	int random_arr_nums[ARRAY_SIZE],msgid,
   	arr_pid[] = {0,0,0};
   	struct my_msgbuf m_msg; 
   	
   		
   	if (argc < 2) {
		fputs("bad vector argument input", stderr);
		exit(EXIT_FAILURE);
	}
	
	srand(atoi(argv[1]));	
	
	m_msg.mtype = TYPE;
	m_msg.data.stop = 0;
	
	create_public_key(&key);
	
	create_private_key(&msgid,key);
	
	makeArrayOfRandomNums(random_arr_nums);
	
	wait_to_start(&m_msg,arr_pid,msgid);
	
	start_bingo(&m_msg, msgid ,random_arr_nums);
	
	//wait for the programs to print
	sleep(1);
	 
	delete_msg_queue(msgid);

    return EXIT_SUCCESS;
}


/****************** create_public_key **********************
 * Create public key for the shared memory.
 * Params:
 * key- public key pointer.
 * ******************************************************/
void create_public_key( key_t *key){
	if(((*key) = ftok(".",'4'))==-1)
	{
		perror("ftok() failed");
		exit(EXIT_FAILURE);
		
	}
	
}


/****************** create_private_key **********************
 * Create private key for the shared memory.
 * Params:
 * key- public key.
 * shm_id - private key pointer.
 * ******************************************************/
void create_private_key(int *msg_id,key_t key){
	(*msg_id)= msgget(key, IPC_CREAT | IPC_EXCL | 0600);
	
	if((*msg_id)==-1)
	{
		perror("msgget() failed");
		exit(EXIT_FAILURE);
	}
}


/****************** wait_to_start **********************
 * The function waiting for all the programs that are picking
 * a randoms number.
 * Params:
 * shm_ptr- pointer to the shared memory data.
 * pid_arr - array of the pids of all programs that takes part
 * in random picking numbers.
 * ******************************************************/
void wait_to_start(	struct my_msgbuf *m_msg ,int pid_arr[],int msgid){
	int i;
	for(i = 0;i<NUM_OF_PROGRAMS;i++){
			if(msgrcv(msgid,&(*m_msg),sizeof(struct msg_data),0,0)==-1){
				perror("msgrcv() failed");
				delete_msg_queue(msgid);
				exit(EXIT_FAILURE);
			}
			pid_arr[i] = (*m_msg).mtype;
	}	
		
	for(i = 0 ;i<NUM_OF_PROGRAMS;i++){
		(*m_msg).mtype = pid_arr[i];
		
		if(msgsnd(msgid,&(*m_msg),sizeof(struct msg_data),0)==-1){
			perror("msgsnd() failed");
			exit(EXIT_FAILURE);
		}
		
	}
	
}


/****************** start_bingo **********************
 * The function recive random numbers by messege queue.
 * if the the number given is exist count it and remove the number
 * from the array of numbers.when no number left,
 *  print how many numbers have been given
 * and how many have been good guess and how much time it took to 
 * guess all the numbers
 * Params:
 * my_msg -messege queue data .
 * arr_nums - array of random numbers.
 * ******************************************************/
void start_bingo(struct my_msgbuf * m_msg,int msgid ,int arr_nums[]){
	int start=0,end=0,num_left = ARRAY_SIZE ,correct_nums=0,num_recived=0,i;
	start = time(NULL);
	
	while (num_left > 0) {
		
			if(msgrcv(msgid,&(*m_msg),sizeof(struct msg_data),TYPE,0)==-1){
				perror("msgrcv() failed");
				exit(EXIT_FAILURE);
			}
			
			(*m_msg).data.good_guess = 0;
			
			if (number_is_found(arr_nums,(*m_msg).data.m_rnd_num)){
				(*m_msg).data.good_guess = 1;
				num_left -- ;
				correct_nums++;
			}
			
			num_recived++;
			
			(*m_msg).mtype = (*m_msg).data.m_pid;
			
			if(msgsnd(msgid,&(*m_msg),sizeof(struct msg_data),0)==-1){
				perror("msgsnd() failed");
				exit(EXIT_FAILURE);
			}
	}
	
	end=time(NULL);
	
	printf("%d  %d %d \n",num_recived,correct_nums,(end - start));
	
	(*m_msg).data.stop = 1;
	
	for(i = 1 ;i < NUM_OF_PROGRAMS + 1;i++){
		
		(*m_msg).mtype = i;
		if(msgsnd(msgid,&(*m_msg),sizeof(struct msg_data),0)==-1){
			perror("msgsnd() failed");
			exit(EXIT_FAILURE);
		}
	}
}


/****************** makeArrayOfRandomNums **********************
 This function inserts random numbers into the array.
 *  The numbers are in the range 0 - NUMS_RANGE.
 * ******************************************************/
void makeArrayOfRandomNums(int arr[]) {
    int i;
    
        for ( i = 0; i < ARRAY_SIZE; i++){
            arr[i] = rand() % NUMS_RANGE;
           
		}
		
}


/****************** number_is_found **********************
 * Check if the number exist in the arr , if does remove it,
 * if not return false.
 * ******************************************************/
bool number_is_found(int arr[],const int num){
	int i = 0;
	
	for (i = 0; i < ARRAY_SIZE; i++){
		if (arr[i] == num) {
			arr[i] = -1;
			return true;
		}
	}
	
	return false;
}


/****************** delete_shm **********************
 * delete the shared memory pointer and data.
 * ******************************************************/
void delete_msg_queue(int msgid){
	
	if (msgctl ( msgid, IPC_RMID, NULL ) == -1 )
	{
		perror("msgctl() failed") ;
		exit(EXIT_FAILURE) ;
	}
	exit(EXIT_SUCCESS) ;
}


