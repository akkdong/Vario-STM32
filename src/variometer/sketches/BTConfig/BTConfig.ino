// BTConfig.ino
//

#include <DefaultSettings.h>
#include <OutputKey.h>

OutputKey keyPowerBT;

int baud_rate[] =
{
	1200,
	2400,
	4800,
	9600,
	19200,
	38400,
	57600,
	115200,
};

int findBaudRate()
{
	for (int i = 0; i < sizeof(baud_rate) / sizeof(baud_rate[0]); i++)
	{
		//
		Serial.print("Try "); Serial.print(baud_rate[i]); Serial.print("...");
		Serial1.begin(baud_rate[i]);
		Serial1.print("AT\r");
		
		char buf[16];
		int n = 0;
		uint32_t tick = millis();
		
		while ((millis() - tick) < 5000)
		{
			if (Serial1.available())
			{
				int c = Serial1.read();
				//Serial.write(c);
				
				if (c == '\r' || c == '\n')
				{
					if (c == '\n' && n > 0)
						break;
				}
				else
				{
					buf[n] = c;
					n++;
					
					if (n == sizeof(buf))
						break;
				}
			}
		}
		
		//
		Serial1.end();
		delay(100);
		
		if ((n == 2 && buf[0] == 'O' && buf[1] == 'K') ||
			(n == 5 && buf[0] == 'E' && buf[1] == 'R' && buf[2] == 'R' && buf[3] == 'O' && buf[4] == 'R'))
		{
			Serial.println("   OK!!!");
			
			return baud_rate[i];
		}
		
		Serial.println("   FAILED!");
	}
	
	return 0;
}


void setup()
{
	// Initialize Serials
	Serial.begin();
	
	#if HW_VERSION == HW_VERSION_V1_REV2
	keyPowerBT.begin(PIN_BT_EN, ACTIVE_HIGH, OUTPUT_ACTIVE);
	#else // HW_VERSION_V1
	keyPowerBT.begin(PIN_BT_EN, ACTIVE_LOW, OUTPUT_ACTIVE);
	#endif
	keyPowerBT.enable();
	delay(1000);
	
	// find Baudrate of BT
	Serial.println("Find BT baud-rate....");
	int baudrate = findBaudRate();
	Serial.print("    -> "); Serial.println(baudrate);
	
	if (baudrate) 
	{
		Serial1.begin(baudrate);
		
		if (baudrate != BAUDRATE_BT)
		{
			Serial.print("Change BT baud-rate to "); Serial.println(BAUDRATE_BT);
			Serial.print("AT+BTUART="); Serial.print(BAUDRATE_BT); Serial.print("\r");
			Serial1.print("AT+BTUART="); Serial1.print(BAUDRATE_BT); Serial1.print("\r");
			delay(200);
			
			//while (Serial1.available())
			//	Serial.write(Serial1.read());
			//
			//Serial1.end();
			//Serial1.begin(BAUDRATE_BT);
		}
		else
		{
			Serial.print("BT communicate with "); Serial.print(baudrate); Serial.println(" bps");
		}
	}
	else
	{
		Serial.println("Fail to find baud-rate!!!");
		while(1);
	}
}

void loop()
{
	while (Serial.available())
		Serial1.write(Serial.read());
	
	while (Serial1.available())
		Serial.write(Serial1.read());
}
