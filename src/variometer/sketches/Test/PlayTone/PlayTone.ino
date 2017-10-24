// PlayTone.ino
//

#include <TonePlayer.h>
#include <VarioBeeper.h>
#include <HardwareTimer.h>

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

TonePlayer	tonePlayer(PLAYER_TIMER_ID, PLAYER_TIMER_CH);
//VarioBeeper	varioBeeper(tonePlayer);

unsigned long lastTick;
int playType = -1;


void setup()
{
	//
	delay(1000);
	
	//
	Serial.begin(115200);
	Serial.println("Tone Test!!!");
	
	tonePlayer.setTone(1000, 100);
	delay(2000);
	
	lastTick = millis();
	
	/*
	pinMode(PA0, PWM);
	pinMode(PA1, PWM);
	pinMode(PA2, PWM);
	pinMode(PA3, PWM);
	
	Timer2.pause();
	Timer2.setMode(1, TIMER_PWM);
	Timer2.setMode(2, TIMER_PWM);
	Timer2.setMode(3, TIMER_PWM);
	Timer2.setMode(4, TIMER_PWM);
	Timer2.setPeriod(1000);
	Serial.print("Overflow = "); Serial.println(Timer2.getOverflow());
	Timer2.setCompare(1, Timer2.getOverflow()/2);
	Timer2.setCompare(2, Timer2.getOverflow()/3);
	Timer2.setCompare(3, Timer2.getOverflow()/4);
	Timer2.setCompare(4, Timer2.getOverflow()/5);
	Timer2.refresh();
	Timer2.resume();
	*/	
	
	/*
	Timer1.pause();
	Timer1.setMode(1, TIMER_PWM);
	Timer1.setPeriod(1000000);
	Timer1.setCompare(1, Timer1.getOverflow() / 2);
	Timer1.refresh();
	Timer1.resume();
	*/
}

void loop()
{
	if (playType < 0 || (millis() - lastTick > 5000))
	{
		if (playType < 1)
		{
			playType = 1;
			tonePlayer.setVolume(100);
			Serial.println("100%");
		}
		else
		{
			playType = 0;
			tonePlayer.setVolume(10);
			Serial.println("10%");
		}
		
		tonePlayer.setMelody(&startTone[0], sizeof(startTone) / sizeof(startTone[0]), 1, 0);
		lastTick = millis();
	}
	
	//
	tonePlayer.update();
	
	/*
	Serial.println("Play with Maximum Volume");
	delay(5000);
	
	Serial.println("Play with 30% Volume");
	tonePlayer.setVolume(30);
	tonePlayer.setMelody(&startTone[0], sizeof(startTone) / sizeof(startTone[0]), 1, 0);
	delay(5000);
	*/
}
