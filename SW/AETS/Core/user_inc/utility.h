/*
 * utility.h
 *
 *  Created on: 8 Nov 2023
 *      Author: Standa
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <stdint.h>


#define UT_ISNAN(x)			        ( (x) != (x) )
#define UT_SIZEOFARRAY(x)		    ( sizeof( (x) ) / sizeof( (x[0]) ) )
#define UT_VALINRANGE(val,min,max)  ( ( val >= (min) ) && ( val <= (max) ) )
#define UT_SATURATE(val, min, max)	if( (val) > (max) ){ (val) = (max); }\
									else if( (val) < (min) ){ (val) = (min); }
#define UT_SAFEASSIGN(ptr,val)		if( (ptr) ){ ( *(ptr) = (val) ); };



typedef struct{
	float Mean;
	float Stdev;
	float Min;	//Not original fast algorithm
	float Max;	//Not original fast algorithm
	uint32_t Cnt;
	float M2;
}ut_MeanStdevTd;



extern void ut_InitializeOnlineMeanStdev(ut_MeanStdevTd* result);
extern void ut_CalculateOnlineMeanStdev(float newValue, ut_MeanStdevTd* result);

extern float util_ExpMovAvg(float alpha, float prevVal, float newVal);
extern float util_CalculateAlpha(float cutOff_Hz, float sampFreq_Hz);
extern float util_CalculateAlpha1(float timeConst_s, float sampPeriod_s);
extern float util_CalculateCutOff(float alpha, float sampFreq_Hz);

#endif /* UTILITY_H_ */
