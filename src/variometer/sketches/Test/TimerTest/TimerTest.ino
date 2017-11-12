// TimerTest.ino
//

#include <HardwareTimer.h>


void TimerProc()
{
	Serial.println(millis());
	Timer2.refresh();
}

void setup()
{
	//
	Serial.begin();
	while (! Serial);
	delay(1000);
	
	Serial.println("HardwareTimer test...");
	
	Timer2.pause();
	Timer2.setMode(1, TIMER_OUTPUT_COMPARE);
	Timer2.setPrescaleFactor(1024);
//	Timer2.setOverflow(704);
	Timer2.setCompare(1, 704);
	Timer2.setCount(0);
	Timer2.attachInterrupt(1, TimerProc);
	Timer2.resume();	
}

void loop()
{
}
