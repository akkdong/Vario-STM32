// VarioBeeper.cpp
//

#include "VarioBeeper.h"

#define BEEP_TYPE_SILENT				(0)
#define BEEP_TYPE_SINKING				(1)
#define BEEP_TYPE_CLIMBING				(2)



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

/*
struct VarioTone
{
	float 	velocity;
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
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

VarioBeeper::VarioBeeper(TonePlayer & tp) :	player(tp)
{
}

void VarioBeeper::setVelocity(float velocity)
{
	boolean typeChanged = false;
	
	switch (beepType)
	{
	case BEEP_TYPE_SINKING :
		if (Config.vario.sinkThreshold + Config.vario.sensitivity < velocity)
			typeChanged = true;
		break;
		
	case BEEP_TYPE_SILENT :
		if (velocity <= Config.vario.sinkThreshold || Config.vario.climbThreshold <= velocity)
			typeChanged = true;
		break;
		
	case BEEP_TYPE_CLIMBING :
		if (velocity < Config.vario.climbThreshold - Config.vario.sensitivity)
			typeChanged = true;
		break;
	}
	
	if (typeChanged)
	{
		if (velocity <= Config.vario.sinkThreshold)
			beepType = BEEP_TYPE_SINKING;
		else if (Config.vario.climbThreshold <= velocity)
			beepType = BEEP_TYPE_CLIMBING;
		else
			beepType = BEEP_TYPE_SILENT;
	}
	
	if (beepType != BEEP_TYPE_SILENT)
	{
		int freq, period, duty;
		
		findTone(velocity, freq, period, duty);		
		player.setBeep(freq, period, duty, 0, Config.volume.vario);
	}
	else
	{
		player.setMute(0);
	}
}

void VarioBeeper::findTone(float velocity, int & freq, int & period, int & duty)
{
	int index;
	
	for (index = 0; index < (sizeof(Config.toneTable) / sizeof(Config.toneTable[0])); index++)
	{
		if (velocity <= Config.toneTable[index].velocity)
			break;
	}
	
	if (index == 0 || index == (sizeof(Config.toneTable) / sizeof(Config.toneTable[0])))
	{
		if (index != 0)
			index -= 1;
		
		freq = Config.toneTable[index].freq;
		period = Config.toneTable[index].period;
		duty = Config.toneTable[index].duty;
	}
	else
	{
		float ratio = Config.toneTable[index].velocity / velocity;
		
		freq = (Config.toneTable[index].freq - Config.toneTable[index-1].freq) / (Config.toneTable[index].velocity - Config.toneTable[index-1].velocity) * (velocity - Config.toneTable[index-1].velocity) + Config.toneTable[index-1].freq;
		period = (Config.toneTable[index].period - Config.toneTable[index-1].period) / (Config.toneTable[index].velocity - Config.toneTable[index-1].velocity) * (velocity - Config.toneTable[index-1].velocity) + Config.toneTable[index-1].period;
		duty = (Config.toneTable[index].duty - Config.toneTable[index-1].duty) / (Config.toneTable[index].velocity - Config.toneTable[index-1].velocity) * (velocity - Config.toneTable[index-1].velocity) + Config.toneTable[index-1].duty;
	}
	
	//period = (int)(period * 1.0);
	duty = (int)(period * duty / 100.0);
	
	if (period == duty)
		period = duty = 0; // infinite beepping
	
	//Serial.print(freq); Serial.print(", "); Serial.print(period); Serial.print(", "); Serial.println(duty);
}
