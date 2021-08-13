/***************************************************************
 
 file:ex4a1.c

                Bingo board holder
====================================================
Written by:
David Sapir, id = 208917351, login = davidsa
Shimshon Polak, id = 315605642, login = shimshonpo

The program Defines an array of integers of fifty thousand cells,
and inserts into the array numbers in the range of zero to two
hundred thousand (not including two hundred thousand).
The purpose of the program is to delete the numbers in the array,
using numbers it receives from other programs.
The program receives through the argument vector the name of
the pipe it will use, and these names of the children.

         	          
compile:gcc -Wall ex4a1.c -o ex4a1
Run:./ex4a1

note: if you want to run it u should compile & run ex4a2.c 3 
      time. each time the process will communicte with this
      program via another fifo.
      The names of the queues will be fifo0...fifo3.
      when fifo0 is the fifo with the process communicte
      write to this program.

Input: No input.
 
Output: the output will be(seperated by spaces):
        1. The time it took to delete all the numbers from the 
           array.
        2. the amout of numbers that the process got sent.
            (i.e. how many try of deletion from the array).
        3. The size of the array assigned to the program.
 
***************************************************************/

//---------------- include section ---------------------
#include <stdio.h>       // for input, output
#include <stdlib.h>      // for exit.
#include <sys/types.h>
#include <sys/wait.h>	 // for wait()
#include <unistd.h>       
#include <signal.h>       // for signals.
#include <errno.h>        // for error.
#include <time.h>         // for time.


//----------------- define section ---------------------
#define ARRAY_SIZE                  50000
#define NUM_OF_NAMED_PIPES     		4
#define NUMS_RANGE                  200000
#define NUM_OF_PROGRAMS             3
#define INPUT_SIZE                  5
#define FOUND                       1
#define NOT_FOUND                   0
#define SEED                        0


//----------------- prototype section ---------------------
void makeArrayOfRandomNums(int arr[]);
int number_is_found(int arr[],const int num);
void open_named_pipes(FILE *fd[NUM_OF_NAMED_PIPES],
	 char **argv, int pidArray[]);
void wait_for_all_programs(FILE *fdw, int pidArray[]);
void check_file_opening_status(const FILE * fptr);
void send_signal_to_begin_playing(FILE *fd[]);
void strat_playing(int numsArray[], FILE *fd[]);
void display_data(const int, const int);
void close_files(FILE *fd[]);


//----------------- main section -----------------------
int main (int argc, char *argv[]) {
	int  numsArray[ARRAY_SIZE];
	int pidArray[3] = { 0 };

	// fd[0] - fifo0
	// fd[1]...fd[3] - fifo1... fifo3
	FILE *fd[4]; 	  
		

	if (argc != INPUT_SIZE) {
		fprintf(stderr,"Bad input, please try again.\n");
		exit(EXIT_FAILURE);
	}
	
	makeArrayOfRandomNums(numsArray);

	open_named_pipes(fd, argv,pidArray);
	
    wait_for_all_programs(fd[0],pidArray);

	// all logged in ==> we can start playing.
	send_signal_to_begin_playing(fd);

  	strat_playing(numsArray, fd);

    close_files(fd);			// the process is done with
   								// the pipes ==> close them

	return EXIT_SUCCESS;
}
//endof main


//--------------- open_named_pipes ------------------
// This function opens the named pieps.
//---------------------------------------------------
void open_named_pipes(FILE *fd[NUM_OF_NAMED_PIPES],
					  char **argv, int pidArray[]) {
	int i;

	// the fifo to read from.
	fd[0] = fopen(argv[1], "r");
	check_file_opening_status(fd[0]);

	// the fifos to write to 
	for (i = 2; i <= NUM_OF_NAMED_PIPES; i++) {
	    fd[i - 1] = fopen(argv[i], "w");
		check_file_opening_status(fd[i - 1]);
	}
} 
//endof open_named_pipes


//---------------- wait_for_all_programs -----------------
// In this function we wait for all the 3 producers to 
// connent into the bingo game.
//--------------------------------------------------------
void wait_for_all_programs(FILE *fdr, int pidArray[]) {
	int i, pid;

	for (i = 0; i < NUM_OF_PROGRAMS; i++) {
		fscanf(fdr, "%d", &pid);
		pidArray[i] = pid;
	} 
}
//endof wait_for_all_programs


//------------- makeArrayOfRandomNums -----------------
// This function inserts random numbers into the array.
// The numbers are in the range 0 - 199999.
//-----------------------------------------------------
void makeArrayOfRandomNums(int arr[]) {
	srand(SEED);      
    int i;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
       arr[i] = rand() % NUMS_RANGE;
	}
}
//endof makeArrayOfRandomNums


//------------------- found_number ------------------
// This function runs over they array that hold 
// the random numbers and searches the current 
// number in the array. 
// The function return wether the number was found
// or not.
//---------------------------------------------------
int number_is_found(int arr[],const int num) {
	int i = 0;
	

	for (i = 0; i < ARRAY_SIZE; i++) {
		if (arr[i] == num) {
			arr[i] = -1;
			return FOUND;
		}
	}

	return NOT_FOUND;
}
//endof found_number


//---------- send_signal_to_begin_playing ----------
// This function sends a "signal" that givea sign
// to process that sends numbers that he can begin 
// sending numbers.
//--------------------------------------------------
void send_signal_to_begin_playing(FILE *fd[]) {
	int i, begin = 1;
	
	for (i = 1; i <= NUM_OF_PROGRAMS;i++) { 
		fprintf(fd[i], "%d\n" ,begin);
		fflush(fd[i]);
	}
}
//endof send_signal_to_begin_playing


//---------------- check_file_opening_status --------------
// This function checks if the opening of the file went 
// successfully.
//---------------------------------------------------------
void check_file_opening_status(const FILE * fptr) {
	if (fptr == NULL){
		fprintf(stderr,"Somthing wend wrong with opening the file.\n");
		exit(EXIT_FAILURE);
	}
}
//endof check_file_opening_status


//----------------------- strat_playing ------------------------
// Here, we begin playing the bingo game.
// In this function we get numbers (from the other process we 
// communicate with through the pipe), check if the number if
// found in the array that hold the random numbers. 
// If the number is found: we delete the number from the array
// that hold the number and send to the process throgh the pipe
// that the number has been found and he sended a good number.
// If the number is not found: we don't delete nothing from the 
// array and we send to the process that the number he sended 
// was not found in the array.
// we stop the bingo game when the amount of numbers left in 
// the array came to zero.
//---------------------------------------------------------------
void strat_playing(int numsArray[], FILE *fd[]){
	int num_amount = ARRAY_SIZE, num, fifo_index, status,
	 	logged_out = 0,numbers_got = 0,
		startTime, endTime;       

	startTime = time(NULL);
	while (logged_out != NUM_OF_PROGRAMS) { 
		fscanf(fd[0], "%d %d",&num, &fifo_index);
		numbers_got++;
		if (num_amount > 0) {
			status = number_is_found(numsArray, num);
		}
		else {
			status = -1;
			logged_out++;
		}
		fprintf (fd[fifo_index], "%d\n", status);
		fflush(fd[fifo_index]);
		if (status == FOUND) {
			num_amount--;
		}
	}
	endTime = time(NULL);


	// end of the game ==> display the needed info
	display_data((endTime - startTime), numbers_got);
}
//endof strat_playing


//---------------------- display_data ------------------------
// In this ufnction the process display to the user the 
// following data(seperated by spaces): 
// 1. how much time it took.
// 2. the amount of number he got from the other 3 processes.
// 3. the size of the array he worked with.
//------------------------------------------------------------
void display_data(const int timeTook, const int numbers_got){

	// give the sending process to print first
	sleep(0.1);  
	
	// display data
	printf("%d %d %d\n", timeTook,numbers_got, ARRAY_SIZE);
}
//endof display_data


//---------------- close_files ------------------
// The process closses all the files he used to
// communicate via the pipes
//-----------------------------------------------
void close_files(FILE *fd[]){
	int i;

	// run over the files and close them
    for (i = 0; i < NUM_OF_NAMED_PIPES; i++) {
		fclose(fd[i]);
	}
}
//endof close_files