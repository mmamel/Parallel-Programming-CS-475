#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <iostream>
#include <stdio.h>

#define XMIN     -1.
#define XMAX      1.
#define YMIN     -1.
#define YMAX      1.

#ifndef NUMNODES
#define NUMNODES 10
#endif

#ifndef NUMT
#define NUMT 1
#endif

#ifndef N
#define N 2
#endif



// how many tries to discover the maximum performance:
#ifndef NUMTRIES
#define NUMTRIES	2
#endif

float Height( int, int );

int main( int argc, char *argv[ ] )
{
    #ifndef _OPENMP
        frpintf(stderr, "No openmp\n");
        return 1
    #endif
    omp_set_num_threads( NUMT );
    
    float maxPerformance = 0;
	float bestvolume = 0;
    // for (int t=0;t<NUMTRIES;t++){
        double time0=omp_get_wtime();
        float volume = 0;
    
        #pragma omp parallel for default(none) reduction(+:volume)
		for( long long int n = 0; n < NUMNODES*NUMNODES; n++ )
		{
            long long int iu = n % NUMNODES;
            long long int iv = n / NUMNODES;
            // the area of a single full-sized tile:

            float fullTileArea = (  ( ( XMAX - XMIN )/(float)(NUMNODES-1) )  *
                        ( ( YMAX - YMIN )/(float)(NUMNODES-1) )  );
            float height = Height(iu, iv);
            volume += (fullTileArea * height);
            // sum up the weighted heights into the variable "volume"
            // using an OpenMP for loop and a reduction:
            
        }
        double time1 = omp_get_wtime();
        double megaHeightsPerSecond = (double)NUMNODES*NUMNODES / ( time1 - time0) / 1000000.;
        if( megaHeightsPerSecond > maxPerformance ){
            maxPerformance = megaHeightsPerSecond;
            bestvolume = volume;
        }
    // }
        bestvolume *= 2;
        printf( "%d\t%d\t%8.3lf\t%8.3lf\n", NUMT, NUMNODES, bestvolume, maxPerformance );
}

float
Height( int iu, int iv )	// iu,iv = 0 .. NUMNODES-1
{
	float x = -1.  +  2.*(float)iu /(float)(NUMNODES-1);	// -1. to +1.
	float y = -1.  +  2.*(float)iv /(float)(NUMNODES-1);	// -1. to +1.

	float xn = pow( fabs(x), (double)N );
	float yn = pow( fabs(y), (double)N );
	float r = 1. - xn - yn;
	if( r < 0. )
	        return 0.;
	float height = pow( 1. - xn - yn, 1./(float)N );
    if(iu == 0){
        height /= 2;
        // if(iv == NUMNODES-1){
        //     height /=2;
        // }
    }
    if(iv == 0){
        height /= 2;
        // if(iu == NUMNODES-1){
        //     height /= 2;
        // }
    }
    if(iu == NUMNODES-1){
        height /= 2;
    }
    if(iv == NUMNODES-1){
        height /= 2;
    }
    // if( iv == NUMNODES -1 && iu == NUMNODES-1){
    //     height /= 2;
    // }
	return height;
}
