// TonePlayer.cpp
//

#include "TonePlayer.h"


////////////////////////////////////////////////////////////////////////////////////
//

static Tone muteTone[] = 
{ 
	{ 0, 0 }
};

static Tone beepTone[] = 
{
	{ 100, 100 },
	{   0, 100 }
};

static Tone beepToneNext[] = 
{
	{ 100, 100 },
	{   0, 100 }
};


////////////////////////////////////////////////////////////////////////////////////
//

TonePlayer::TonePlayer(ToneGenerator & gen) : toneGen(gen)
{
	playTone.playType	= PLAY_NONE;
	nextTone.playType	= PLAY_NONE;
	
	/*
	//
	soundPlayPtr	= 0;
	soundPlayCount 	= 0;
	soundPlayType	= PLAY_NONE;
	
	//
	soundNextPtr 	= 0;
	soundNextCount	= 0;
	soundNextType	= PLAY_NONE;
	*/
	
	//
	toneIndex 		= 0;
	toneStartTick 	= 0;
	//
	playCount 		= 0;
	
	//
	volume			= 100;
	volumeRecovery	= -1;

	//
	//Timer.pause();
	//Timer.setMode(timerCh, TIMER_OUTPUT_PWM);
	//Timer.setPeriod(1000000)
	//Timer.setCompare(timerCh, 0); // always LOW
	//Timer.refresh();
	//Timer.resume();
}

void TonePlayer::setMelody(Tone * tonePtr, int toneCount, int repeat, int instant)
{
	nextTone.tonePtr 		= tonePtr;
	nextTone.toneCount		= toneCount;
	nextTone.repeatCount	= repeat;
	nextTone.playType		= PLAY_MELODY;
	
	if (instant)
		playNext();
}

void TonePlayer::setMelodyEx(Tone * tonePtr, int toneCount)
{
	nextTone.tonePtr 		= tonePtr;
	nextTone.toneCount		= toneCount;
	nextTone.repeatCount	= 1;
	nextTone.playType		= PLAY_MELODY;
	
	volumeRecovery = volume;
	volume = RECOVERY_TONE_VOLUME;
	
	playNext();
}

void TonePlayer::setBeep(int freq, int period, int duty) 
{
	// update next beep tone
	beepToneNext[0].freq = freq;
	beepToneNext[0].length	= duty;

	beepToneNext[1].freq = 0;
	beepToneNext[1].length	= period - duty;
	
	//
	nextTone.tonePtr 		= &beepTone[0];	// beepToneNext? ??? ?? nextTone ???? beepTone?? ????.
	nextTone.toneCount		= sizeof(beepTone) / sizeof(beepTone[0]); // 2
	nextTone.repeatCount	= 0; // infinite repeat
	nextTone.playType		= PLAY_BEEP;
}

void TonePlayer::setMute(int instant)
{
	nextTone.tonePtr 		= &muteTone[0];
	nextTone.toneCount		= sizeof(muteTone) / sizeof(muteTone[0]); // 1
	nextTone.repeatCount	= 0; // infinite repeat
	nextTone.playType		= PLAY_MUTE;
	
	if (instant)
		playNext();
}

void TonePlayer::setTone(int freq, int vol)
{
	if (vol > 0)
		setVolume(vol);
	
	if (freq > 0)
	{
		/*
		//
		pinMode(PA8, PWM);
		//
		Timer.pause();
		Timer.setMode(timerCh, TIMER_PWM);
		Timer.setPeriod(1000000.0/freq); // 50Hz -> 1/50 period(s) -> 1/50*1000000 period(us)
		Timer.setCompare(timerCh, Timer.getOverflow() * volume / 200.0); // 100% volume -> 50% duty cycle : overflow * 100 / (volume / 2)
		Timer.refresh();
		Timer.resume();
		*/
		
		toneGen.setTone(freq, volume);
	}
	else
	{
		/*
		//
		pinMode(PA8, INPUT);
		//
		//Timer.pause();
		Timer.setMode(timerCh, TIMER_DISABLED);
		//Timer.resume();
		*/
		
		toneGen.setTone(0);
	}
}

void TonePlayer::setVolume(int value)
{
	if (value < 0)
		volume = 0;
	else if (value > 100)
		volume = 100;
	else
		volume = value;
}

void TonePlayer::update()
{
	if (playCheck())
	{
		if (volumeRecovery >= 0)
		{
			setVolume(volumeRecovery);
			volumeRecovery = -1;
		}
		
		playNext();
	}
}

int TonePlayer::playCheck()
{
	if (playTone.playType == PLAY_NONE)
	{
		//if (nextTone.playType != PLAY_NONE)
		//	return 1;
		//
		//return 0;
		
		return 1;
	}
	
	if (playTone.tonePtr[toneIndex].length > 0)
	{
		unsigned long now = millis();

		if (toneStartTick + playTone.tonePtr[toneIndex].length <= now)
		{
			toneIndex += 1;
			
			if (toneIndex < playTone.toneCount)
			{
				toneStartTick = now;
				setTone(playTone.tonePtr[toneIndex].freq, volume);
			}
			else
			{
				// now all tone is played
				// move next or repeat again ?
				playCount += 1;
				
				if (playTone.repeatCount == 0 || playCount < playTone.repeatCount)
				{
					// interrupt continuous tone if next tone exist
					if (playTone.repeatCount == 0 && nextTone.playType != PLAY_NONE)
						return 1; // play next
					
					// else replay from first tone
					toneIndex = 0;
					toneStartTick = now;
					setTone(playTone.tonePtr[toneIndex].freq, volume);
				}
				else // repeatCount > 0 && playerCount == repeatCount
				{
					playTone.playType = PLAY_NONE; // stop play
					setTone(); // mute
					
					return 1; // play next
				}
			}
		}
		else
		{
			// insert mute gap between tones.
			if (toneStartTick + playTone.tonePtr[toneIndex].length - GAP_BETWEEN_TONE <= now)
				setTone(); // mute
		}
	}
	else
	{
		// 
		if (nextTone.playType != PLAY_NONE)
			return 1;
	}

	return 0;
}

void TonePlayer::playNext()
{
	if (nextTone.playType == PLAY_NONE)
		return;
	
	// copy next context to current 
	playTone = nextTone;
	// copy beepToneNext to beepTone if next play type is beep
	if (playTone.playType == PLAY_BEEP)
	{
		beepTone[0] = beepToneNext[0];
		beepTone[1] = beepToneNext[1];
	}

	// reset next play
	nextTone.playType = PLAY_NONE;
	
	//
	toneIndex = 0;
	toneStartTick = millis();
	playCount = 0;
	
	setTone(playTone.tonePtr[toneIndex].freq, volume);
}
