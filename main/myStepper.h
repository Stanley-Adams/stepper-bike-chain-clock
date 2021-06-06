#include <Stepper.h>

/* Macros */
#define STEPS_ONE_REV 2048 // number of steps in one revolution of motor
#define STEPS_HOUR STEPS_ONE_REV //Number of steps in an hour
#define STEPS_TEN_MINS 341 //Number of steps to do in ten minutes. A correction will be meeded as it is really 341.33 (surplus -1/3)
#define CORRECTION_TENMININTERVAL 1 //Every 3x10 mins, step this amount
#define INIT_STEP 0 //# of steps to do on powerup
#define MOTOR_RPM 10 //Max RPM around 10-15

/* Globals */
Stepper stepper(STEPS_ONE_REV, 8, 10, 9, 11); //Define stepper object with GPIO pins
unsigned int tenMinIntervalCount = 0; //Counts how many ten minute intervals have passed
