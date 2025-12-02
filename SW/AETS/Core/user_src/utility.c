/*
 * utility.c
 *
 *  Created on: 8 Nov 2023
 *      Author: Standa
 */

#include "utility.h"
#include <sys/types.h>
#include <math.h>



/*
 * Calculates mean and stdev based on stream of values (online)
 * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance - Welford's online algorithm
 * Using __ieee754_sqrtf instead sqrtf because it does not link huge ddiv function
 */
void ut_CalculateOnlineMeanStdev(float newValue, ut_MeanStdevTd* result){
	if(result){
		result->Cnt++;														//Increment sample count
		float Delta1 = newValue - result->Mean;
		result->Mean = result->Mean + Delta1 / result->Cnt;					//Calculate mean
		result->M2 = result->M2 + Delta1 * ( newValue - result->Mean );		//Calculate M2
		result->Stdev = sqrtf( result->M2 / result->Cnt );

		if(newValue < result->Min) result->Min = newValue; 		//Not original fast algorithm
		else if(newValue > result->Max) result->Max = newValue; //Not original fast algorithm
	}
}


/*
 * Initializes/resets the storage for mean and stdev results and calculation parameters
 */
void ut_InitializeOnlineMeanStdev(ut_MeanStdevTd* result){
	if(result){
		for(uint8_t i=0; i<sizeof(ut_MeanStdevTd); i++) ((uint8_t*)result)[i] = 0;


		result->Min =  +INFINITY;
		result->Max = -INFINITY;
	}
}



//Calculates exponential moving average
//Tau ... time constant
//T ... sampling period
//Alpha = coefficient
//Output = (1-Alpha) * value + Alpha * PrevValue;
//Alpha = exp(-T/Tau);
//Tau = -T/ln(Alpha);
//T = -Tau * ln(Alpha);
float util_ExpMovAvg(float alpha, float prevVal, float newVal){
	return (1-alpha) * newVal + alpha * prevVal;
}


float util_CalculateAlpha(float cutOff_Hz, float sampFreq_Hz){
	return exp( -1 * M_TWOPI * cutOff_Hz / sampFreq_Hz );
}


float util_CalculateAlpha1(float timeConst_s, float sampPeriod_s){
	return exp( -1 * sampPeriod_s / timeConst_s );
}


float util_CalculateCutOff(float alpha, float sampFreq_Hz){
	return -1 * sampFreq_Hz * log(alpha) /  M_TWOPI;
}


