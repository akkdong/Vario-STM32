// VarioBeeper.h
//

#include <Arduino.h>
#include "TonePlayer.h"

#ifndef __VARIOBEEPER_H__
#define __VARIOBEEPER_H__

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define BEEP_DEFAULT_SINKING_THRESHOLD	(-3.0)
#define BEEP_DEFAULT_CLIMBING_THRESHOLD	(0.2)

#define BEEP_VELOCITY_SISITIVITY		(0.15)


#define BEEP_TYPE_SILENT				(0)
#define BEEP_TYPE_SINKING				(1)
#define BEEP_TYPE_CLIMBING				(2)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class VarioBeeper
{
public:
	VarioBeeper(TonePlayer & sp, double climbingThreshold = BEEP_DEFAULT_CLIMBING_THRESHOLD, double sinkingThreshold = BEEP_DEFAULT_SINKING_THRESHOLD);
	
public:
	void			setThreshold(double climbingThreshold, double sinkingThreshold);
	void			setVelocity(double velocity);
	
private:
	void 			findTone(double velocity, int & freq, int & period, int & duty);

private:
	double 			climbingThreshold;
	double			sinkingThreshold;
	
	uint8_t			beepType;	// sinking, silent, (gliding), climbling
	
	TonePlayer &	player;
};

#endif // __VARIOBEEPER_H__
