// VarioBeeper.cpp
//

#include "VarioBeeper.h"

#define BEEP_TYPE_SILENT				(0)
#define BEEP_TYPE_SINKING				(1)
#define BEEP_TYPE_CLIMBING				(2)



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

struct VarioTone
{
	double 	velocity;
	int 	freq;
	int 	period;
	int 	duty;
};

static VarioTone varioTone[] = 
{
#if 0
	{ -10.00,	 200,	100,	100 }, 
	{  -3.00,	 280,	100,	100 },
	{  -0.51,	 300,	500,	100 },
	{  -0.50,	 200,	800,	  5 },
	{   0.09,	 400,	600,	 10 },
	{   0.10,	 640,	600,	 50 },
	{   1.16,	 880,	552*1.0, 52 },
	{   2.67,	1180,	483*0.9, 55 },
	{   4.24,	1400,	412*0.8, 58 },
	{   6.00,	1580,	322*0.7, 62 },
	{   8.00,	1750,	241*0.6, 66 },
	{  10.00,	1900,	150*0.5, 70 },
#else	
	{ -10.0,   200, 200, 100 },
	{  -3.0,   293, 200, 100 },
	{  -2.0,   369, 200, 100 },
	{  -1.0,   440, 200, 100 },
	{  -0.5,   475, 600, 100 },
	{   0.0,   493, 600,  50 },
	{   0.37, 1000, 369,  50 },
	{   0.92, 1193, 219,  50 },
	{   1.90, 1324, 151,  50 },
	{   3.01, 1428, 112,  50 },
	{   5.35, 1567, 100,  50 },
	{  10.00, 1687,  83,  50 },
#endif
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

VarioBeeper::VarioBeeper(TonePlayer & tp, double ct, double st) :	player(tp)
{
	setThreshold(ct, st);
}

void VarioBeeper::setThreshold(double ct, double st)
{
	climbingThreshold = ct;
	sinkingThreshold = st;
}

void VarioBeeper::setVelocity(double velocity)
{
	boolean typeChanged = false;
	
	switch (beepType)
	{
	case BEEP_TYPE_SINKING :
		if (sinkingThreshold + Config.vario_sensitivity < velocity)
			typeChanged = true;
		break;
		
	case BEEP_TYPE_SILENT :
		if (velocity <= sinkingThreshold || climbingThreshold <= velocity)
			typeChanged = true;
		break;
		
	case BEEP_TYPE_CLIMBING :
		if (velocity < climbingThreshold - Config.vario_sensitivity)
			typeChanged = true;
		break;
	}
	
	if (typeChanged)
	{
		if (velocity <= sinkingThreshold)
			beepType = BEEP_TYPE_SINKING;
		else if (climbingThreshold <= velocity)
			beepType = BEEP_TYPE_CLIMBING;
		else
			beepType = BEEP_TYPE_SILENT;
	}
	
	if (beepType != BEEP_TYPE_SILENT)
	{
		int freq, period, duty;
		
		findTone(velocity, freq, period, duty);		
		player.setBeep(freq, period, duty);
	}
	else
	{
		player.setMute(0);
	}
}

void VarioBeeper::findTone(double velocity, int & freq, int & period, int & duty)
{
	int index;
	
	for (index = 0; index < (sizeof(varioTone) / sizeof(varioTone[0])); index++)
	{
		if (velocity <= varioTone[index].velocity)
			break;
	}
	
	if (index == 0 || index == (sizeof(varioTone) / sizeof(varioTone[0])))
	{
		if (index != 0)
			index -= 1;
		
		freq = varioTone[index].freq;
		period = varioTone[index].period;
		duty = varioTone[index].duty;
	}
	else
	{
		double ratio = varioTone[index].velocity / velocity;
		
		freq = (varioTone[index].freq - varioTone[index-1].freq) / (varioTone[index].velocity - varioTone[index-1].velocity) * (velocity - varioTone[index-1].velocity) + varioTone[index-1].freq;
		period = (varioTone[index].period - varioTone[index-1].period) / (varioTone[index].velocity - varioTone[index-1].velocity) * (velocity - varioTone[index-1].velocity) + varioTone[index-1].period;
		duty = (varioTone[index].duty - varioTone[index-1].duty) / (varioTone[index].velocity - varioTone[index-1].velocity) * (velocity - varioTone[index-1].velocity) + varioTone[index-1].duty;
	}
	
	//period = (int)(period * 1.0);
	duty = (int)(period * duty / 100.0);
	
	if (period == duty)
		period = duty = 0; // infinite beepping
	
	//Serial.print(freq); Serial.print(", "); Serial.print(period); Serial.print(", "); Serial.println(duty);
}
