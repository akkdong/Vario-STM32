// TonePlayer.h
//

#ifndef __TONEPLAYER_H__
#define __TONEPLAYER_H__

#include <Arduino.h>
#include <ToneGenerator.h>


////////////////////////////////////////////////////////////////////////////////////
//

#define PLAY_NONE				(0)
#define PLAY_MUTE				(1)
#define PLAY_BEEP				(2)
#define PLAY_MELODY				(3)

#define SOUND_MUTE				(1)
#define SOUND_BEEP				(2)
#define SOUND_MELODY			(3)


#define GAP_BETWEEN_TONE		(1)		// 1ms

#define RECOVERY_TONE_VOLUME	(10)

////////////////////////////////////////////////////////////////////////////////////
//

struct Tone 
{
	int			freq;
	int			length;
};

struct PlayContext
{
	//
	Tone *			tonePtr;
	int				toneCount;
	
	//
	int				playType;
	
	//
	int				repeatCount;
};


////////////////////////////////////////////////////////////////////////////////////
//

class TonePlayer 
{
public:
	TonePlayer(ToneGenerator & gen);
	
public:	
	void			setMelody(Tone * tonePtr, int toneCount, int repeat = 1, int instant = 1);
	void			setMelodyEx(Tone * tonePtr, int toneCount);
	void			setBeep(int freq, int period, int duty); // duty --> ms, <= period
	void			setMute(int instant = 1);
	
	void			setTone(int freq = 0, int volume = -1);
	void			setVolume(int value);
	
	void			update();

private:
	int				playCheck();
	void			playNext();

public:
	//
	PlayContext		playTone;
	PlayContext		nextTone;
	
	/*
	//
	Sound *			soundPlayPtr;
	int				soundPlayCount;
	int				soundPlayType;

	//
	Sound *			soundNextPtr;
	int				soundNextCount;
	int				soundNextType;
	*/

	//
	int				toneIndex;
	unsigned long	toneStartTick;
	//
	int				playCount;
	
	//
	int				volume;
	int				volumeRecovery;
	
	//
	ToneGenerator &	toneGen;
};

#endif // __TONEPLAYER_H__
