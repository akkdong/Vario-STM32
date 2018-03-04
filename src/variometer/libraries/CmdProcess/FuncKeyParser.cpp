// FuncKeyParser.cpp
//

#include <GlobalConfig.h>
#include "FuncKeyParser.h"

#define MAX_RESPONSE_INPUT_TIME		(5000)	// 5s


/////////////////////////////////////////////////////////////////////////////
// class FuncKeyParser

FuncKeyParser::FuncKeyParser(FunctionKey & input, CommandStack & stack, TonePlayer & player) : Input(input), Stack(stack),	Player(player)
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
		Player.setBeep(NOTE_C3, 1000, 800, 1, Config.volume.effect);
		
		// reset input command
		inputCmd = 0;
	}
	
	//
	if (Input.fired())
	{
		uint16_t value = Input.getValue();
		//Serial.print("Key input = "); Serial.println(value, HEX);

		// replay key-input by sound
		Player.setMelody(Input.getTone(), Input.getToneCount(), 1, PLAY_PREEMPTIVE, Config.volume.effect);

		switch (value)
		{
		case 0x4008 : // count: 4, code: LSSS	// mode change : Calibration
			inputCmd = CMD_MODE_SWITCH;
			inputParam = PARAM_MS_CALIBRATION;
			inputTick = millis();
			//Serial.println("change to calibration mode");
			break;
		case 0x3004 : // count: 3, code: LSS	// mode change : UMS
			inputCmd = CMD_MODE_SWITCH;
			inputParam = PARAM_MS_UMS;
			inputTick = millis();
			//Serial.println("change to UMS mode");
			break;

		case 0x4007 : // count: 4, code: SLLL	// volume : loud
			inputCmd = CMD_SOUND_LEVEL;
			inputParam = PARAM_SL_LOUD;
			inputTick = millis();
			//Serial.println("change volume to loud level");
			break;
		case 0x3003 : // count: 3, code: SLL	// volume : quiet
			inputCmd = CMD_SOUND_LEVEL;
			inputParam = PARAM_SL_MEDIUM;
			inputTick = millis();
			//Serial.println("change volume to quiet level");
			break;
		case 0x2001 : // count: 2, code: SL	// volume : mute
			inputCmd = CMD_SOUND_LEVEL;
			inputParam = PARAM_SL_MUTE;
			inputTick = millis();
			//Serial.println("change volume to mute level");
			break;
		
		case 0x2000 : // count: 2, code: SS	// CANCEL signal
			if (inputCmd)
			{
				// reset input command
				inputCmd = 0;
				//Serial.println("cancel!!");
			}
			break;
		case 0x1001 : // count: 1, code: L	// OK signal
			if (inputCmd)
			{
				// enqueue command & play ok melody
				Stack.enqueue(Command(CMD_FROM_KEY, inputCmd, inputParam));
				//Serial.println("OK!!");
				//Serial.print("enqueue command: ");
				//Serial.write((char)(inputCmd >> 8)); Serial.write((char)(inputCmd & 0x00FF));
				//Serial.print(", "); 
				//Serial.println(inputParam);
		
				// reset input command
				inputCmd = 0;
			}
			break;
			
		case 0xFFFF : // long long press
			// command reset
			Stack.enqueue(Command(CMD_FROM_KEY, CMD_RESET, 0));
			break;
			
		default : // unsupport(unused) key-input
			Player.setBeep(NOTE_C3, 1000, 800, 1, Config.volume.effect);
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

