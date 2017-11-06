// PlayTone.ino
//

#include <TonePlayer.h>
#include <VarioBeeper.h>
#include <ToneGenerator.h>

#define PLAYER_TIMER_ID		(1)
#define PLAYER_TIMER_CH		(1)

static Tone startTone[] = {
	{ 262, 1000 / 4 }, 
	{ 196, 1000 / 8 }, 
	{ 196, 1000 / 8 }, 
	{ 220, 1000 / 4 }, 
	{ 196, 1000 / 4 }, 
	{   0, 1000 / 4 }, 
	{ 247, 1000 / 4 }, 
	{ 262, 1000 / 4 },
	{   0, 1000 / 8 }, 
};

ToneGenerator toneGen;
TonePlayer	tonePlayer(toneGen);
VarioBeeper	varioBeeper(tonePlayer);


unsigned long lastTick;
int playType = 0;


void setup()
{
	//
	delay(200);

	//
	Serial.begin(115200);
	Serial.println("Tone Test!!!");

	//
	toneGen.begin(PA8);
	
	//
	tonePlayer.setMelody(&startTone[0], sizeof(startTone) / sizeof(startTone[0]));
	lastTick = millis();
}

void loop()
{
	static int v = 0;
	
	tonePlayer.update();
	
	if ((millis() - lastTick) > 10000)
	{
		tonePlayer.setVolume(v ? 90 : 10);
		v = 1 - v;
		
		tonePlayer.setMelody(&startTone[0], sizeof(startTone) / sizeof(startTone[0]));
		lastTick = millis();
	}
}
