// config.h
//

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <VarioSettings.h>
#include <I2CDevice.h>


// target definition
//

#define TARGET_STM32DUINO_VARIOMETER

// sensor definition
// 
// it supports MPU6050, MPU6500, MPU9150, MPU9250

#define MPU6050


/* The following functions must be defined for this platform:
 * i2c_write(unsigned char slave_addr, unsigned char reg_addr,
 *      unsigned char length, unsigned char const *data)
 * i2c_read(unsigned char slave_addr, unsigned char reg_addr,
 *      unsigned char length, unsigned char *data)
 * delay_ms(unsigned long num_ms)
 * get_ms(unsigned long *count)
 * reg_int_cb(void (*cb)(void), unsigned char port, unsigned char pin)
 * labs(long x)
 * fabsf(float x)
 * min(int a, int b)
 */

#define i2c_write   		!I2C.writeBytes
#define i2c_read    		!I2C.readBytes
#define delay_ms    		delay

inline void get_ms(long unsigned int *timestamp)
{
    *timestamp = millis(); 
}

inline void reg_int_cb(struct int_param_s *int_param)
{
	// nop
}

#define log_i(...)     		do {} while (0)
#define log_e(...)     		do {} while (0)

// below functions are standcard c++ function
//
// labs(long x)
// fabsf(float x)
// min(int a,b)


// nop 
//
#define __no_operation()	__asm__("nop\n\t")


#endif // __CONFIG_H__
