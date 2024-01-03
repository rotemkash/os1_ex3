/*
======================================================================
File: ex3b.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
Mahmood Jazmawy, ID = 211345277

In this program the parent initializes an array and then creates 3 children.
the parent runs 100 time in each iteration he calls the children. 
each child draws random numbers until getting a prime one, then it sends
it to the parent. the parent checks if the number is larger than all the 
numbers in the array it adds it to the array, if not it doesn't.
we keep doing this and creating an ascending prime numbers array.
then the parent prints the array size and the min/max values in it
then each child prints about how he performed, how many numbers he entered.
*/
//-------include section---------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdbool.h>
#include <math.h>

//-------const section---------------
const int ARR_SIZE = 100, NUM_OF_CHILDREN = 3, SEED = 17;

//------prototypes---------------------
bool is_prime(int num);

void do_child(int *children_pipe, int *main_pipe);

void do_father(int children_pipe[NUM_OF_CHILDREN][2], 
				int *main_pipe, pid_t pid_arr[]);

void signal_handler(int signal);

//--------global section---------------
int found_primes = 0;

//---------main section---------------
int main() {
    int child_pipe[NUM_OF_CHILDREN][2], main_pipe[2];
    pid_t pid_children[NUM_OF_CHILDREN];

    if (pipe(main_pipe) == -1) {
        perror("main pipe failed\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < NUM_OF_CHILDREN; i++) {
        if (pipe(child_pipe[i]) == -1) {
            perror("child pipe failed\n");
            exit(EXIT_FAILURE);
        }
    }

    srand(SEED);
    for (int j = 0; j < NUM_OF_CHILDREN; j++) {
        pid_children[j] = fork();
        if (pid_children[j] < 0) { // check fork
            perror("fork failed\n");
            exit(EXIT_FAILURE);
        }
        if (pid_children[j] == 0) {
            do_child(child_pipe[j], main_pipe);
        }

    }
    for (int k = 0; k < NUM_OF_CHILDREN; k++) {
        close(child_pipe[k][0]); // close for reading
    }
    close(main_pipe[1]); // close for writing
    do_father(child_pipe, main_pipe, pid_children);
    return EXIT_SUCCESS;
}

/**
 * This function is responsible for running the children, each one draws a 
 * random prime number then sends it to the father
 */
void do_child(int *children_pipe, int *main_pipe) {
	
    signal(SIGINT, signal_handler);
    
    int num, num_from_father;
    int prime_and_pid[2];

    close(children_pipe[1]); // close for writing
    close(main_pipe[0]); // close for reading

    while (1) {
        num = rand() % ((int)(pow(10,6)) - 1) + 2; // draw random number
        if (is_prime(num)) { // check if prime
            prime_and_pid[0] = num;
            prime_and_pid[1] = getpid();
            // write to father
            if (write(main_pipe[1], &prime_and_pid, sizeof(prime_and_pid)) < 0){
                perror("write from child to father failed\n ");
                exit(EXIT_FAILURE);
            }
            // read from father
            if (read(children_pipe[0], &num_from_father, sizeof(int)) < 0) {
                perror("read from child to father failed\n ");
                exit(EXIT_FAILURE);
            }
            if (num_from_father == 1) {
                found_primes++;
            }
        }
    }
}

/**
 * This function is responsible for running the father. it creates 3 children
 * and receives through a pipe the numbers from them. 
 * it checks if the number meets the criteria it adds it to the array.
 *  if not it doesn't and returns zero to the child
 */
void do_father(int children_pipe[NUM_OF_CHILDREN][2], 
				int *main_pipe, pid_t pid_arr[]) 
{
    int prime_arr[ARR_SIZE], child_answer[2];
    int found = 0, curr_cell = 0, prime_to_add;

    // init the primes arr to zeros
    for (int i = 0; i < ARR_SIZE; ++i) {
        prime_arr[i] = 0;
    }

    for (int child = 0; child < NUM_OF_CHILDREN; child++) {
        close(children_pipe[child][0]); // close for reading
    }
    close(main_pipe[1]); // close for writing

    for (int i = 0; i < ARR_SIZE; ++i) { //run until 100
        if (read(main_pipe[0], &child_answer, sizeof(int) * 2) < 0) {
            perror("cannot read\n");
            exit(EXIT_FAILURE);
        }

        prime_to_add = child_answer[0];
        if (curr_cell == 0) { // if it's the first time adding a number
            prime_arr[curr_cell++] = prime_to_add;
            found = 0;
        } else { // if there's already numbers in the array
            if (prime_arr[curr_cell - 1] <= prime_to_add) {
                prime_arr[curr_cell++] = prime_to_add;
                found = 1;
            } else {
                found = 0;
            }
        }

        if (child_answer[1] == pid_arr[0]) { // send reply to children
            if (write(children_pipe[0][1], &found, sizeof(int)) < 0) {
                perror("cannot write1\n");
                exit(EXIT_FAILURE);
            } 
        } else if (child_answer[1] == pid_arr[1]) {
            if (write(children_pipe[1][1], &found, sizeof(int)) < 0) {
                perror("cannot write2\n");
                exit(EXIT_FAILURE);
            }

        } else if (child_answer[1] == pid_arr[2]) {
            if (write(children_pipe[2][1], &found, sizeof(int)) < 0) {
                perror("cannot write3\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    printf("array size: %d. Min Value: %d. Max Value: %d.\n", curr_cell, 
			prime_arr[0], prime_arr[curr_cell - 1]);
    // kill all children
    for (int j = 0; j < NUM_OF_CHILDREN; ++j) {
        kill(pid_arr[j], SIGINT);
    }

    for (int child = 0; child < NUM_OF_CHILDREN; child++) {
        close(children_pipe[child][1]); // close for reading
    }
    close(main_pipe[0]); // close for writing
	
    
}

/**
 * signal handler for SIGINT
 */
void signal_handler(int signal) {
    printf("child %d has inserted: %d prime nums to array\n", 
    getpid(), found_primes);
    exit(EXIT_SUCCESS);
}


/**
 * This function checks if a number is prime or not
 */
bool is_prime(int num) {
    int i, mid;
    mid = num / 2;
    if (num < 2) {
        return false;
    }
    for (i = 2; i <= mid; i++) {
        if (num % i == 0) {
            return false;
        }
    }
    return true;
}

