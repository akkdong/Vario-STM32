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

TonePlayer::TonePlayer(int id, int ch) : Timer(id), timerCh(ch)
{
	playCurr.playType	= PLAY_NONE;
	playNext.playType	= PLAY_NONE;
	
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
	
	//
	Timer.pause();
	//Timer.setMode(timerCh, TIMER_OUTPUT_PWM);
	//Timer.setPeriod(1000000)
	//Timer.setCompare(timerCh, 0); // always LOW
	//Timer.refresh();
	//Timer.resume();
}

void TonePlayer::setMelody(Tone * tonePtr, int toneCount, int repeat, int instant)
{
	playNext.tonePtr 		= tonePtr;
	playNext.toneCount		= toneCount;
	playNext.repeatCount	= repeat;
	playNext.playType		= PLAY_MELODY;
	
	if (instant)
		updateNow();
}

void TonePlayer::setBeep(int freq, int period, int duty) 
{
	// update next beep tone
	beepToneNext[0].freq = freq;
	beepToneNext[0].length	= duty;

	beepToneNext[1].freq = 0;
	beepToneNext[1].length	= period - duty;
	
	//
	playNext.tonePtr 		= &beepTone[0];	// beepToneNext? ??? ?? updateNow ???? beepTone?? ????.
	playNext.toneCount		= sizeof(beepTone) / sizeof(beepTone[0]); // 2
	playNext.repeatCount	= 0; // infinite repeat
	playNext.playType		= PLAY_BEEP;
}

void TonePlayer::setMute(int instant)
{
	playNext.tonePtr 		= &muteTone[0];
	playNext.toneCount		= sizeof(muteTone) / sizeof(muteTone[0]); // 1
	playNext.repeatCount	= 0; // infinite repeat
	playNext.playType		= PLAY_MUTE;
	
	if (instant)
		updateNow();
}

void TonePlayer::setTone(int freq, int vol)
{
	if (vol > 0)
		setVolume(vol);
	
	if (freq > 0)
	{
		Timer.pause();
		//
		Timer.setPeriod(1000000.0/freq); // 50Hz -> 1/50 period(s) -> 1/50*1000000 period(us)
		Timer.setCompare(timerCh, Timer.getOverflow() * 200.0 / volume); // 100% volume -> 50% duty cycle : overflow * 100 / (volume / 2)
		Timer.refresh();
		Timer.resume();
	}
	else
	{
		Timer.pause();
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
	if (updateCheck())
		updateNow();
}

int TonePlayer::updateCheck()
{
	if (playCurr.playType == PLAY_NONE)
	{
		//if (playNext.playType != PLAY_NONE)
		//	return 1;
		//
		//return 0;
		
		return 1;
	}
	
	if (playCurr.tonePtr[toneIndex].length > 0)
	{
		unsigned long now = millis();

		if (toneStartTick + playCurr.tonePtr[toneIndex].length <= now)
		{
			toneIndex += 1;
			
			if (toneIndex < playCurr.toneCount)
			{
				toneStartTick = now;
				setTone(playCurr.tonePtr[toneIndex].freq, volume);
			}
			else
			{
				// now all tone is played
				// move next or repeat again ?
				playCount += 1;
				
				if (playCurr.repeatCount == 0 || playCount < playCurr.repeatCount)
				{
					if (playCurr.repeatCount == 0 && playNext.playType != PLAY_NONE)
						return 1; // play next
					// else play again
					
					// ??? tone ?? ??...
					toneIndex = 0;
					toneStartTick = now;
					setTone(playCurr.tonePtr[toneIndex].freq, volume);
				}
				else // repeatCount > 0 && playerCount == repeatCount
				{
					playCurr.playType = PLAY_NONE; // stop play
					setTone(); // mute
					
					return 1; // play next
				}
			}
		}
		else
		{
			// ?? ? ???? ?? ?? ??? ??? ????? ??? ??.
			// !!! BUT !!! ???? ???? ??? ??? ??? ??? ?? ???? ?? ???? ?? ??? ??.
			if (toneStartTick + playCurr.tonePtr[toneIndex].length - GAP_BETWEEN_TONE <= now)
				setTone(); // mute
		}
	}
	else
	{
		// mute, sinking beep ?? length? 0? ?? ?? ??? ??? ?? ??? ????.
		if (playNext.playType != PLAY_NONE)
			return 1;
	}

	return 0;
}

void TonePlayer::updateNow()
{
	if (playNext.playType == PLAY_NONE)
		return;
	
	// copy next context to current 
	playCurr = playNext;
	// copy beepToneNext to beepTone if next play type is beep
	if (playCurr.playType == PLAY_BEEP)
	{
		beepTone[0] = beepToneNext[0];
		beepTone[1] = beepToneNext[1];
	}

	// reset next play
	playNext.playType = PLAY_NONE;
	
	//
	toneIndex = 0;
	toneStartTick = millis();
	playCount = 0;
	
	setTone(playCurr.tonePtr[toneIndex].freq, volume);
}
