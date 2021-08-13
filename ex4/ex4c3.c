/*************************************************************

File: ex4c3.c

program name: Client program
====================================================
Written by:
David Sapir, id = 208917351, login = davidsa
Shimshon Polak, id = 315605642, login = shimshonpo

description:
 * The program communicate with register,and application server.
 * the program first register the client(process) in the register
 * server and then wait for user input command and input accordinly
 *  to the input command,the following input recived
 * from the user are:
 * 'n' - for number input
 * 's' - for the string input to recive.
 * 'e' - to sign out from the register server and exit.
 * 
 * if n recived,the client send to the application server to 
 * check if the data is prime.
 * if s recived, the client send to the application sercer to
 * check if the string is palindrom.
 * 
 * input:char and input according to inserted char
 * n - number
 * s - string
 * e - exit program
 * 
 * output:according to inserted char
 * n - is prime number or not
 * s - is the given string is palindrom or not.
 * *************************************************************/


//--------------- include section ------------------		
#include <stdio.h>       // for input, output
#include <stdlib.h>      // for exit.
#include <stdbool.h> 	//for bool.
#include <signal.h> 
#include <unistd.h>   //for getpid().
#include <sys/types.h>	  //
#include <sys/ipc.h>	
#include <sys/msg.h> 
#include <string.h>


//--------------- define section ------------------	
#define MAX_LEN 100
#define STRING_INPUT 's'
#define NUMBER_INPUT 'n'

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

void start_client(struct my_register_msgbuf client,
struct my_app_msgbuf str_messege);

void sign_in_client(struct my_register_msgbuf m_sign_msg);

void signout_client(struct my_register_msgbuf m_sign_msg);

void communicate_application(struct my_app_msgbuf str_messege);

void msgrcv_error();

void msgsnd_error();
//------------------ consts section --------------------

const int FULL_PIDS = 2,SIGNED = 1 ,NOT_SIGNED = -1,SIGN_OUT =3,
PALINDROM_OPERATION = 2,PRIME_OPERATION =1,MAX_STR_LEN = 100;
const char APP_SERVER_KEY= 'd',SIGN_SERVER = 'c',NOT_SIGNED_USER[]="-1" ;      // range of the numbers.


//------------------ global section --------------------
int msgid_sign_server,apllication_id;


//--------------- main section ------------------
int main(){
	key_t key_sign_server,key_apllication_server;
	struct my_register_msgbuf client;
	struct my_app_msgbuf messege;
	
	create_public_key(&key_sign_server,SIGN_SERVER);
	
	create_public_key(&key_apllication_server,APP_SERVER_KEY);
	
	create_private_key(&apllication_id,key_apllication_server);
	
	create_private_key(&msgid_sign_server,key_sign_server);
	
	start_client(client,messege);
	
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


/****************** start_client **********************
 * start the client,sign int o register server,
 * and wait for user input by following commands:
 * n - number input.
 * s - string input. 
 * for the number,is prime.
 * for the string, check if it is a palindrom.
 * ******************************************************/
void start_client(struct my_register_msgbuf sign_msg,
struct my_app_msgbuf str_messege){
	
	char client_command;
	
	sign_in_client(sign_msg);
	
	scanf("%c",&client_command);

	while( client_command != 'e' ){
		
		switch(client_command){
			
			case NUMBER_INPUT:
				scanf("%s",str_messege.data.mtext);
				str_messege.mtype = PRIME_OPERATION;
				break;
				
			case STRING_INPUT:
				scanf("%s",str_messege.data.mtext);
				str_messege.mtype = PALINDROM_OPERATION;
				break;
				
			default:
				str_messege.mtype = 0;
				break;
				
		}
		str_messege.data.m_pid = getpid();
		
		if(str_messege.mtype !=0){
			communicate_application(str_messege);
		}
		str_messege.mtype = 0;
		client_command = 0;
		
		scanf("%c",&client_command);
		
	}
	
	signout_client(sign_msg);
	
}


/****************** sign_in_client **********************
 * sign the user in the register server.
 * if recived there is no space left, exit.
 * ******************************************************/
void sign_in_client(struct my_register_msgbuf m_sign_msg){
	//****************************************************************
	
		m_sign_msg.data.m_pid = getpid();
		m_sign_msg.mtype = SIGNED;
		m_sign_msg.data.recived_msg = 0;
		
		//send to sign server.
		if(msgsnd(msgid_sign_server,&m_sign_msg,
		sizeof(struct register_data),0)==-1)
			msgsnd_error();
		
		
		if(msgrcv(msgid_sign_server,&m_sign_msg,
		sizeof(struct register_data),0,0)==-1)
			msgrcv_error();
		
		if(m_sign_msg.data.recived_msg == FULL_PIDS){
			return exit(EXIT_SUCCESS);
		}

}


/****************** signout_client **********************
 * signout the user from the register server.
 * ******************************************************/
void signout_client(struct my_register_msgbuf m_sign_msg){
	
		m_sign_msg.data.m_pid = getpid();
		m_sign_msg.mtype = SIGN_OUT;
		
		//send to sign server.
		if(msgsnd(msgid_sign_server,&m_sign_msg,
		sizeof(struct register_data),0)==-1)
			msgsnd_error();
		
		//recive msg from server.
		if(msgrcv(msgid_sign_server,&m_sign_msg,
		sizeof(struct register_data),0,0)==-1)
			msgrcv_error();
	
}


/****************** communicate_application **************
 * communicate with the application server and 
 * wait for response data and print it.
 * ******************************************************/
void communicate_application(struct my_app_msgbuf str_messege){
	
	str_messege.data.m_pid = getpid();
	
	if(msgsnd(apllication_id,&str_messege,
	sizeof(struct app_data),0)==-1)
		msgsnd_error();
	
	
	//recive msg from server.
	if(msgrcv(apllication_id,&str_messege,
	sizeof(struct app_data),getpid(),0)==-1){
		msgrcv_error();
	}
	
	sleep(1);
	
	//if recived -1 the user is not registered and exit.
	if(strcmp(str_messege.data.mtext,NOT_SIGNED_USER)==0){
		exit(EXIT_SUCCESS);
	}
	
	printf("%s\n",str_messege.data.mtext);
}


/****************** msgrcv_error **************
 * handle recive error.
 * *******************************************/
void msgrcv_error(){
	perror("msgrcv() failed");
	exit(EXIT_FAILURE);
}

/****************** msgrcv_error **************
 * handle send error.
 * *******************************************/
void msgsnd_error(){
	perror("msgsnd() failed");
	exit(EXIT_FAILURE);
}

