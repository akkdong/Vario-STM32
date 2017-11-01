// FuncKeyParser.cpp
//

#include "FuncKeyParser.h"

#define MAX_RESPONSE_INPUT_TIME		(3000)	// 3s

#define FREQ_C3		131
#define FREQ_C4		262
#define FREQ_G4		392

#define DELAY_LONG	300
#define DELAY_SHORT	100

Tone toneCancel[] = 
{ 
	{ FREQ_C3, DELAY_SHORT },
	{       0, DELAY_SHORT },
	{ FREQ_C3, DELAY_SHORT },
	{       0, DELAY_SHORT },
};

Tone toneOK[] = 
{ 
	{ FREQ_G4, DELAY_LONG },
	{       0, DELAY_SHORT },
};

Tone toneMode[] = 
{
	{ FREQ_G4, DELAY_LONG },
	{       0, DELAY_SHORT },
	{ FREQ_G4, DELAY_SHORT },
	{       0, DELAY_SHORT },
	{ FREQ_G4, DELAY_SHORT },
	{       0, DELAY_SHORT },
	{ FREQ_G4, DELAY_SHORT },
	{       0, DELAY_SHORT },
};

Tone toneVolume[] = 
{
	{ FREQ_G4, DELAY_SHORT },
	{       0, DELAY_SHORT },
	{ FREQ_G4, DELAY_LONG },
	{       0, DELAY_SHORT },
	{ FREQ_G4, DELAY_LONG },
	{       0, DELAY_SHORT },
	{ FREQ_G4, DELAY_LONG },
	{       0, DELAY_SHORT },
};


/////////////////////////////////////////////////////////////////////////////
// class FuncKeyParser

FuncKeyParser::FuncKeyParser(DigitalInput & input, CommandStack & stack, TonePlayer & player) : Input(input), Stack(stack),	Player(player)
{
	inputCmd = inputParam = 0;
}

void FuncKeyParser::update()
{
	//
	Input.update();
	
	//
	if (inputCmd && (millis() - inputTick) > MAX_RESPONSE_INPUT_TIME)
	{
		// play cancel melody
		Player.setMelody(&toneCancel[0], sizeof(toneCancel)/sizeof(toneCancel[0]));
		
		// reset input command
		inputCmd = 0;
	}
	
	//
	if (Input.fired())
	{
		uint8_t value = Input.getValue();

		switch (value)
		{
		case 0x80 : // count: 4, code: LSSS	// mode change : UMS
			inputCmd = CMD_MODE_SWITCH;
			inputParam = PARAM_SW_UMS;
			inputTick = millis();
			Player.setMelody(&toneMode[0], (value & 0xE0) >> 5);
			break;
		case 0x60 : // count: 3, code: LSS	// mode change : Calibration
			inputCmd = CMD_MODE_SWITCH;
			inputParam = PARAM_SW_CALIBRATION;
			inputTick = millis();
			Player.setMelody(&toneMode[0], (value & 0xE0) >> 5);
			break;
		
		case 0x87 : // count: 4, code: SLLL	// volume : loud
			inputCmd = CMD_SOUND_LEVEL;
			inputParam = PARAM_LV_LOUD;
			inputTick = millis();
			Player.setMelody(&toneVolume[0], (value & 0xE0) >> 5);
			break;
		case 0x63 : // count: 3, code: SLL	// volume : quiet
			inputCmd = CMD_SOUND_LEVEL;
			inputParam = PARAM_LV_QUIET;
			inputTick = millis();
			Player.setMelody(&toneVolume[0], (value & 0xE0) >> 5);
			break;
		case 0x41 : // count: 2, code: SL	// volume : mute
			inputCmd = CMD_SOUND_LEVEL;
			inputParam = PARAM_LV_MUTE;
			inputTick = millis();
			Player.setMelody(&toneVolume[0], (value & 0xE0) >> 5);
			break;
		
		/*
		case 0x67 : // count: 3, code: LLL
		case 0x65 : // count: 3, code: LLS
		case 0x65 : // count: 3, code: LSL
		case 0x64 : // count: 3, codd: LSS
		case 0x63 : // count: 3, code: SLL
		case 0x62 : // count: 3, code: SLS
		case 0x61 : // count: 3, code: SSL
		case 0x60 : // count: 3, code: SSS
		*/
		
		case 0x40 : // count: 2, code: SS	// CANCEL signal
			if (inputCmd)
			{
				// play cancel melody
				Player.setMelody(&toneCancel[0], sizeof(toneCancel)/sizeof(toneCancel[0]));
				
				// reset input command
				inputCmd = 0;
			}
			break;
		case 0x21 : // count: 1, code: L	// OK signal
			if (inputCmd)
			{
				// enqueue command & play ok melody
				Stack.enqueue(Command(CMD_FROM_KEY, inputCmd, inputParam));
				Player.setMelody(&toneOK[0], sizeof(toneOK)/sizeof(toneOK[0]));
				
				// reset input command
				inputCmd = 0;
			}
			break;
			
		case 0xFF : // long long press
			// command reset
			Stack.enqueue(Command(CMD_FROM_KEY, CMD_DEVICE_RESET, 0));
			//Player.setMelody(&toneReset[0], sizeof(toneReset)/sizeof(toneReset[0]));
			break;
		}
	}
}

// mode change
//
// <-- 0x80 or 0x60
// --> beep replay
// <-- 0x21(confirm), 0x40(cancel), timeout(cancel)
// --> beep replay
//
// generate command and enqueue to stack
//

// volume change
//
// similiar to mode change but code changed to 0x87(loud), 0x63(quiet), 0x41(mute)
// ...

