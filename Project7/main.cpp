#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <omp.h>
#include <fstream>
#include <xmmintrin.h>
#include <unistd.h>
#include "CL/cl.h"
#include "CL/cl_platform.h"

// setting the number of threads:
#ifndef NUMT
#define NUMT		4
#endif

using namespace std;

void printTime(double time0, double time1,int Size);
void saveData(ofstream & fs, float * Sums);
float SimdMulSum(float *a, float *b, int Size);
float SimdMulSumA(float *A, float *B, int Size);
void				Wait( cl_command_queue );
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
        cout << "Performance for 4-thread OpenMp: ";

    printTime(time0,time1,Size);
    saveData(fs, Sums);
    cout << "done with 4 thread"<<endl;
    //Simd
    time0=omp_get_wtime();
    printf("%d", omp_get_num_threads());
    for(int shift=0; shift<Size; shift++){
        Sums[shift] = SimdMulSum(&A[0], &A[0+shift],Size);
    }
    time1=omp_get_wtime();
        cout << "Performance for SIMD: ";

    printTime(time0,time1,Size);
    saveData(fs, Sums);
    cout << "done with SIMD"<<endl;

    //OPENCL
    FILE *fp;
    cl_int status;
    cl_platform_id platform;
    status = clGetPlatformIDs( 1, &platform, NULL );
    	if( status != CL_SUCCESS )
		fprintf( stderr, "clGetPlatformIDs failed (2)\n" );
    cl_device_id device;
    status = clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL );
    if( status != CL_SUCCESS )
		fprintf( stderr, "clGetDeviceIDs failed (2)\n" );
    fp = fopen( "AutoCorrelate.cl", "r" );

    if (fp == NULL){
        fprintf(stderr,"cannot open source");
    }
    
    float *hA =     new float[ 2*Size ];
    float *hSums  = new float[ 1*Size ];
    for(int i =0;i<Size*2;i++){
        hA[i] = A[i];
    }
    cl_context context = clCreateContext( NULL, 1, &device, NULL, NULL, &status );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateContext failed\n" );
    cout << "here5"<<endl;
    cl_command_queue cmdQueue = clCreateCommandQueue( context, device, 0, &status );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateCommandQueue failed\n" );

    cl_mem dA =     clCreateBuffer( context, CL_MEM_READ_ONLY,  2*Size*sizeof(cl_float), NULL, &status );
    if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateBuffer failed (1)\n" );
    cl_mem dSums  = clCreateBuffer( context, CL_MEM_WRITE_ONLY, 1*Size*sizeof(cl_float), NULL, &status );
    if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateBuffer failed (2)\n" );
    status = clEnqueueWriteBuffer( cmdQueue, dA, CL_FALSE, 0, 2*Size*sizeof(cl_float), hA, 0, NULL, NULL );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clEnqueueWriteBuffer failed (1)\n" );
    status = clEnqueueWriteBuffer( cmdQueue, dSums, CL_FALSE, 0, 1*Size*sizeof(cl_float), hSums, 0, NULL, NULL );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clEnqueueWriteBuffer failed (1)\n" );
    cout << "here1"<<endl;
    Wait( cmdQueue );
    fseek( fp, 0, SEEK_END );
	size_t fileSize = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	char *clProgramText = new char[ fileSize+1 ];		// leave room for '\0'
	size_t n = fread( clProgramText, 1, fileSize, fp );
	clProgramText[fileSize] = '\0';
	fclose( fp );
	if( n != fileSize )
		fprintf( stderr, "Expected to read %d bytes read from 'AutoCorrelate.cl' -- actually read %d.\n", fileSize, n );
    cout << "here2"<<endl;
	// create the text for the kernel program:

	char *strings[1];
	strings[0] = clProgramText;
	cl_program program = clCreateProgramWithSource( context, 1, (const char **)strings, NULL, &status );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateProgramWithSource failed\n" );
	delete [ ] clProgramText;

	// 8. compile and link the kernel code:

	char *options = { "" };
	status = clBuildProgram( program, 1, &device, options, NULL, NULL );
	if( status != CL_SUCCESS )
	{
		size_t size;
		clGetProgramBuildInfo( program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &size );
		cl_char *log = new cl_char[ size ];
		clGetProgramBuildInfo( program, device, CL_PROGRAM_BUILD_LOG, size, log, NULL );
		fprintf( stderr, "clBuildProgram failed:\n%s\n", log );
		delete [ ] log;
	}
    cout << "here3"<<endl;
	// 9. create the kernel object:

	cl_kernel kernel = clCreateKernel( program, "AutoCorrelate", &status );
	if( status != CL_SUCCESS )
		fprintf( stderr, "clCreateKernel failed\n" );

    status = clSetKernelArg( kernel, 0, sizeof(cl_mem), &dA );
    if( status != CL_SUCCESS )
		fprintf( stderr, "clSetKernelArg failed (1)\n" );
    status = clSetKernelArg( kernel, 1, sizeof(cl_mem), &dSums  );
    if( status != CL_SUCCESS )
		fprintf( stderr, "clSetKernelArg failed (1)\n" );
    

    size_t globalWorkSize[3] = { Size,         1, 1 };
    size_t localWorkSize[3]  = { 64,   1, 1 };

    Wait( cmdQueue );
    time0=omp_get_wtime();
    status = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, globalWorkSize,localWorkSize,0,NULL,NULL);
    if( status != CL_SUCCESS )
		fprintf( stderr, "clEnqueueNDRangeKernel failed: %d\n", status );

	Wait( cmdQueue );
    time1=omp_get_wtime();
    status = clEnqueueReadBuffer( cmdQueue, dSums, CL_TRUE, 0, 1*Size*sizeof(cl_float), hSums, 0, NULL, NULL );
	if( status != CL_SUCCESS )
			fprintf( stderr, "clEnqueueReadBuffer failed\n" );
    cout << "Performance for OpenCL: ";
    printTime(time0,time1, Size);
    saveData(fs,Sums);
    return 0;
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

float SimdMulSum(float *a, float *b, int Size){
    // sleep(5);
    int SSE_WIDTH = 4;
    int len = Size/omp_get_num_threads();
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

void
Wait( cl_command_queue queue )
{
      cl_event wait;
      cl_int      status;

      status = clEnqueueMarker( queue, &wait );
      if( status != CL_SUCCESS )
	      fprintf( stderr, "Wait: clEnqueueMarker failed\n" );

      status = clWaitForEvents( 1, &wait );
      if( status != CL_SUCCESS )
	      fprintf( stderr, "Wait: clWaitForEvents failed\n" );
}