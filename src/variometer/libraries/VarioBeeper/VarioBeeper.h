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
	VarioBeeper(TonePlayer & sp);
	
public:
//	void			setThreshold(double climbingThreshold, double sinkingThreshold);
	void			setVelocity(double velocity);
	
private:
	void 			findTone(double velocity, int & freq, int & period, int & duty);

private:
//	double 			climbingThreshold;
//	double			sinkingThreshold;
	
	//
	TonePlayer &	player;
	
	//
	uint8_t			beepType;	// sinking, silent, (gliding), climbling
};

#endif // __VARIOBEEPER_H__
