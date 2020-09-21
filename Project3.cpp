#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <iostream>
#include <stdio.h>
#include <fstream>

int	NowYear;		// 2020 - 2025
int	NowMonth;		// 0 - 11
int i = 1;
float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
int	NowNumDeer;		// number of deer in the current population
int NowNumWolf;
int OldNumWolf;
unsigned int seed = 0;

const float GRAIN_GROWS_PER_MONTH =		9.0;
const float ONE_DEER_EATS_PER_MONTH =		1.0;

const float AVG_PRECIP_PER_MONTH =		7.0;	// average
const float AMP_PRECIP_PER_MONTH =		6.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;

void GrainDeer();
void Grain();
void Watcher(std::ofstream &);
void MyAgent();
float calculateHeight();
float tempFactor();
float precipFactor();
float Ranf( unsigned int *seedp,  float low, float high );
int Ranf( unsigned int *seedp, int ilow, int ihigh );
void incrementYear();
void calculateTemp();
void calculatePrecip();
int main(){
    std::ofstream myFile;
    myFile.open("data.csv");
        // starting date and time:
    NowMonth =    0;
    NowYear  = 2020;

    // starting state (feel free to change this if you want):
    NowNumDeer = 3;
    NowHeight =  3.;
    NowNumWolf = 1;
    OldNumWolf = 1;
    omp_set_num_threads( 4 );	// same as # of sections
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            GrainDeer( );
        }

        #pragma omp section
        {
            Grain( );
        }

        #pragma omp section
        {
            Watcher( myFile );
        }

        #pragma omp section
        {
            MyAgent( );	// your own
        }
    }       // implied barrier -- all functions must return in order
        // to allow any of them to get past here
    myFile.close();

    return 0;
}

float tempFactor(){
    return exp(-1*pow(((NowTemp-MIDTEMP)/10),2));
}

float precipFactor(){
    return exp(-1*pow(((NowPrecip-MIDPRECIP)/10),2));
}

void GrainDeer(){
    while(NowYear < 2026){
        float newNumDeer = NowNumDeer;
        if(NowNumDeer > NowHeight){
            newNumDeer--;
        }
        else if(NowNumDeer < NowHeight){
            newNumDeer++;
        }

        if(NowNumWolf > NowNumDeer){
            newNumDeer++;
        }
        if(newNumDeer <0){
            newNumDeer = 0;
        }
        //DoneComputing
        #pragma omp barrier
        NowNumDeer = newNumDeer;
        //DoneAssigning
        #pragma omp barrier
        //DonePrinting
        #pragma omp barrier
    }
}
void Grain(){
    while(NowYear < 2026){
        float newHeight = calculateHeight();
        //DoneComputing
        #pragma omp barrier
        NowHeight = newHeight;
        //DoneAssigning
        #pragma omp barrier
        //DonePrinting
        #pragma omp barrier
    }
}
void Watcher(std::ofstream& myFile){
    while(NowYear < 2026){
        //DoneComputing
        #pragma omp barrier
        //DoneAssigning
        #pragma omp barrier
        std::cout << "Month " << i << std::endl;
        std::cout<<"Temperture (C) = "<<(5./9.)*(NowTemp-32) << std::endl;
        std::cout<<"Precipitation (cm) = "<<NowPrecip*2.54 << std::endl;
        std::cout<<"Dear Count = "<<NowNumDeer << std::endl;
        std::cout <<"Grain Height (cm)= " << NowHeight*2.54<<std::endl;
        std::cout <<"Enviornmentalist Count = " << NowNumWolf << std::endl;
        std::cout << std::endl;
        myFile <<i<<","<< (5./9.)*(NowTemp-32) << "," << NowPrecip*2.54 << "," << NowNumDeer << ","<<NowHeight*2.54<<","<<NowNumWolf<<std::endl;
        i++;
        incrementYear();
        calculateTemp();
        calculatePrecip();
        //NOTE remeber to add in myagent value
        //DonePrinting
        #pragma omp barrier
    }
}
void MyAgent(){
    while(NowYear < 2026){
        int newWolf = NowNumWolf;

        if(newWolf <= NowNumDeer){
            newWolf++;
        }
        else if(newWolf > NowNumDeer){
            newWolf--;
        }
        if(newWolf <0){
            newWolf = 0;
        }
        //DoneComputing
        #pragma omp barrier
        // if(NowMonth ==4){
        //     OldNumWolf = NowNumWolf;
        // }
        // if(NowMonth==10){
        //     NowNumWolf = OldNumWolf;
        // }
        // if(NowMonth < 4 || NowMonth > 10){
            NowNumWolf=newWolf;
        // }
        // else{
        //     NowNumWolf = 0;
        // }
        //DoneAssigning
        #pragma omp barrier
        //DonePrinting
        #pragma omp barrier
    }
}
void incrementYear(){
    if(NowMonth == 11){
        NowMonth = 0;
        NowYear++;
    }
    else{
        NowMonth++;
    }
}
void calculateTemp(){
    float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );
    float temp = AVG_TEMP - AMP_TEMP * cos( ang );
    NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );
}
void calculatePrecip(){
    float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );
    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
    NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
    if( NowPrecip < 0. )
	    NowPrecip = 0.;
}
float Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX

        return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}
int Ranf( unsigned int *seedp, int ilow, int ihigh )
{
        float low = (float)ilow;
        float high = (float)ihigh + 0.9999f;

        return (int)(  Ranf(seedp, low,high) );
}
float calculateHeight(){
    float nextHeight = NowHeight;
    nextHeight += tempFactor() * precipFactor() * GRAIN_GROWS_PER_MONTH;
    nextHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;
    if(nextHeight < 0){
        nextHeight = 0;
    }
    return nextHeight;
}
