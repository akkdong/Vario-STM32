#ifndef __VERTVELOCITY_H__
#define __VERTVELOCITY_H__

#include <Arduino.h>

/*********************************************************/
/* compute velocity from known position and acceleration */
/* p = position, v = velocity, a = acceleration          */
/*********************************************************/

class VertVelocity
{

 public:
  /**********************************************************/
  /*           init with your first measured values         */
  /*  !!! sigmap and sigmaa are very important values !!!   */
  /* make experimentations : ex sigmap = 0.1 , sigmaa = 0.3 */
  /**********************************************************/
  void init(float startp, float starta, float sigmap, float sigmaa, unsigned long timestamp);

  /* run each time you get new values */
  void update(float mp, float ma, unsigned long timestamp);

  /* at any time get result */
  float getPosition();
  float getCalibratedPosition();
  float getVelocity();
  float getAcceleration();
  unsigned long getTimestamp();

  /* reset the current position without changing velocity and acceleration */
  void calibratePosition(float newPosition);

 private:
  /* position variance, acceleration variance */
  float varp, vara;
  
  /* position, velocity, acceleration, timestamp */
  float p, v, a;
  unsigned long t;

  /* calibration */
  float calibrationDrift;

  /* covariance matrix */
  float p11, p21, p12, p22;
  
};

#endif // __VERTVELOCITY_H__
