/******************************************************************************
* 
* dense_mm.c
* 
* This program implements a dense matrix multiply and can be used as a
* hypothetical workload. 
*
* Usage: This program takes a single input describing the size of the matrices
*        to multiply. For an input of size N, it computes A*B = C where each
*        of A, B, and C are matrices of size N*N. Matrices A and B are filled
*        with random values. 
*
* Written Sept 6, 2015 by David Ferry
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sched_monitor.h>


const int num_expected_args = 2;
const unsigned sqrt_of_UINT32_MAX = 65536;

int main( int argc, char* argv[] ){

	unsigned index, row, col; //loop indicies
	unsigned matrix_size, squared_size;
	double *A, *B, *C;

	if( argc != num_expected_args ){
		printf("Usage: ./dense_mm <size of matrices>\n");
		exit(-1);
	}

	matrix_size = atoi(argv[1]);
	
	if( matrix_size > sqrt_of_UINT32_MAX ){
		printf("ERROR: Matrix size must be between zero and 65536!\n");
		exit(-1);
	}

	squared_size = matrix_size * matrix_size;

	printf("Generating matrices...\n");

	A = (double*) malloc( sizeof(double) * squared_size );
	B = (double*) malloc( sizeof(double) * squared_size );
	C = (double*) malloc( sizeof(double) * squared_size );

	for( index = 0; index < squared_size; index++ ){
		A[index] = (double) rand();
		B[index] = (double) rand();
		C[index] = 0.0;
	}
        int fd, status;
        fd = open(DEV_NAME, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Could not open %s: %s\n", DEV_NAME, strerror(errno));
        return -1;
    }

    status = enable_preemption_tracking(fd);
    if (status < 0) {
        fprintf(stderr, "Could not enable preemption tracking on %s: %s\n", DEV_NAME, strerror(errno));
        close(fd);
        return -1;
    }
	printf("Multiplying matrices...\n");

	for( row = 0; row < matrix_size; row++ ){
		for( col = 0; col < matrix_size; col++ ){
			for( index = 0; index < matrix_size; index++){
			C[row*matrix_size + col] += A[row*matrix_size + index] *B[index*matrix_size + col];
			}	
		}
	}

	printf("Multiplication done!\n");
        status = disable_preemption_tracking(fd);
    if (status < 0) {
        fprintf(stderr, "Could not disable preemption tracking on %s: %s\n", DEV_NAME, strerror(errno));
        close(fd);
        return -1;
    }

    printf("Tracking part finished.\n"); 
    printf("Start recording data.\n");
    int ret = 1;
    int count = 0;
    int migration = 0;
    double mean_sched_in = 0;
    double mean_sched_out = 0;
    double total_sched_in = 0;
    double total_sched_out = 0;
    struct preemption_info *prev, *cur;
    char prev_buff[100], cur_buff[100];
    int count_rcu = 0;
    char test[4] = "rcu";
    while (ret != 0) {
        if (count > 0) {
            strcpy(prev_buff, cur_buff);
            prev = (struct preemption_info *) prev_buff;
        }
        ret = read(fd, cur_buff, sizeof(struct preemption_info));
        if (ret == 0) {
		printf("Preemption FInished.\n");
		break;
	}
        cur = (struct preemption_info *)cur_buff;
        if (cur->end_time == 0) {
		printf("Current not preempted.\n");
		break;
	}
        if (strncmp(cur->preempt_process_name, test, 3) == 0 ) {
		count_rcu ++;
	}
        printf("Cur_length: %llu\n", cur->end_time - cur->start_time);
        mean_sched_in = (double) count / (count+1)  * mean_sched_in + (double)(cur->end_time - cur->start_time) / (count + 1);
        total_sched_in += cur->end_time - cur->start_time;
        if (count > 0) {
		printf("Cur_gap: %llu\n", cur->start_time - prev->end_time);
           mean_sched_out =  (double)(count - 1)/count * mean_sched_out + (cur->start_time - prev->end_time) / (double)count;
		total_sched_out += cur->start_time - prev->end_time;
		if (cur->core_num != prev->core_num) {
			migration += 1;
		}
       };
       count += 1; 
    }   
    printf("Count rcu %d\n", count_rcu);
    printf("Mean Sched_in: %f, Mean sched_out: %f, # migrations: %d, # preemptions: %d\n", mean_sched_in, mean_sched_out, migration, count); 
	printf("Total sched_in : %f, total sched_out: %f.\n", total_sched_in, total_sched_out);
	return 0;
}
