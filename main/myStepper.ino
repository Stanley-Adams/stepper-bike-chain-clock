/* stepTenMin
  Step clock number of steps to advance clock ten minutes
*/
void stepTenMin()
{
  Serial.println("Stepping ten mins");
  stepper.step(STEPS_TEN_MINS);
  Serial.println("Finished stepping");
  tenMinIntervalCount = tenMinIntervalCount + 1;
  Serial.print("Ten Min Batches Stepped: ");
  Serial.println(tenMinIntervalCount);
  correctTenMinSteps();//Check if correction step is needed
}

/* stepTenMin
  Check if correct step is required based on number of 10min intervals stepped
*/
void correctTenMinSteps()
{
  if (tenMinIntervalCount % 3 == 0)
  {
    Serial.println("Thirty min correction step of 1");
    stepper.step(CORRECTION_TENMININTERVAL);
    tenMinIntervalCount = 0;//Reset to prevent integer rollover bug
  }
}


/* timeToPosition
  Converts hours and minutes to position on clock.
  Position is based on 12:00, to 11:50 in ten minute intervals, as 0 to 71.
*/
unsigned int timeToPosition(unsigned int h, unsigned int m)
{

  unsigned int pos = (h % 12) * 6;//Convert to 12 hour time and calculate hour portion
  //Add minutes as ten minute chunks
  if (m >= 50)
  {
    pos += 5;
  } else if (m >= 40)
  {
    pos += 4;
  } else if (m >= 30)
  {
    pos += 3;
  } else if (m >= 20)
  {
    pos += 2;
  } else if (m >= 10)
  {
    pos += 1;
  }
  return pos;
}
