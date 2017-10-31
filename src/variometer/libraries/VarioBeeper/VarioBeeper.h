// VarioBeeper.h
//

#ifndef __VARIOBEEPER_H__
#define __VARIOBEEPER_H__

#include <DefaultSettings.h>
#include <Arduino.h>
#include <TonePlayer.h>
#include <GlobalConfig.h>


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class VarioBeeper
{
public:
	VarioBeeper(TonePlayer & sp, double climbingThreshold = VARIOMETER_CLIMBING_THRESHOLD, double sinkingThreshold = VARIOMETER_SINKING_THRESHOLD);
	
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
