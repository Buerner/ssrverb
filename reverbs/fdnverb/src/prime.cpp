#include "prime.h"
#include <math.h>

// function to check weather a given integer is a prime number
bool is_prime (long number)
{
	
    if ((number == 1) || (number == 2) || (number == 3)) return 1;
    
    // An even number is in in every case (except 2) not a prime number
    // thus we can skip those and increment by 2 in the following loop.
    // This takes approximately 50% less computaion time.
    if (number % 2 == 0 ) return 0;


    // calculate biggest possible devider, used as limit in loop
    int limit = floor(sqrt(number));
    
    // only check devider starting at 3 and increment by 2
    for(int i=3 ; i<=limit ; i+=2) {
            if ((number % i)==0) return 0;
    }
    
    return 1;
}

// function that returns the nth prime number
long nth_prime (int n)
{
    // catch first case 
    if (n == 1) return 2;

    else 
    {
    // start at 3, 2 is covered abover
	long i = 3;
	int prime_counter = 1;

	while (prime_counter < n){
	    if (is_prime(i)) prime_counter++;
            // increment by 2, even numbers can be ignored
            i+=2;
	}
	return i-2;
    }
}

long* get_n_primes(int n)
{
    long* result = new long[n];
    
    
    for (int i = 0; i<n; i++) {
        result[i] = nth_prime(i+1);
    }
    
    return result;
}
