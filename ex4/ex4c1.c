/*************************************************************

file: ex4c1.c

		     Register server program
            ===========================================
Written by:
David Sapir, id = 208917351, login = davidsa
Shimshon Polak, id = 315605642, login = shimshonpo

The program register manage users in the server,
if user requested to sign up the server will register him
if he is already register the server will send messege he is already
registered,if no space left the server will notify about that, 
the server operate according to the following requsted type.
'1' - register the user/pid.
'2' - check if the giving pid exist.
'3' - sign out the givin pid.
 
input:
type of command.

output:according to the requsted operation
register the user: 
   0-user added succseffuly,
   1-user already exist,
   2-no space
   check if user exits:
   0 - not existed.
   1 - existed. 
 *************************************************************/

//--------------- include section ------------------	
#include <stdio.h>       // for input, output
#include <stdlib.h>      // for exit.
#include <stdbool.h> 	//for bool.
#include <signal.h> 
#include <sys/types.h>	  //
#include <sys/ipc.h>	
#include <sys/msg.h> 
#include <sys/wait.h>
#include <unistd.h> 	//for sleep
#include <string.h>



#define MAX_LEN 100


//--------------- struct section ------------------		  
struct register_data
{
	pid_t m_pid;
	int recived_msg;
};

struct my_register_msgbuf
{
	long mtype ;
	struct register_data data;
};



//--------------- prototype section ------------------
void create_public_key( key_t *key);

void create_private_key(int *shm_id,key_t key);

void sign_server(int msg_id,struct my_register_msgbuf);

int add_new_process(int pid_arr[],int pid);

int check_if_existed_pid(int pid_arr[],int pid);

void remove_process(int pid_arr[],int pid);

void delete_msg_queue(int msgid);

void intilize_pid_arr( pid_t pid[] );

void signal_handler(int signal_num);

//------------------ consts section --------------------

const int NUMS_PID = 5,
NOT_EXISTED = 0,
ADDED_PID = 0,
EXISTED = 1,
FULL_PIDS = 2,      // range of the numbers.
FREE_SPACE = -1;

int msgid;

//--------------- main section ------------------
int main(){
	key_t key;
	struct my_register_msgbuf my_msg;
	
	create_public_key(&key);
	
	create_private_key(&msgid,key);
	
	sign_server(msgid,my_msg);
	
	return EXIT_SUCCESS;
}

/****************** create_public_key **********************
 * Create public key for the shared memory.
 * Params:
 * key- public key pointer.
 * ******************************************************/
void create_public_key( key_t *key){
	if(((*key) = ftok(".",'c'))==-1)
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

/****************** sign_new_process **********************
 *********************************************************/
void sign_server(int msgid,struct my_register_msgbuf my_msg){
	pid_t pid_arr[NUMS_PID] ;
	
	intilize_pid_arr(pid_arr);
	
	signal(SIGINT,signal_handler);
	
	while(1){
		
		if(msgrcv(msgid,&my_msg,sizeof(struct register_data),0,0)==-1){
			perror("msgsrcv() failed");
			exit(EXIT_FAILURE);
		}
		
		if(my_msg.data.m_pid > 0){
			
			switch(my_msg.mtype){
			
				case 1:
					my_msg.data.recived_msg = add_new_process(pid_arr,my_msg.data.m_pid);
				break;
			
				case 2:
					my_msg.data.recived_msg = check_if_existed_pid(pid_arr,my_msg.data.m_pid);
				break;
			
				case 3:
					remove_process(pid_arr,my_msg.data.m_pid);
				break;
			
			}
		}
		
		if(msgsnd(msgid,&my_msg,sizeof(struct register_data),0)==-1){
			perror("msgssnd() failed");
			exit(EXIT_FAILURE);
		}
		sleep(1);
	}
}


/****************** add_new_process********************
 * add new process to the register server
 * params:
 * pid_arr:array of stored process.
 * pid: registered pid.
 * output:
 * 0 - user added
 * 1 - user already existed
 * 2 - no space left.
 ******************************************************/
int add_new_process(int pid_arr[],int pid){
	int i=0;
	if(check_if_existed_pid(pid_arr,pid)!= 1){
		
		for(i = 0 ;i<NUMS_PID;i++){
			
			if(pid_arr[i] == -1){
				pid_arr[i] = pid;
				return ADDED_PID;
			}
			
		}
		
		return FULL_PIDS;
	}
	
	return EXISTED;
}


/****************** check_if_existed_pid********************
 * check if the user existec in the server.
 * params:
 * pid_arr:array of stored process.
 * pid: registered pid.
 * 
 * output:
 * 0 - user exist.
 * 1 - user not exist.
 ******************************************************/
int check_if_existed_pid(int pid_arr[],int pid){
	int i=0;
	for(i = 0 ;i<NUMS_PID;i++){
				
	if(pid_arr[i] == pid)
		return EXISTED;		
	}
	return NOT_EXISTED;
}


/****************** remove_process ********************
 * remove the giving user pid from server.
 * params:
 * pid_arr:array of stored process.
 * pid: registered pid.
 ******************************************************/
void remove_process(int pid_arr[],int pid){
	int i;
	for(i = 0 ;i<NUMS_PID;i++){
			
			if(pid_arr[i] == pid){
				pid_arr[i] = -1;
			}
				
	}
}


/****************** delete_shm *************************
 * delete the shared memory pointer and data.
 ********************************************************/
void delete_msg_queue(int msgid){
	
	if (msgctl ( msgid, IPC_RMID, NULL ) == -1 )
	{
		perror("msgctl() failed") ;
		exit(EXIT_FAILURE) ;
	}


}


/****************** intilize_pid_arr *************************
 * set defult values in pid_arr.
 ********************************************************/
void intilize_pid_arr( pid_t pid[] ){
	int i=0;
	
	for(i=0;i < NUMS_PID;i++)
		pid[i] = FREE_SPACE;
	
}


/****************** signal_handler *************************
 * close the server and delete the messsege queue.
 ********************************************************/
void signal_handler(int signal_num){
	delete_msg_queue(msgid);
	exit(EXIT_SUCCESS);
}


