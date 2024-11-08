/*
Copyright (C) 2001-2009 Makoto Matsumoto and Takuji Nishimura.
Copyright (C) 2009 Mutsuo Saito
All rights reserved.

Redistribution and use in source and binary forms, with or without
                                                             modification, are
permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above
          copyright notice, this list of conditions and the following
              disclaimer in the documentation and/or other materials provided
             with the distribution.

         THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
         "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
          OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
                         LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfU    // Constant vector a
#define UPPER_MASK 0x80000000U  // Most significant w-r bits
#define LOWER_MASK 0x7fffffffU  // Least significant r bits

extern uint32_t mt[N];   // The array for the state vector
extern int mti;  // mti == N+1 means mt[N] is not initialized

// Initialize the generator from a seed
static void
init_genrand(uint32_t s)
{
    mt[0] = s & 0xffffffffU;
    for (mti = 1; mti < N; mti++) {
        mt[mti] = (1812433253U * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti);
        mt[mti] &= 0xffffffffU;  // For >32 bit machines
    }
}

// Generate a random number on [0, 0xffffffff]-interval
static uint32_t
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
static float
rand_f(void)
{
    return (float)(genrand_int32() * (1.0 / 4294967295.0));  // Dividing by 2^32-1
}

static float
rand_between(float lo, float hi)
{
    return (float)(lo + (hi - lo) * rand_f());
}

typedef struct {
    float min;
    float max;
    int randomize;
    int in_use;
} generator;

static float
genrand_from(const generator *g)
{
    return g->randomize ? rand_between(g->min, g->max) : g->min;
}
