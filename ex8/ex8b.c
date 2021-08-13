/***************************************************************

  file: ex8b.c

            GCD, DIPD and Front End threads
  ====================================================

  This program has a main thread that gives birth to
  three children(threads), keeps their IDs in a global array,
  And finishes.

  The three threads are:
  1. GCD server - give service to clients that want to
     calculate the Greatest common divisor of 2 numbers.

     The server works as follows:
     The server goes to sleep until receiving the SIGUSR1
     signal.
     When waking up due to receiving the above signal,
     pulls out the two data waiting for it in a dedicated
     global array.
     Calculate the gcd of the 2 given number and return the
     answer to the client (also with the signal SIGUSR1).

  2. DIPF (Decomposition into primary factors) - give service
     to clients that want to find the decomposition into
     primary factors of a number.

  3. Front End - give the client to choose what kind of
     serive he wish to get and read accordingly the numbers.

     The Frond End works as follows:
     In an endless loop:
     A)  If the user enters the chacter g then: Reads
         the two numbers whose GCD should be calculated.
     B)  If the user enters the chacter d then:
         Reads a natural number to be broken down into
         primary factors.
     C) Stores the input in the required array.
     D) Signals to the right server.
     E) Waits for the server to end and return then answer.
     F) Displays the answer to the standard outout.


  compile: gcc -Wall ex8b.c -lpthread -lm -o ex8b
  run: ./ex8b


  input:
  Depending on the type of service:
  1) GCD server service - g and 2 numbers greater than zero.
  2) DIPF server service - d and a numbers greater than zero.

  output:
  Depending on the type of service:
  1) GCD server service - the gcd number.
  2) DIPF server service - the number Disassembled into
     primary factors.

***************************************************************/


//----------------- include section ---------------------
#include <pthread.h>           // to use threads
#include <stdio.h>
#include <stdlib.h>            // for exit()
#include <string.h>            // for memset()
#include <signal.h>            // for signal()
#include <unistd.h>            // for pause()
#include <errno.h>             // for perror()
#include <time.h>              // for time()
#include <math.h>              // for sqrt()


//---------------- consts section --------------------

#define GCD_SERVER_ARR_SIZE  3
#define PF_SERVER_ARR_SIZE   11
#define NUM_OF_THREAD        4       // main thread that gives birth
                                     // to more three threads ==> 4

const int
RUN_SERVER = 1,
RUN_FRONT_END = 1,
MAIN_THREAD_LOC = 0,
GCD_SERVER_LOC = 1,
PF_SERVER_LOC = 2,
FRONT_END_LOC = 3,

GCD_SERVER_ANSWER_LOC = 2,
GCD_SERVER_NUM1_LOC = 0,
GCD_SERVER_NUM2_LOC = 1,

PF_SERVER_NUM_LOC = 0,
PF_SERVER_ANSWER_LOC_BEG = 1;


// to know the kind of service to give
const char
NO_TASK_REQUESTED = 'n',
GCD_SERVER_SERVICE_CHAR = 'g',
DIPF_SERVER_SERVICE_CHAR = 'd';


//--------------------- prototype section ----------------------

// main thread runs and create
// all the needed threads
void* create_threads(void * arg);

// Get an error message, print it to stderr
// and exit with failure.
// for non S.C functions that doesn't set errno.
void handle_error(const char* msg);

// gcd thread
// (more info in definition)
void* run_gcd_server(void* arg);

// give the user the service of the GCD server
void gcd_server_service();

// Return the gcd of 2 given non-zero numbers
int find_gcd(const int n1, const int n2);

// prime factors thread
// (more info in function definition)
void* run_prime_factor_server(void* arg);

// clean the array for the next client
void clean_pf_arr();

// Calc the prime factos of a number
void primeFactors(int n);

// frond_end thread
// (more info in function definition)
void* run_front_end(void* arg);

// give the user the service of the DIPF server
void dipf_server_service();

// Prints the answer of recived from the DIPF server
void print_dipf_server_answer();

// Define signal handlers
void define_signals();

// catch SIGUSR1 signal
void catch_sigusr1(int sig_num);

// catch SIGINT signal
void catch_sigint(int sig_num);

// catch SIGUSR2 signal
void catch_sigusr2(int sig_num);

// frees the mutex and the cv 
void free_mutex_and_cv();


//------------------ global variables -------------------

// to hold the 4 threads ids
pthread_t threads_id[4];

// global arrays to give service to the client.
int gcd_arr[GCD_SERVER_ARR_SIZE],
    pf_arr[PF_SERVER_ARR_SIZE];

int run_server = 1;

// create condition variable for GCD server
pthread_cond_t cv1 = PTHREAD_COND_INITIALIZER;

// create condition variable for DIPF server
pthread_cond_t cv2 = PTHREAD_COND_INITIALIZER;

// create a mutex for the GCD server
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

// create a mutex for the DIPF server
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

//--------------------- main section --------------------
int main()
{
    int i;       // for for()

    // define signals for this program
    define_signals();


    // main thread creates all the other threads:
    // gcd, prime factors and front end.
    pthread_create(&(threads_id[MAIN_THREAD_LOC]),
                   NULL,
                   create_threads,
                   NULL);


    // wait for all the threads to finish
    for (i = 0; i < NUM_OF_THREAD; i++)
    {
        pthread_join(threads_id[i], NULL);
    }

    // before exiting, free all the cvs and mutexses
    free_mutex_and_cv();

    puts("");
    return EXIT_SUCCESS;
}


//----------------- create_servers_and_front_end ---------------
// The main thread creates 3 threads:
// 1. gcd_server thread.
// 2. primes factor thread
// 3. front end thread.
//
// Every time, After creating a thread we check if the return
// value of  pthread_create function is not equal to zero.
// If the return value is equal to zero we call the function
// handle_error with the wanted string and print to stderr to
// notify the user.
//
// When the main thread creates all the needed threads he
// finishes with pthread_exit().
//--------------------------------------------------------------
void* create_threads(void * arg)
{
    int status;          // to hold the return status of
    // pthread_create

    // create the gcd server thread
    status  = pthread_create(&(threads_id[GCD_SERVER_LOC]),
                             NULL,
                             run_gcd_server,
                             NULL);
    if (status) handle_error("pthread_create failed");


    // create the prime factors thread
    status  = pthread_create(&(threads_id[PF_SERVER_LOC]),
                             NULL,
                             run_prime_factor_server,
                             NULL);
    if (status) handle_error("pthread_create failed");


    // create the front end thread
    status  = pthread_create(&(threads_id[FRONT_END_LOC]),
                             NULL,
                             run_front_end,
                             NULL);
    if (status) handle_error("pthread_create failed");


    // the main thread finishes.
    pthread_exit(NULL);
}


//---------------- start_gcd_server ------------------
// The gcd thread.
// Here we control the server and give service to the
// clients that want to find the greatest common
// divisor of 2 numbers.
//
// Signals the client with SIGUSR1 that he can read the
// answer.
//----------------------------------------------------
void* run_gcd_server(void * arg)
{
    int gcd , rc;

    // begin running the server
    while(RUN_SERVER)
    {
        // lock the mutex
        rc = pthread_mutex_lock(&mutex1);
        if (rc) handle_error("pthread_mutex_lock failed");

        // wait with condition 
        rc = pthread_cond_wait(&cv1, &mutex1);
        if (rc) handle_error("pthread_cond_wait failed");

        // calc the gcd for the client
        gcd = find_gcd(gcd_arr[GCD_SERVER_NUM1_LOC],
                       gcd_arr[GCD_SERVER_NUM2_LOC]);


        // insert the answer into thr array
        gcd_arr[GCD_SERVER_ANSWER_LOC] = gcd;


        // done with the critical section ==> unlock the mutex
        // the client can read the answer. 
        rc = pthread_mutex_unlock(&mutex1);
        if (rc) handle_error("pthread_mutex_unlock failed");


        // send signal to client ==> he can read the answer
        pthread_kill(threads_id[FRONT_END_LOC], SIGUSR1);
    }

    pthread_exit(NULL);
}


//----------------- find_gcd ---------------------
// Finds the greatest common divisor of two given
// numbers and return the answer.
//------------------------------------------------
int find_gcd(const int n1, const int n2)
{
    int gcd = 1, i;

    for(i=1; i <= n1 && i <= n2; ++i)
    {
        // Checks if i is factor of both integers
        if(n1%i==0 && n2%i==0)
            gcd = i;
    }


    // return the greatest common devisor
    return gcd;
}


//----------------- run_prime_factor_server ----------------
// the prime factors thread.
// Here we give service to clients that want to calculate
// the prime fuactor of a number.
//
// Signals the client with SIGUSR1 that he can read the
// answer.
//----------------------------------------------------------
void* run_prime_factor_server(void* arg)
{
    int rc;

    // start running the server
    while(RUN_SERVER)
    {
        // lock tue mutex
        rc = pthread_mutex_lock(&mutex2);
        if (rc) handle_error("pthread_mutex_lock failed");

        // wait with conditaion
        rc = pthread_cond_wait(&cv2, &mutex2);
        if (rc) handle_error("pthread_cond_wait failed");

        // clean the answer array for next client.
        clean_pf_arr();

        // give service to user and decompose the number
        primeFactors(pf_arr[PF_SERVER_NUM_LOC]);

        // unlock the mutex 
        rc = pthread_mutex_unlock(&mutex2);
        if (rc) handle_error("pthread_mutex_unlock failed");

        // send signal to client ==> he can read the answer
        pthread_kill(threads_id[FRONT_END_LOC], SIGUSR1);
    }

    pthread_exit(NULL);
}


//--------------- clean_pf_arr ----------------
// cleans the array for the next client.
//---------------------------------------------
void clean_pf_arr()
{
    int i;

    // run over the global array and clean 
    // it for the next client.
    for (i = 1; i < PF_SERVER_ARR_SIZE; i++)
    {
        pf_arr[i] = 0;
    }
}


//-------------- primeFactors ---------------
// The function get a point to the shm and
// the number to decompose.
// the funciton finds the primary factors
// and sets them into the right place in
// the array of the server.
//-------------------------------------------
void primeFactors(int n)
{
    int index = PF_SERVER_ANSWER_LOC_BEG, i;

    // Print the number of 2s that divide n
    while (n % 2 == 0)
    {
        pf_arr[index] = 2;
        index++;
        n = n/2;
    }

    // n must be odd at this point.  So we can skip
    // one element (Note i = i +2)
    for (i = 3; i <= sqrt(n); i = i + 2)
    {
        // While i divides n, print i and divide n
        while (n % i == 0)
        {
            pf_arr[index] = i;
            index++;
            n = n/i;
        }
    }

    // This condition is to handle the case when n
    // is a prime number greater than 2
    if (n > 2)
        pf_arr[index] = n;
}


//----------------- begin_running_front_end -----------------
// This function reads from the user the action he wish
// to perform and the needed data for that task,
// inserts the data into the right array,
// waits for signal, i.e. does pause().
//-----------------------------------------------------------
void* run_front_end(void* arg)
{
    char wanted_task;

    // start running the server
    while(RUN_FRONT_END)
    {
        // get a request from client
        wanted_task = NO_TASK_REQUESTED;
        scanf("%c", &wanted_task);

        // check the type of the request
        if (wanted_task == GCD_SERVER_SERVICE_CHAR)
        {
            gcd_server_service();  
        }
        else if (wanted_task == DIPF_SERVER_SERVICE_CHAR)
        {
            dipf_server_service();
        }
    }

    pthread_exit(NULL);
}


//--------------- gcd_server_service ---------------
// Reads from the user the need input for the
// server service and sends the request to the
// gcd server.
// Then, waits for the server to finish and
// display the gcd to the user.
//--------------------------------------------------
void gcd_server_service()
{
    int rc;

    // lock mutex before entering the critical section
    rc = pthread_mutex_lock(&mutex1);
    if (rc) handle_error("pthread_mutex_lock failed\n");

    // read 2 numbers from the user.
    scanf("%d", &(gcd_arr[GCD_SERVER_NUM1_LOC]));
    scanf("%d", &(gcd_arr[GCD_SERVER_NUM2_LOC]));


    // unlock the mutex before sending a signal
    rc = pthread_mutex_unlock(&mutex1);
    if (rc) handle_error("pthread_mutex_unlock failed\n");


    // signal to the gcd server he can use the critical section
    rc = pthread_cond_signal(&cv1);
    if (rc) handle_error("pthread_cond_signal failed\n");


    // wait for server to end
    pause();

    // display the answer to the client
    printf("%d\n", gcd_arr[GCD_SERVER_ANSWER_LOC]);
}


//------------- dipf_server_service --------------
// Reads from the user the need input for the
// server service and inserts in the right place.
// Then, waits for the server to finish and
// display the decomposition into primary
// factors of the number the user delivered.
//-----------------------------------------------
void dipf_server_service()
{
    int rc;

    // lock mutex before entering the critical section
    rc = pthread_mutex_lock(&mutex2);
    if (rc) handle_error("pthread_mutex_lock failed\n");

    // read number from user
    scanf("%d", &pf_arr[PF_SERVER_NUM_LOC]);

    // unlock mutex ==> the gcd can use the critical section
    rc = pthread_mutex_unlock(&mutex2);
    if (rc) handle_error("pthread_mutex_unlock failed\n");
    

    // signal to the gcd server he can use the critical section
    rc = pthread_cond_signal(&cv2);
    if (rc) handle_error("pthread_cond_signal failed\n");

    // wait for server to end
    pause();

    // dipslay to the user the
    // decomposition into primary factors
    print_dipf_server_answer();
}


//-------- print_dipf_server_answer -----------
// Displays to the user the answer The answer
// received for the DIPF server.
//---------------------------------------------
void print_dipf_server_answer()
{
    int i;

    for (i = PF_SERVER_ANSWER_LOC_BEG;
            i < PF_SERVER_ARR_SIZE && pf_arr[i] != 0;
            i++)
    {
        printf("%d ", pf_arr[i]);
    }

    // empty output buffer
    puts(" ");
}


//-------------- handle_error --------------
// Receives a string, dispaly to stderr and
// then exits with failure.
//
// note: The function should be used for
//       situations that errno is not set.
//------------------------------------------
void handle_error(const char* msg)
{
    fputs(msg, stderr);
    exit(EXIT_FAILURE);
}


//------------ define_signals ------------
// Define signal handlers
//----------------------------------------
void define_signals()
{
    // define signal handler for SIGINT
    signal(SIGINT, catch_sigint);

    // define signal handler for SIGUSR1
    signal(SIGUSR1, catch_sigusr1);

    // define signal handler for SIGUSR1
    signal(SIGUSR2, catch_sigusr2);
}


//----------- catch_sigusr1 --------------
// signal handler for the signal SIGUSR1.
// Needed to exit the pause.
//----------------------------------------
void catch_sigusr1(int sig_num)
{
    signal(SIGUSR1, catch_sigusr1);     // reload signal
}


//----------------- catch_sigusr2 ------------------
// The program got ^C ==> all threads should exit
//--------------------------------------------------
void catch_sigusr2(int sig_num)
{
    pthread_exit(NULL);
}


//----------------------- free_mutex_and_cv --------------------
// frees the mutexes and the condition variables we used for
// this program.
// Before destroing a mutex we should assure that the condition
// variable is not in pthread_cond_wait so we open the mutex 
// and send signal to cv.
// Also, we can free the mutex there are no cvs 
// in pthread_cond_wait. 
// If there is a cv in pthread_cond_wait both 
// pthread_cond_destroy and pthread_mutex_destroy will fail.
//--------------------------------------------------------------
void free_mutex_and_cv()
{
    int rc;         // to hold return values
    

    // first, unlock the mutex
    pthread_mutex_unlock(&mutex1);

    // signal cv1 to exit the condition wait
    pthread_cond_signal(&cv1);

    // destroy the condition variable
    rc = pthread_cond_destroy(&cv1);
    if (rc) handle_error("pthread_cond_destroy failed");

    // first, unlock the mutex
    pthread_mutex_unlock(&mutex2);

    // signal cv2 to exit the condition wait
    pthread_cond_signal(&cv2);

    // destroy the condition variable
    rc = pthread_cond_destroy(&cv2);
    if (rc) handle_error("pthread_cond_destroy failed");

    // destroy mutex1
    rc = pthread_mutex_destroy(&mutex1);
    if (rc) handle_error("pthread_mutex_destroy failed");

    // destory mutex2
    rc = pthread_mutex_destroy(&mutex2);
    if (rc) handle_error("pthread_mutex_destroy failed");
}


//------------- catch_sigint ---------------
// Got ^C ==> need to exit the program.
//------------------------------------------
void catch_sigint(int sig_num)
{
    int i;

    // run over the threads and send them signal to exit
    for (i = 1; i < NUM_OF_THREAD; i++)
    {
        pthread_kill(threads_id[i], SIGUSR2);
    }
}
