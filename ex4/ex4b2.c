/****************************************************************
 
 file:ex4b2.c 

               Bingo player
====================================================  
Written by:
David Sapir, id = 208917351, login = davidsa
Shimshon Polak, id = 315605642, login = shimshonpo

The program pick a random number and send it by messege queue
to the numbers holder program ,and print in the end how many number
were correct ,and how many number have been recived by the player.

Input: none
 
Output:
-number been sent to ex4b1.c program.
-number of right guesses.
 
****************************************************************/


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

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h> 

//--------------- prototype section ------------------
void makeArrayOfRandomNums(int arr[]);

void create_public_key( key_t *key);

void create_private_key(int *msg_id,key_t key);

void ready_start(struct my_msgbuf *m_msg,int msgid);

void start_pick_random_nums(struct my_msgbuf *m_msg,int msgid,long type_num);

void start_handl(int sig_num);

void correct_num_handler(int sig_num);

void picked_num_handler(int signal);

void end_sending_handler(int sig_num);


//------------------ consts section --------------------
const int NUMS_RANGE = 200000; // range of the numbers.  
const long TYPE = 4;


//---------------------- main section ---------------------------
int main (int argc, char **argv){	     
   	key_t key;  
   	int msgid;
   	int seed_num;

   	struct my_msgbuf m_msg;  
   	
  	if (argc < 2) {
		fputs("bad vector argument input", stderr);
		exit(EXIT_FAILURE);
	}
	seed_num = atoi(argv[1]);
   	
   	srand(seed_num);
	
	create_public_key(&key);
	
	create_private_key(&msgid,key);
	
	m_msg.mtype = seed_num;
	m_msg.data.stop = 0;
	
	ready_start(&m_msg,msgid);
	
	start_pick_random_nums(&m_msg,msgid,seed_num);
    
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
 * msg_id - messege id of messege queue.
 * ******************************************************/
void create_private_key(int *msg_id,key_t key){
	(*msg_id) = msgget(key,0);
	
	if((*msg_id)==-1)
	{
		perror("msgget() failed");
		exit(EXIT_FAILURE);
	}
}


/****************** ready_to_start **********************
 * Announcing the player is on the ready to start and pick random numbers.
 * the function will wait until recive SIGUSER from the number holder.
 * Params:
 * my_msg- messege queue data.
 * msgid- messege id of messege queue.
 * ******************************************************/
void ready_start(struct my_msgbuf *m_msg,int msgid){
	
	if(msgsnd(msgid,&(*m_msg),sizeof(struct msg_data),0)==-1){
			perror("msgsnd() failed");
			exit(EXIT_FAILURE);
	}

	if((msgrcv(msgid,&(*m_msg),sizeof(struct msg_data),0,0)==-1)){
			perror("msgrcv() failed");
			exit(EXIT_FAILURE);
	}	
	
}


/****************** start_pick_random_nums ******************
 * Start pick a randoms number and sent them by the messege queue,
 *the function send the random number , and recive if the given number
 * was good.
 * in the end will be printer how many number have been sent 
 * and how many were good.
 * **********************************************************/
void start_pick_random_nums(struct my_msgbuf *m_msg,int msgid,long type_num){
	int good_picks =0;
	long number_of_picks = 0;
	while(1){
		
		(*m_msg).mtype = TYPE;
						
		(*m_msg).data.m_rnd_num = rand()%NUMS_RANGE;
				
		(*m_msg).data.m_pid = type_num;
				
		if(msgsnd(msgid,&(*m_msg),sizeof(struct msg_data),0) == -1){
			perror("msgsnd()faild:");
			exit(EXIT_FAILURE);
		}
				
		if(msgrcv(msgid,&(*m_msg),sizeof(struct msg_data),type_num,0) == -1){
			perror("msgrcv()faild:");
			exit(EXIT_FAILURE);
		}	

		number_of_picks++;
				
		if((*m_msg).data.stop == 1){
			break;
		}
				
		if((*m_msg).data.good_guess > 0){
			good_picks++;
			(*m_msg).data.good_guess=0;
		}		
	}
	printf("%ld %d \n",number_of_picks, good_picks);
	
	sleep(1);
}
