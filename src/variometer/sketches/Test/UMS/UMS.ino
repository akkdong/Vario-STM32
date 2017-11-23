// UMS.ino
//
	
#include <DefaultSettings.h>
#include <SdFat.h>
#include <UsbMassStorage.h>

#define SDCARD_CS  PIN_SD_CS
#define SDCARD_SPI SDCARD_CHANNEL

#define SerialDbg		Serial1


class Sd2CardEx : public SdSpiCard
{
public:
	Sd2CardEx() {
		m_spi.setPort(1);
	}
	Sd2CardEx(uint8_t spiPort) {
		m_spi.setPort(spiPort);
	}

	/** Initialize the SD card.
	* \param[in] csPin SD chip select pin.
	* \param[in] settings SPI speed, mode, and bit order.
	* \return true for success else false.
	*/
	bool begin(uint8_t csPin = SS, SPISettings settings = SD_SCK_MHZ(50)) {
		return SdSpiCard::begin(&m_spi, csPin, settings);
	}
	
private:
	SdFatSpiDriver m_spi;
};


Sd2CardEx sdcard(SDCARD_SPI);

uint32_t MAL_massBlockCount[2];
uint32_t MAL_massBlockSize[2];
	
	
void setup()
{
	//
	pinMode(PIN_BT_EN, OUTPUT);
	digitalWrite(PIN_BT_EN, LOW);

	//
	Serial.end();

	//
	SerialDbg.begin(115200);
	delay(5000);	
	SerialDbg.println("UMS Test!!");

	//
	sdcard.begin(SDCARD_CS, SPI_QUARTER_SPEED);
	
	uint32_t numberOfBlocks = sdcard.cardSize();
	SerialDbg.print("Number of Blocks = "); SerialDbg.println(numberOfBlocks);
	
	MAL_massBlockCount[0] = numberOfBlocks;
	MAL_massBlockCount[1] = 0;
	MAL_massBlockSize[0] = 512;
	MAL_massBlockSize[1] = 0;
	SerialDbg.print("cardSize = "); SerialDbg.println(numberOfBlocks * 512);
	
	//
	SerialDbg.println("USBMassStorage.begin");
	USBMassStorage.begin();
}

void loop()
{
	USBMassStorage.loop();
}


/*
 * The following methods are used by the USBMassStorage driver to read and write to the SDCard.  
 */

extern "C" uint16_t usb_mass_mal_init(uint8_t lun)
{
	return 0;
}

extern "C" uint16_t usb_mass_mal_get_status(uint8_t lun)
{
	return sdcard.errorCode();
}

extern "C" uint16_t usb_mass_mal_write_memory(uint8_t lun, uint32_t memoryOffset, uint8_t *writebuff, uint16_t transferLength)
{
	uint32_t block = memoryOffset / 512;

	if (lun != 0)
		return USB_MASS_MAL_FAIL;

	if (sdcard.writeBlock(block, writebuff))
		return USB_MASS_MAL_SUCCESS;

	return USB_MASS_MAL_FAIL;
}

extern "C" uint16_t usb_mass_mal_read_memory(uint8_t lun, uint32_t memoryOffset, uint8_t *readbuff, uint16_t transferLength)
{
	if (lun != 0)
		return USB_MASS_MAL_FAIL;

	if (sdcard.readBlock(memoryOffset / 512, readbuff))
	return USB_MASS_MAL_SUCCESS;

	return USB_MASS_MAL_FAIL;
}

extern "C" void usb_mass_mal_format()
{
}
