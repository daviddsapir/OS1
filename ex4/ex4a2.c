/***************************************************************
 
file:ex4a2.c

                Number manufacturers
====================================================
Written by:
David Sapir, id = 208917351, login = davidsa
Shimshon Polak, id = 315605642, login = shimshonpo

This program receives through the argument vector the name of the
queue and fifo0, and the number of the current process (1,2,3)
which will also determine the name of the queue file through which 
the current process will receive feedback on whether the number it
sent to plan A was received / rejected The one that runs will be 
given a different name)

         	          
compile:gcc -Wall ex4a2.c -o ex4a2
Run:./ex4a2

note: you need to send make 3 process of ex4a2, to each one send 
      his own number.

Input: No input.
 
Output: the output will be:
        1. the amout of numbers that the process sent.
         (i.e. how many try of deletion from the array).
        2. The amount of successfull deletions.

        note: the output will be seperated by spaces.


****************************************************************/

//---------------- include section ---------------------
#include <stdio.h>       // for input, output
#include <stdlib.h>      // for exit.
#include <sys/types.h>
#include <sys/wait.h>	 // for wait()
#include <unistd.h>       
#include <signal.h>       // for signals.
#include <errno.h>        // for error.
#include <time.h>         // for time.
#include <string.h>       // for strcat()


//----------------- define section ---------------------
#define NUMS_RANGE                  200000
#define INPUT_SIZE                  3
#define STR_LEN                     10
#define NUM_OF_FILES                2
#define STOP_SENDING                -1
#define GOOD_GUESS                  1


//-------------------- prototype section -----------------------
void open_named_pipes(FILE *fd[], char fifo_name[],char **argv);
void login_into_game(FILE* fdw, FILE* fdr);
void start_sending(FILE* fdw , FILE* fdr, const int);
void display_data(const int, const int);
void close_files(FILE *fd[]);


//----------------------- main section -------------------------
int main(int argc, char *argv[]) {
    int seed = atoi(argv[2]);
    srand(seed);

    // fd[0] - write
    // fd[1] - read
    FILE *fd[NUM_OF_FILES];
    char fifo_name[STR_LEN] = "fifo";

    if (argc != INPUT_SIZE) {
        fprintf(stderr,"Bad input, please try again.\n");
		exit(EXIT_FAILURE);
    }

    open_named_pipes(fd, fifo_name, argv);

    login_into_game(fd[0], fd[1]);

    start_sending(fd[0], fd[1],atoi(argv[2]));

    close_files(fd);                // the process is done with
                                    // the pipes ==> close them

    return EXIT_SUCCESS;
}
//endof main


//------------- open_named_pipes -------------
// This function opens the fifos.
// opens fd[0] for writing.
// opens fd[1] for reading.
//--------------------------------------------
void open_named_pipes(FILE *fd[],
    char fifo_name[],char **argv) {

    strcat(fifo_name, argv[2]);
    fd[0] = fopen(argv[1], "w");
    fd[1] = fopen(fifo_name, "r");
}
//endof open_named_pipes


//--------------------- login_into_game ------------------------
// In this function the process logins into the game by sending 
// a "signal" (his pid) to the game server that he is ready 
// to begin playing.
// After sending his signal, he is waiting for an answer from 
// the pipe he read from. the answer indicates that he can 
// begin "playing" (sending numbers).
//--------------------------------------------------------------
void login_into_game(FILE *fdw, FILE *fdr) {
    int start_sending = 0;


    // send ur pid to login into game.
    fprintf(fdw,"%d\n", getpid());
    fflush(fdw);                      // empty the fdw buffer.


    // wait for a signal to begin sending numbers. ==>
    // the process is blocked till it reads from the pipe
    fscanf(fdr, "%d", &start_sending);
}
//endof login_into_game


//----------------------- start_sending ------------------------------
// This is the main function of that program. 
// Here, the process sends the numbers (via the pipe "fifo") to the 
// program that hold the numbers that should get deleted.
// The function cound how many number he has sends and how many 
// of them got to the deletion of the number in the process that 
// hold the numbers to be deleted.
//--------------------------------------------------------------------
void start_sending(FILE* fdw, FILE* fdr, const int pipe_id){    
    int num,                  // the current random number
        numbersSend = 0,      // number send counter
        numberGotToDel = 0,   // numbers got to del counter
        status;               // the status of the current number &&
                              // to know when to stop sending numbers.

    while (status != STOP_SENDING){
        numbersSend++;
        num = rand() % NUMS_RANGE;
        fprintf(fdw, "%d %d\n", num, pipe_id);
        fflush(fdw);
        fscanf(fdr, "%d", &status);
        if (status == GOOD_GUESS)
            numberGotToDel++;
    }

    // the process is done sending ==> display info
    display_data(numbersSend,numberGotToDel);
}
//endof start_sending


//---------------- display_data ---------------------
// The process displays how many numbers he has 
// send, and how many of them got to Deletion.
//---------------------------------------------------
void display_data(const int numbersSend,
    const int numberGotToDel)
{
    printf("%d %d\n", numbersSend, numberGotToDel);
}
//endof display_data


//------------ close_files ----------------
// The process closses all the files he 
// used to communicate via the pipes
//-----------------------------------------
void close_files(FILE *fd[]){
    fclose(fd[0]);       // close fifo0
    fclose(fd[1]);       // close fifo1
}
//endof close_files