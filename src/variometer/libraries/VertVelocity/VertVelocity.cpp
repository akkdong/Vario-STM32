#include <VertVelocity.h>

#include <Arduino.h>

void VertVelocity::init(float startp, float starta, float sigmap, float sigmaa, unsigned long timestamp) {

  /* init base values */
  p = startp;
  v = 0;
  a = starta;
  t = timestamp;
  calibrationDrift = 0.0;
    
  /* init variance */
  varp = sigmap * sigmap;
  vara = sigmaa * sigmaa;

  /* init covariance matrix */
  p11 = 0;
  p12 = 0;
  p21 = 0;
  p22 = 0;
}

void VertVelocity::update(float mp, float ma, unsigned long timestamp) {

  /**************/
  /* delta time */
  /**************/
  unsigned long deltaTime = timestamp - t;
  float dt = ((float)deltaTime)/1000.0;
  t = timestamp;

  /**************/
  /* prediction */
  /**************/

  /* values */
  a = ma;  // we use the last acceleration value for prediction 
  float dtPower = dt * dt; //dt^2
  p += dt*v + dtPower*a/2;
  v += dt*a;
  //a = ma; // uncomment to use the previous acceleration value 

  /* covariance */
  float inc;
  
  dtPower *= dt;  // now dt^3
  inc = dt*p22+dtPower*vara/2;
  dtPower *= dt; // now dt^4
  p11 += dt*(p12 + p21 + inc) - (dtPower*vara/4);
  p21 += inc;
  p12 += inc;
  p22 += dt*dt*vara;

  /********************/
  /* gaussian product */
  /********************/

  /* kalman gain */
  float s, k11, k12, y;

  s = p11 + varp;
  k11 = p11/s;
  k12 = p12/s;
  y = mp - p;

  /* update */
  p += k11 * y;
  v += k12 * y;
  p22 -= k12 * p21;
  p12 -= k12 * p11;
  p21 -= k11 * p21;
  p11 -= k11 * p11;
 
}

float VertVelocity::getPosition() {

  return p;
}

float VertVelocity::getCalibratedPosition() {

  return (p + calibrationDrift);
}

float VertVelocity::getVelocity() {

  return v;
}

float VertVelocity::getAcceleration() {

  return a;
}

unsigned long VertVelocity::getTimestamp() {

  return t;
}

void VertVelocity::calibratePosition(float newPosition) {

  calibrationDrift = newPosition - p;
}
