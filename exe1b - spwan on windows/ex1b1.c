/********************************************************************

  file:ex1a.c


					Tower of Hanoi
====================================================
Written by:
David Sapir, id = 208917351, login = davidsa
Shimshon Polak, id = 315605642, login = shimshonpo

This program sovles the Tower of Hanoi problem for 15, 20,
25 and 30 rings and for display to the user how much time it took
for every amount of rings.

compile:cl ex1a.c
Run:./ex1a


Input: No input.

Output: The output of this program will be the time it took for the
		processor to solve the Tower of Hanoi problem for 15, 20,
		25 and 30 rings.
		The program displays display for every rings amount how much
		time it took to solve it.

 *********************************************************************/


/********************** include section ******************************/
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


/************************ main section *********************************/
int main() {
	int pid,size = 5,i, second_before=0,
		second_after=0;
	char* str[10] = { "ex1b2.exe",""};

	for (i = 0; i < 5; i++) {
		size += 5;
		sprintf(str[1], "%d", size);

		second_before = time(NULL);
		
		if (_spawnl(_P_WAIT, "ex1b2.exe", str[0],str[1], NULL) < 0) {
			fputs("error in fork\n", stderr);
			exit(EXIT_FAILURE);
		}

		second_after = time(NULL);
		printf("%d = %d \n", size, second_after - second_before);
	}

	return EXIT_SUCCESS;
}