/*
 *   Sieve of Eratosthenes
 *
 *   Programmed by Michael J. Quinn
 *
 *   Last modification: 7 September 2001
 */

#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define MIN(a,b)  ((a)<(b)?(a):(b))
#define BLOCK_LOW(id, p, n) ((id) * (n) / (p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1, p, n) - 1)


int main (int argc, char *argv[])
{
   	unsigned long int    count;        /* Local prime count */
   	double elapsed_time; /* Parallel execution time */
   	unsigned long int    first;        /* Index of first multiple */
   	int   local_first;
   	unsigned long int    global_count = 0; /* Global prime count */
   	unsigned long long int    high_value;   /* Highest value on this proc */
   	unsigned long int    i;
   	int    id;           /* Process ID number */
   	unsigned long int    index;        /* Index of current prime */
   	unsigned long long int    low_value;    /* Lowest value on this proc */
   	char  *marked;       /* Portion of 2,...,'n' */
   	char  *local_prime_marked;
   	unsigned long long int    n;            /* Sieving from 2, ..., 'n' */
   	int    p;            /* Number of processes */
   	unsigned long int    proc0_size;   /* Size of proc 0's subarray */
   	unsigned long int    prime;
   	unsigned long int  local_prime;        /* Current prime */
   	unsigned long int    size;         /* Elements in 'marked' */
   	unsigned long int  local_prime_size;


   	MPI_Init (&argc, &argv);

   	/* Start the timer */

   	MPI_Comm_rank (MPI_COMM_WORLD, &id);
   	MPI_Comm_size (MPI_COMM_WORLD, &p);
   	MPI_Barrier(MPI_COMM_WORLD);
   	elapsed_time = -MPI_Wtime();

   	if (argc != 2) {
      	if (!id) printf ("Command line: %s <m>\n", argv[0]);
      	MPI_Finalize();
      	exit (1);
   		}

   	n = atoll(argv[1]);

   	/* Figure out this process's share of the array, as
      well as the integers represented by the first and
      last array elements */

   	/* Add you code here  */


	low_value = 2 + BLOCK_LOW(id, p, n-1); 
	high_value = 2 + BLOCK_HIGH(id, p, n-1); 

	low_value += (low_value+1)%2; 
	high_value -= (high_value+1)%2; 
	size = (high_value - low_value)/2 + 1; 
	local_prime_size = (int)sqrt((double)n) - 1; 

	
	proc0_size = (n/2 -1)/p;

	if((proc0_size + 2) < (int)sqrt(((double)n)/2)){
		if(!id) printf("Too many processes\n");
		MPI_Finalize();
		exit(1);
	}

	marked = (char*) malloc(size); 
	local_prime_marked = (char*)malloc(local_prime_size); 

	if(marked == NULL || local_prime_marked == NULL){
		printf("idL %d - Cannot allocate enough memory\n", id);
		MPI_Finalize();
		exit(1); 
	}

	local_prime = 2; 
	for(i = 0; i<size; i++) marked[i] = 0; 
	for(i = 0; i<local_prime_size; i++) local_prime_marked[i] = 0; 

	index = 0;
	do{
		local_first = local_prime * local_prime -2; 
		for(i = local_first; i<local_prime_size; i+= local_prime) local_prime_marked[i] = 1; 
		while(local_prime_marked[++index]); 
		local_prime = 2 + index; 
	}while(local_prime*local_prime <= n); 



	unsigned long int block_size = 1048576; 
	unsigned long long int block_low_val = low_value; 
	unsigned long long int block_high_val = block_low_val + 2*(block_size-1); 

	do{
		index = 0;
		prime = 3; 
		while(prime * prime <= block_high_val){
			if(prime*prime>block_low_val) first = (prime*prime - block_low_val)/2; 
			else{
				if(!(block_low_val%prime)) first = 0;
				else first = (prime - (block_low_val%prime) + block_low_val/prime % 2*prime) / 2; 
			}
			for(i=first+(block_low_val - low_value) / 2; i<= (block_high_val - low_value)/2; i+= prime) marked[i] = 1; 
		
		while(local_prime_marked[++index]); 
		prime = index+2; 
		}

		block_low_val = block_high_val + 2; 
		block_high_val = block_low_val + 2*(block_size - 1); 
		if(block_high_val > high_value) block_high_val = high_value; 

	}while(block_low_val <= high_value);


	count = 0;
	for(i = 0; i < size; i++) {
		if(!marked[i]) count++;
	}
	if(!id) count++; 
	if(p>1) MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD); 


   /* Stop the timer */

   elapsed_time += MPI_Wtime();


   /* Print the results */

   if (!id) {
      printf("The total number of prime: %ld, total time: %10.6f, total node %d\n", global_count, elapsed_time, p);

   }
   MPI_Finalize ();
   return 0;
}

