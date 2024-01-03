/*
 * ======================================================================
File: ex3a.c
======================================================================
Writen by : Rotem Kashani, ID = 209073352
    Mahmood Jazmawy, ID = 211345277

* The father defines an array of one hundred thousand cells and fills
* it in an acsending order.
* He does this by inserting into cell #0 a random value in the range
 * between 0 and 10 (including 0, not including 10),
* and for each additional cell the value that was in the previous cell and
* another random value in the range 0..9 will be inserted. The father then gives
* birth to two children. Each child grills a million times a number between zero
* and a million (including 0 and not including a million), and searches for it in
* the array. Child A searches for the value serial search, child B searches for
* binary search. Each child measures how long it took him to complete the task
 * and how much times it found a number. then it puts in the standard output
 * s/b [number of found] [time it took]
 * the father then reads the output using a pipe.
 * The father calculates and displays two numbers (on one line, with a space
* between them): how long on average did the ten children who performed a serial
* search run, how long on average did the ten children who performed a binary
* search ran. Finally, the father presents in a separate line another figure
* (third): how long it took him to run from the moment he started,
* until he finished.
 */
//=================== INCLUDES ===========================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

//====================== CONSTS ===========================
#define ARR_LEN 100000
#define MILL 1000000
#define NUM_OF_CHIL 10

//========================== PROTOTYPES ============================
void fill_array(int arr[]);

int series_search(const int arr[ARR_LEN], int num_to_search);

void doFirstChild(int arr[ARR_LEN], int pipe_descs[]);

void doSecondChild(int arr[ARR_LEN], int pipe_descs[]);

int binary_search(const int arr[ARR_LEN], int num_to_search);


//==================== MAIN =============================

int main(int argc, char *argv[]) {
    struct timeval t0, t1;
    int pipe_descs[2]; // pipe descerptors

    gettimeofday(&t0, NULL);

    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments!!");
        return EXIT_FAILURE;
    }

    if (pipe(pipe_descs) == -1) {
        perror("cannot open pipe");
        exit(EXIT_FAILURE);
    }

    srand(strtol(argv[1], NULL, 10));
    int veryBigArray[ARR_LEN];

    pid_t child_1, child_2;
    int childTime = 0, // variable to hold the time the child returns
    tmp, // temp number to swallow the middle value in the output
    fatherTime, // the father's time
    sChildAvg = 0, bChildAvg = 0, // avareges of the children
    sIndex = 0, sChildArr[NUM_OF_CHIL], // arr to hold s children times
    bIndex = 0, bChildArr[NUM_OF_CHIL]; // arr to hold b children times
    char searchType;


    for (int i = 0; i < NUM_OF_CHIL; ++i) {
        // re-fill the array
        fill_array(veryBigArray);

        child_1 = fork();
        if (child_1 < 0) {
            perror("error in fork");
            exit(EXIT_FAILURE);
        }
        if (child_1 == 0) {
            doFirstChild(veryBigArray, pipe_descs);
            exit(EXIT_SUCCESS); // sends the time to the father
        }

        child_2 = fork();
        if (child_2 < 0) {
            perror("error in fork");
            exit(EXIT_FAILURE);
        }
        if (child_2 == 0) {
            doSecondChild(veryBigArray, pipe_descs);
            exit(EXIT_SUCCESS); // sends the time to the father
        }

        close(pipe_descs[1]);
        dup2(pipe_descs[0], STDIN_FILENO);

        scanf("%c %d %d", &searchType, &tmp, &childTime);
        if (searchType == 's') {
            sChildArr[sIndex++] = childTime;
        } else { // if 'b'
            bChildArr[bIndex++] = childTime;
        }
        scanf("%c %d %d", &searchType, &tmp, &childTime);
        if (searchType == 's') {
            sChildArr[sIndex++] = childTime;
        } else { // if 'b'
            bChildArr[bIndex++] = childTime;
        }
    }


    // calculate avg for s children
    for (int i = 0; i < NUM_OF_CHIL; ++i) {
        sChildAvg += sChildArr[i];
    }
    sChildAvg = sChildAvg / NUM_OF_CHIL;

    // calculate avg for b children
    for (int i = 0; i < NUM_OF_CHIL; ++i) {
        bChildAvg += bChildArr[i];
    }
    bChildAvg = bChildAvg / NUM_OF_CHIL;

    printf("%d %d\n", sChildAvg, bChildAvg);


    gettimeofday(&t1, NULL);
    fatherTime = (int) ((double) (t1.tv_usec - t0.tv_usec) / 1000000 +
                        (double) (t1.tv_sec - t0.tv_sec));
    // print the father's time in the file
    printf("%d\n", fatherTime);

    return EXIT_SUCCESS;
}


/**
 * This function runs the first child's functionality.
 * it receives the array and the file. it generate 10 Million random numbers
 * then it searches each number in the big array(using primitive search method).
 * if it finds the current number it adds one to the variable that
 * hold the number of found numbers. then it returns that number.
 */
void doFirstChild(int arr[ARR_LEN], int pipe_descs[]) {
    int rand_num, found_times = 0;
    struct timeval t0, t1;
    int task_time;
    gettimeofday(&t0, NULL);

    // create the numbers and search for them
    for (int i = 0; i < MILL; ++i) {
        rand_num = rand() % MILL;
        if (series_search(arr, rand_num) == 1) {
            found_times++;
        }
    }
    gettimeofday(&t1, NULL);
    task_time = (int) ((double) (t1.tv_usec - t0.tv_usec) / 1000000 +
                       (double) (t1.tv_sec - t0.tv_sec));

    close(pipe_descs[0]);
    close(STDOUT_FILENO);
    dup(pipe_descs[1]);
    printf("s %d %d\n", found_times, task_time);
    close(pipe_descs[1]);

    exit(EXIT_SUCCESS);
}


/**
 * This function runs the second child's functionality.
 * it receives the array and the file. it generate 10 Million random numbers
 * then it searches each number in the big array(using binary search method).
 * if it finds the current number it adds one to the variable that
 * hold the number of found numbers. it prints the proper data
 * to the file then it returns that number.
 */
void doSecondChild(int arr[ARR_LEN], int pipe_descs[]) {
    int rand_num, found_times = 0;
    struct timeval t0, t1;
    int task_time;

    gettimeofday(&t0, NULL);

    for (int i = 0; i < MILL; ++i) {
        rand_num = rand() % MILL;
        if (binary_search(arr, rand_num) == 1) {
            found_times++;
        }
    }
    gettimeofday(&t1, NULL);
    task_time = (int) ((double) (t1.tv_usec - t0.tv_usec) / 1000000 +
                       (double) (t1.tv_sec - t0.tv_sec));

    close(pipe_descs[0]);
    close(STDOUT_FILENO);
    dup(pipe_descs[1]);
    printf("b %d %d\n", found_times, task_time);
    close(pipe_descs[1]);

    exit(EXIT_SUCCESS);
}


/**
 * this is the function that receives an array and fills it
 * with the million values
 */
void fill_array(int arr[]) {
    int prev_val = 0;
    for (int i = 0; i < ARR_LEN; ++i) {
        arr[i] = prev_val + rand() % 10;
        prev_val = arr[i];
    }
}


/**
 * This function receives an array and a number.
 * then it searches the number in the array using primitive search.
 * it goes cell by cell searching for the number.
 * returns 1 if it found it, 0 otherwise
 */
int series_search(const int arr[ARR_LEN], int num_to_search) {
    for (int i = 0; i < ARR_LEN; ++i) {
        // no need to search we reached a number that's
        // bigger and the arr is sorted
        if (arr[i] > num_to_search) {
            return 0;
        }
        if (arr[i] == num_to_search) {
            return 1; // num found
        }
    }
    return 0; // num not found
}


/**
 * This function receives an array and a number.
 * then it searches the number in the array using binary search.
 * it goes cell by cell searching for the number.
 * returns 1 if it found it, 0 otherwise
 */
int binary_search(const int arr[ARR_LEN], int num_to_search) {
    int left = 0, right = ARR_LEN - 1, mid;

    while (left <= right) {
        mid = (left + right) / 2;
        if (arr[mid] == num_to_search) {
            return 1; // num found
        } else if (arr[mid] < num_to_search) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return 0; // num not found
}
