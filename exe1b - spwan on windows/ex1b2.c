/*****************************************************************

file:ex1b2.c
                    solve tower of hanoi
================================================================
Written by:
David Sapir, id = 208917351, login = davidsa
Shimshon Polak, id = 315605642, login = shimshonpo


This program gets from his father the amount of rings he should 
solve the problem tower of hanoi.

This program sovles the Tower of Hanoi problem for 15, 20,
25 and 30 rings and for display to the user how much time it took
for every amount of rings.


compile:cl ex1b2.c
Run:./ex1b2


Input: No input.

Output: No output. The process send the output to NUL.

*****************************************************************/

/********************* include section ****************************/
#include <stdio.h>
#include <process.h>
#include <stdlib.h>


/********************* prototype section *****************************/
void hanoi(int size, char src, char dest, char aux, FILE* fptr);


/************************ main section *******************************/
int main(int arc, char* argv[]) {
	FILE* fptr;
	int size = atoi(argv[1]);

	// we send the output to /dev/null to
	fptr = fopen("NUL", "w");
	hanoi(size, 'a', 'c', 'b', fptr);

	return EXIT_SUCCESS;
}


/************************** hanoi *********************************
 This function function prints the operations we need to do for
 the current size of rings the function got.
************************************T*****************************/
void hanoi(int size, char src, char dest, char aux, FILE* fptr) {
	if (size == 1)
		fprintf(fptr, "Move ring of size %d from %c to %c\n",
			size, src, dest);
	else {
		hanoi(size - 1, src, aux, dest, fptr);
		fprintf(fptr, "Move ring of size %d from %c to %c\n",
			size, src, dest);
		hanoi(size - 1, aux, dest, src, fptr);
	}
}