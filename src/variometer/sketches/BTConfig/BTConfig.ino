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
	
	keyPowerBT.begin(PIN_BT_EN, ACTIVE_HIGH, OUTPUT_INACTIVE);
	keyPowerBT.enable();
	delay(1000);
	
	// find Baudrate of BT
	Serial.println("Find buad-rate of BT....");
	int buadrate = findBaudRate();
	Serial.print("    -> "); Serial.println(buadrate);
	
	if (buadrate) 
	{
		Serial1.begin(buadrate);
	}
	else
	{
		Serial.println("Fail to find baud-rate!!!");
		while(1);
	}
}

void loop()
{
	if (Serial.available())
		Serial1.write(Serial.read());
	
	if (Serial1.available())
		Serial.write(Serial1.read());
}
