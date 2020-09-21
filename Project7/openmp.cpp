#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <omp.h>
#include <fstream>

using namespace std;
void printTime(double time0, double time1,int Size);
void saveData(ofstream & fs, float * Sums);

int main(){
    ofstream fs;
    fs.open("output.csv", ofstream::app);
    FILE *ft = fopen( "signal.txt", "r" );
    if( ft == NULL )
    {
        fprintf( stderr, "Cannot open file 'signal.txt'\n" );
        exit( 1 );
    }
    int Size;
    fscanf( ft, "%d", &Size );
    float *A =     new float[ 2*Size ];
    float *Sums  = new float[ 1*Size ];
    for( int i = 0; i < Size; i++ )
    {
        fscanf( ft, "%f", &A[i] );
        A[i+Size] = A[i];		// duplicate the array
    }
    fclose( ft );
    //one thread openmp
    omp_set_num_threads( 1 );
    float sum = 0.;
    double time0 = omp_get_wtime( );
    #pragma omp parallel for default(none) shared(Size, A, Sums) reduction(+:sum)
    for( int shift =0; shift < Size; shift++){
        sum = 0.;
        for( int i = 0; i <Size; i++){
            sum += A[i] * A[i + shift];
        }
        Sums[shift] = sum;
    }
    double time1 = omp_get_wtime();
    cout << "Performance for 1-thread OpenMp: ";
    printTime(time0,time1,Size);
    saveData(fs, Sums);
    cout << "Done with one thread"<<endl;
    //4 thread openmp
    omp_set_num_threads( 4 );
    time0=omp_get_wtime();
    #pragma omp parallel for default(none) shared(Size,A,Sums) reduction(+:sum)
    for (int shift=0;shift<Size;shift++){
        sum=0.;
        for(int i=0; i<Size;i++){
            sum+=A[i] * A[i+shift];
        }
        Sums[shift] = sum;
    }
    // omp_set_num_threads(1);
    time1 = omp_get_wtime();
    cout << "Performance for 1-thread OpenMp: ";

    printTime(time0,time1,Size);
    saveData(fs, Sums);
    cout << "done with 4 thread"<<endl;
}
void printTime(double time0, double time1, int Size){
    double megaTrialsPerSecond = (double)Size*Size / ( time1 - time0 ) / 1000000.;
    cout << megaTrialsPerSecond<< " megatrialspersecond"<<endl;
}
void saveData(ofstream & fs, float* Sums){
    for( int i=0;i<512;i++){
        fs << Sums[i];
        if (i!=511){
            fs << ",";
        }
        else{
            fs << "\n";
        }
    }
}