#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfU    // Constant vector a
#define UPPER_MASK 0x80000000U  // Most significant w-r bits
#define LOWER_MASK 0x7fffffffU  // Least significant r bits

static uint32_t mt[N];   // The array for the state vector
static int mti = N + 1;  // mti == N+1 means mt[N] is not initialized

// Initialize the generator from a seed
void
init_genrand(uint32_t s)
{
    mt[0] = s & 0xffffffffU;
    for (mti = 1; mti < N; mti++) {
        mt[mti] = (1812433253U * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
        mt[mti] &= 0xffffffffU;  // For >32 bit machines
    }
}

// Generate a random number on [0, 0xffffffff]-interval
uint32_t
genrand_int32(void)
{
    uint32_t y;

    if (mti >= N) {  // Generate N words at one time
        static const uint32_t mag01[2] = {0x0U, MATRIX_A};
        int kk;

        if (mti == N + 1)
            init_genrand(5489U);

        for (kk = 0; kk < N - M; kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1U];
        }
        for (; kk < N - 1; kk++) {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1U];
        }
        y = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
        mt[N - 1] = mt[M - 1] ^ (y >> 1) ^ mag01[y & 0x1U];

        mti = 0;
    }

    y = mt[mti++];

    // Tempering
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680U;
    y ^= (y << 15) & 0xefc60000U;
    y ^= (y >> 18);

    return y;
}

// Generate a random number on [0,1]-real-interval
double
genrand_real1(void)
{
    return genrand_int32() * (1.0 / 4294967295.0);  // Dividing by 2^32-1
}


double
rand_real_between(double lo, double hi)
{
    return lo + (hi - lo) * genrand_real1();
}