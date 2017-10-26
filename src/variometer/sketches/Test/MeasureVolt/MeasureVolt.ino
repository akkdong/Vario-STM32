// MeasureVolt.ino
//

#include <BatteryVoltage.h>

#define TEST_CLASS      1

BatteryVoltage batVolt;
unsigned long lastTick;

void setup()
{
	//
	delay(1000);
	
	//
	Serial.begin(115200);
	while(! Serial);
	
	Serial.println("Voltage Measurement Test!!!");
	lastTick = millis();
	
	#if TEST_CLASS
	batVolt.begin(PA0);
	#else
	pinMode(PA0, INPUT_ANALOG);
	#endif // TEST_CLASS
}



void loop()
{
	if ((millis() - lastTick) > 1000)
	{
		#if TEST_CLASS
		double v = batVolt.getVoltage();
		#else
		double v = ADC_TO_VOLTAGE(analogRead(PA0));
		#endif // TEST_CLASS
		
		Serial.print("measure = "); Serial.println(v);
		lastTick = millis();
	}
	
	batVolt.update();
}
