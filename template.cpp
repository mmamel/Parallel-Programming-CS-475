#include <stdio.h>
#include <math.h>
#include <iostream>
#include <omp.h>

#ifndef NUMT
#define NUMT 1
#endif

#ifndef NUMTRIES
#define NUMTRIES 5
#endif

#ifndef SIZE
#define SIZE 20000
#endif

float A[SIZE];
float B[SIZE];
float C[SIZE];
//g++ -o proj proj.cpp -lm -fopenmp

int main(int argc, char ** argv){
   #ifndef _OPENMP
        fprintf( stderr, "OpenMP is not supported here -- sorry.\n" );
        return 1;
   #endif
	// inialize the arrays:
   volatile double sum=0;
	for( int i = 0 ;i < SIZE; i++ )
	{
		A[ i ] = 1.;
		B[ i ] = 2.;
	}

        omp_set_num_threads( NUMT );
        fprintf( stderr, "Using %d threads\n", NUMT );

        double maxMegaMults = 0.;

        for( int t = 0; t < NUMTRIES; t++ )
        {
                double time0 = omp_get_wtime( );

                #pragma omp parallel for
                for( int i = 0; i < SIZE; i++ )
                {
                        C[i] = A[i] * B[i];
                }
                double time1 = omp_get_wtime( );
                double megaMults = (double)SIZE/(time1-time0)/1000000.;
                sum += megaMults;
                if( megaMults > maxMegaMults )
                        maxMegaMults = megaMults;
        }
        printf( "Peak Performance 1 = %8.2lf MegaMults/Sec\n", maxMegaMults );
        printf( "Baseline Performance 1 = %8.2lf MegaMults/Sec\n", sum/(double)NUMTRIES);
        
        double maxMegaMults4 = 0.;
        sum = 0;
        #undef NUMT
        #define NUMT 4
        omp_set_num_threads( NUMT );
        fprintf( stderr, "Using %d threads\n", omp_get_num_threads() );

        for( int t = 0; t < NUMTRIES; t++ )
        {
                double time0 = omp_get_wtime( );

                #pragma omp parallel for
                for( int i = 0; i < SIZE; i++ )
                {
                        C[i] = A[i] * B[i];
                }
                double time1 = omp_get_wtime( );
                double megaMults4 = (double)SIZE/(time1-time0)/1000000.;
                sum += megaMults4;
                if( megaMults4 > maxMegaMults4 )
                        maxMegaMults4 = megaMults4;
        }
        printf( "Peak Performance 4 = %8.2lf MegaMults/Sec\n", maxMegaMults4 );
        printf( "Baseline Performance 4 = %8.2lf MegaMults/Sec\n", sum/(double)NUMTRIES);
	// note: %lf stands for "long float", which is how printf prints a "double"
	//        %d stands for "decimal integer", not "double"

   double S = maxMegaMults4 / maxMegaMults;
   printf("S = %8.2lf\n", S);
   float Fp = (4./3.) * (1. - (1./S));
   printf("Fp = %8.2lf\n", Fp);

   return(0);
}
