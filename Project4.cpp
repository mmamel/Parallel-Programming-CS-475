#include <xmmintrin.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <iostream>
#include <stdio.h>
#include <fstream>

#define SSE_WIDTH		4
#ifndef NUMARR
#define NUMARR 1
#endif

#ifndef NUMT
#define NUMT 2
#endif

#ifndef NUMTRIES
#define NUMTRIES	3
#endif

#define NUM_ELEMENTS_PER_CORE NUMARR/NUMT
float SimdMulSum (float *a, float *b, int len);

int main(int argc, char *argv[ ] ){
    #ifndef _OPENMP
        frpintf(stderr, "No openmp\n");
        return 1
    #endif

    omp_set_num_threads( NUMT );
    float *arr_ptr = new float[NUMARR];
    float *arr_ptr2 = new float[NUMARR];

    for (int i=0;i<NUMARR;i++){
        arr_ptr[i] = log(i);
        arr_ptr2[i]= log(i);
    }
    float maxSIMDPerformance = 0;
    float maxNORMPerformance = 0;
    float bestsum=0;
    std::ofstream fs;
    fs.open("data.csv", std::ofstream::app);
    for(int i = 0; i<NUMTRIES;i++){
        double time0=omp_get_wtime();
        #pragma omp parallel
        {
            int first = omp_get_thread_num() * NUM_ELEMENTS_PER_CORE;
            int sum = SimdMulSum(&arr_ptr[first], &arr_ptr2[first], NUM_ELEMENTS_PER_CORE);

        }
        double time1 = omp_get_wtime();
        double megaMultsPerSecond = (double)NUMARR / ( time1 - time0) / 1000000.;
        if( megaMultsPerSecond > maxSIMDPerformance ){
            maxSIMDPerformance = megaMultsPerSecond;
        }
        int sum_temp = 0;
        if(NUMT== 1){
            time0=omp_get_wtime();
            for(int j=0;j<NUMARR;j++){
                sum_temp += arr_ptr[j] * arr_ptr2[j];
            }
            time1=omp_get_wtime();
            megaMultsPerSecond = (double)NUMARR / (time1-time0)/1000000.;
            if(megaMultsPerSecond > maxNORMPerformance){
                maxNORMPerformance = megaMultsPerSecond;
            }
        }
    }
    if(NUMT == 1){
        fs << NUMARR << ","<<maxNORMPerformance << ",";
    }
    fs << maxSIMDPerformance;
    if(NUMT == 8){
        fs << "\n";
    }
    else{
        fs << ",";
    }
    return 0;
}
float
SimdMulSum( float *a, float *b, int len )
{
	float sum[4] = { 0., 0., 0., 0. };
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;
	register float *pa = a;
	register float *pb = b;

	__m128 ss = _mm_loadu_ps( &sum[0] );
	for( int i = 0; i < limit; i += SSE_WIDTH )
	{
		ss = _mm_add_ps( ss, _mm_mul_ps( _mm_loadu_ps( pa ), _mm_loadu_ps( pb ) ) );
		pa += SSE_WIDTH;
		pb += SSE_WIDTH;
	}
	_mm_storeu_ps( &sum[0], ss );

	for( int i = limit; i < len; i++ )
	{
		sum[0] += a[i] * b[i];
	}

	return sum[0] + sum[1] + sum[2] + sum[3];
}