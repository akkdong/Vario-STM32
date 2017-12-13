// Standby.ino
//

#include <STM32Sleep.h>
#include <libmaple/pwr.h>
#include <libmaple/scb.h>

uint32_t lastTick;
int ledState;

void goToSleep2(SleepMode mode)
{
    // Clear PDDS and LPDS bits
    PWR_BASE->CR &= ~PWR_CR_PDDS;
    PWR_BASE->CR &= ~PWR_CR_LPDS;

    // Clear previous wakeup register
    PWR_BASE->CR |= PWR_CR_CWUF;

    if (mode == STANDBY)
	{
		PWR_BASE->CR |= PWR_CR_PDDS;
    }

    PWR_BASE->CR |= PWR_CR_LPDS;

    SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;
	

    // Now go into stop mode, wake up on interrupt
    asm("    WFI");  
}

void enterStandbyMode()
{
	//
	PWR_BASE->CSR |= PWR_CSR_EWUP;
	delay(2);
	
	//
	PWR_BASE->CR |= PWR_CR_CWUF;
	PWR_BASE->CR |= PWR_CR_PDDS;
	SCB_BASE->SCR |= SCB_SCR_SLEEPDEEP;
	
	//
	asm("    WFI");  
}

void toggleLed()
{
	if (millis() - lastTick > 500)
	{
		lastTick = millis();
		ledState = 1 - ledState;
		
		Serial.println(ledState ? "On" : "Off");
		digitalWrite(PD2, ledState ? LOW : HIGH);
	}
}

void setup()
{
	//
	delay(1000);
	Serial.begin(115200);
	
	pinMode(PD2, OUTPUT);
	pinMode(PA0, INPUT_PULLDOWN);
	
	lastTick = millis();
	ledState = 1;
	digitalWrite(PD2, ledState ? LOW : HIGH);
	
	while (Serial.available())
		Serial.read();
}

void loop()
{
	if (Serial.available() )
	{
		while (Serial.available())
			Serial.read();
		
		//goToSleep2(STANDBY);
		enterStandbyMode();
	}
	
	toggleLed();
}