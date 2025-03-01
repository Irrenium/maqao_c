#ifdef OPT1

/* Removing of store to load dependency (array ref replaced by scalar) */
#include <omp.h>
// n x n, row-major float matrix c
// vectors a, b each of length n
// We assume c is not constant across calls; otherwise, consider precomputing sums.
void kernel(unsigned n, float a[n], float b[n], float c[n][n]) {
#pragma omp parallel for  // parallelize over i
    for (unsigned i = 0; i < n; i++) {
        float temp  = a[i];
        float inv_b = 1.0f / b[i];  // single division
        float sum   = 0.0f;

        // Potentially vectorizable
#pragma omp simd reduction(+:sum)
        for (unsigned j = 0; j < n; j++) {
            sum += c[i][j];
        }

        temp += sum * inv_b;
        a[i] = temp;
    }
}

#elif defined OPT2

#include <string.h> // memset
//#include <immintrin.h> // For AVX/SSE intrinsics

void kernel(unsigned n, float a[n], float b[n], float c[n][n]) {
    unsigned i, j;

    for (i = 0; i < n; i++) {
        float temp = 0.0f;
        // Unrolling the inner loop for better performance
        for (j = 0; j + 4 <= n; j += 4) {
            temp += c[i][j] + c[i][j+1] + c[i][j+2] + c[i][j+3];
        }
        // Handle remaining elements
        for (; j < n; j++) {
            temp += c[i][j];
        }
        a[i] += temp / b[i];  // Reduce division operations
    }
}

#else

/* original */
void kernel (unsigned n, float a[n], float b[n], float c[n][n]) {
	unsigned i , j ;
	for ( j =0; j < n ; j ++)
		for ( i =0; i < n ; i ++)
			a [ i ] += c [ i ][ j ] / b[ i ];
}

#endif
