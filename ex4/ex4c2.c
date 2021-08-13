/*************************************************************

File: ex4b2.c

Written by:
David Sapir, id = 208917351, login = davidsa
Shimshon Polak, id = 315605642, login = shimshonpo


  Name: Server application

The program check if the giving str by messege queue is palindrom
or prime numbe from the cliend,before doing the checking the data,
the server check if the giving client is registered, if not the server
return -1 answer.

input:operation type
1 - check if the number is prime or not.
2 - check if the giving str is palindrom.

output:according to the operation
1 - prime or not prime.
2 - palindrom or not palindrom. 
user not registered - "-1"
 
 
 **************************************************************/

#include <stdio.h>       // for input, output
#include <stdlib.h>      // for exit.
#include <stdbool.h> 	//for bool.
#include <signal.h> 
#include <unistd.h>   //for getpid().
#include <sys/types.h>	  //
#include <sys/ipc.h>	
#include <sys/msg.h> 
#include <string.h>


#define MAX_LEN 100


//--------------- struct section ------------------		  
struct register_data
{
	pid_t m_pid;
	int recived_msg;
};

struct app_data
{
	pid_t m_pid;
	char mtext[MAX_LEN];
};

struct my_app_msgbuf
{
	long mtype ;
	struct app_data data;
};

struct my_register_msgbuf
{
	long mtype ;
	struct register_data data;
};


//--------------- prototype section ------------------
void create_public_key( key_t *key,const char name);

void create_private_key(int *msgid,key_t key);

void start_application_server(struct my_register_msgbuf sign_msg ,
 struct my_app_msgbuf client_msg);

int is_client_sign(struct my_register_msgbuf client_msg);

void signal_handler(int signal_num);

bool is_prime(char *str);

bool ispalindrom(char *str);

void delete_msg_queue(int msgid);

void msgrcv_error();

void msgsnd_error();
//------------------ consts section --------------------

const int NUMS_PID = 5,
SIGNED = 1 ,NOT_SIGNED = -1,
PALINDROM_OPERATION = 2,
PRIME_OPERATION =1,
MAX_STR_LEN=100;
const char 
APP_SERVER_KEY= 'd',SIGN_SERVER = 'c',
PRIME[]="prime",NOT_PRIME[] = "not prime",
PALINDROM[]="palindrom",NOT_PALINDROM[] = "not palindrom" ;      // range of the numbers.


//------------------ global section --------------------
int msgid_sign_server,apllication_id;

//--------------- main section ------------------
int main(){
	key_t key_sign_server,
	key_apllication_server;
	struct my_register_msgbuf sign_msg;
	struct my_app_msgbuf client_msg;
	
	signal(SIGINT,signal_handler);
	
	create_public_key(&key_apllication_server,APP_SERVER_KEY);
	
	create_public_key(&key_sign_server,SIGN_SERVER);
	
	create_private_key(&msgid_sign_server,key_sign_server);
	
	apllication_id = msgget(key_apllication_server, IPC_CREAT | IPC_EXCL | 0600);
	
	if(apllication_id==-1)
	{
		perror("msgget() failed");
		exit(EXIT_FAILURE);
	}
	
	start_application_server(sign_msg,client_msg);
	
	return EXIT_SUCCESS;
}

/****************** create_public_key **********************
 * Create public key for the shared memory.
 * Params:
 * key- public key pointer.
 * ******************************************************/
void create_public_key( key_t *key,const char name){
	if(((*key) = ftok(".",name))==-1)
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
	//(*msg_id) = msgget(key,0);
	(*msg_id)= msgget(key, 0);
	
	if((*msg_id)==-1)
	{
		perror("msgget() failed");
		exit(EXIT_FAILURE);
	}
}

/******************* start_application_server *******************
 * start the application functinallty, if the type recived is
 * 1 - check if given str is a prime number.
 * 2 - check if the given str is a palindrom.
 * return the client answer about the data.
 *****************************************************/
void start_application_server(struct my_register_msgbuf sign_msg ,
 struct my_app_msgbuf client_msg){
	int client_signed;
	
	while(1){

		if(msgrcv(apllication_id,&client_msg,sizeof(struct app_data),0,0)==-1)
			break;
		
		sign_msg.data.m_pid = client_msg.data.m_pid;
		if(sign_msg.data.m_pid!= 0){
			
			client_signed = is_client_sign(sign_msg);
			
			if(client_signed == 1 ){
				
				switch(client_msg.mtype){
					case 1:
						if(is_prime(client_msg.data.mtext))
							strcpy(client_msg.data.mtext,PRIME);
						else
							strcpy(client_msg.data.mtext,NOT_PRIME);
					
						break;
					
					case 2:
						if(ispalindrom(client_msg.data.mtext))
							strcpy(client_msg.data.mtext,PALINDROM);
						else
							strcpy(client_msg.data.mtext,NOT_PALINDROM);
						break;	
					default:
						break;
				}
				
				client_msg.mtype = client_msg.data.m_pid;
				if(msgsnd(apllication_id, &client_msg,sizeof(struct app_data), 0)==-1){
					break;
				}
				
			}else{
				puts("set -1");
				client_msg.mtype = client_msg.data.m_pid;
				strcpy(client_msg.data.mtext,"-1");
				if(msgsnd(apllication_id, &client_msg,sizeof(struct app_data), 0)==-1){
					break;
				}
				
			}
			sleep(1);
			client_msg.mtype = 0;
		}
	}
		
}

/******************* is_client_sign *******************
 * send messeges to register to see if 
 * the client is signed in
 *****************************************************/
int is_client_sign(struct my_register_msgbuf client_msg){
	//****************************************************************
	
		client_msg.mtype = 2;
		
		//send to sign server.
		if(msgsnd(msgid_sign_server,&client_msg,
		sizeof(struct register_data),0)==-1)
			msgsnd_error();
		
		//recive msg from server.
		if(msgrcv(msgid_sign_server,&client_msg,
		sizeof(struct register_data),0,0)==-1)
			msgrcv_error();
		
		client_msg.mtype = 0;
		if(client_msg.data.recived_msg == 1){
			return SIGNED;
		}
		
		return NOT_SIGNED;
		
}

/******************* signal_handler *******************
 * delete messege quque application.
 *****************************************************/
void signal_handler(int signal_num){
	delete_msg_queue(apllication_id);
}

/******************* is_prime ************************
 * check if the str is prime number
 *****************************************************/
bool is_prime(char *str){
	int num = atoi(str), i;
	
	for (i = 2; i <= num / 2; ++i) {

        // condition for non-prime
        if (num % i == 0) {
            return false;
        }
    }
	return true;
}

/****************** ispalindrom **********************
 * check if string is palindrom
 *****************************************************/
bool ispalindrom(char *str){
    int l = 0; 
    int h = strlen(str) - 1; 
  
    while (h > l) 
    { 
        if (str[l++] != str[h--]) 
        { 
            return false; 
        } 
    } 
   return true;
} 

/****************** delete_shm **********************
 * delete the shared memory pointer and data.
 *******************************************************/
void delete_msg_queue(int msgid){
	
	if (msgctl ( msgid, IPC_RMID, NULL ) == -1 )
	{
		perror("msgctl() failed") ;
		exit(EXIT_FAILURE) ;
	}
}

void msgrcv_error(){
	perror("msgrcv() failed");
	exit(EXIT_FAILURE);
}

void msgsnd_error(){
	perror("msgsnd() failed");
	exit(EXIT_FAILURE);
}


