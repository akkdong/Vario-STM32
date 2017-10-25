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

//TonePlayer	tonePlayer(PLAYER_TIMER_ID, PLAYER_TIMER_CH);
//VarioBeeper	varioBeeper(tonePlayer);

unsigned long lastTick;
int playType = 0;


void setup()
{
	//
	delay(1000);
	
	//
	Serial.begin(115200);
	Serial.println("Tone Test!!!");
	
	//tonePlayer.setTone(1000, 100);
	//delay(2000);
	
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
	
	pinMode(PA8, PWM);
	
	Timer1.pause();
	Timer1.setMode(1, TIMER_PWM);
	Timer1.setPrescaleFactor(72); // 72MHz / 72 = 1MHz
	Timer1.setOverflow(5000);	  // 1MHz / 5000 = 200Hz
//	Timer1.setPeriod(100000);
	Timer1.setCompare(1, 0);
	Timer1.refresh();
	Timer1.resume();
	
	lastTick = millis();
}

void loop()
{
	static int percent = 0;
	
	if (millis() - lastTick > 100)
	{
		//
		Timer1.setCompare(1, Timer1.getOverflow() * percent / 100);
		
		if (percent == 0)
			Serial.println("0%");
		
		//
		percent += 1;
		
		if( percent > 50)
			percent = 0;
		
		//
		lastTick = millis();
	}
}
