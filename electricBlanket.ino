/*
curl https://api.particle.io/v1/devices/22001f000447343138333038/checkAction -d access_token=17eca6cd5930b921751370f8de980dfe3f59c2ac
The above works, now to pass params?

curl https://api.particle.io/v1/devices/22001f000447343138333038/checkAction -d access_token=17eca6cd5930b921751370f8de980dfe3f59c2ac -d params=high
success!!
*/
#define debug 0

//variables for knowing the state the blanket is in
int blanketState;
int tempState;
int tempCount;
int tempVal;
//variables for reading and contolling the buttons on the controller
int PowerRead = 2;
int upBut = 3;
int downBut = 4;
int PowerSwitch = 5;

void setup() {
  RGB.control(true);
  RGB.color(0, 0, 0);

  if(debug == 1)
  {
      Serial.begin(9600);
      // Wait for a USB serial connection for up to 30 seconds
      waitFor(Serial.isConnected, 30000);

      // Prints out the local IP over Serial.
      Serial.println(WiFi.localIP());
  }

  pinMode(PowerRead, INPUT);// set to inputs (High Z) so that the manual controls still work
  pinMode(PowerSwitch, INPUT);
  pinMode(upBut, INPUT);
  pinMode(downBut, INPUT);
  digitalWrite(PowerSwitch, HIGH);
  digitalWrite(upBut, HIGH);
  digitalWrite(downBut, HIGH);

// read the state of the blanket (on or off) upon startup to set the variable accordingly
  if(digitalRead(PowerRead) == LOW)
      blanketState = 0;
  if(digitalRead(PowerRead) == HIGH)
      blanketState = 1;

  //Upon hooking up the blanket to the Arduino, it needs to be set at 5 so it and the counter are synced up
  tempState = 1;//Medium
  tempCount = 5;//Medium

  Particle.function("checkAction", checkAction);
  Particle.function("setTemp", setTemp);

  Particle.variable("blanketState", &blanketState, INT);
  Particle.variable("tempState", &tempState, INT);
  Particle.variable("tempCount", &tempCount, INT);
}

void loop() {

  //If the buttons on the controller are pressed manually, keep track of each press.
  if(digitalRead(upBut) == LOW && digitalRead(PowerRead) == HIGH)//if the up button is pressed and the unit is on...
  {
    tempCount++;
    delay(200);//debounce
    if(tempCount > 10)//if temp count is already HIGH, stay at 10
    tempCount = 10;
  }
  if(digitalRead(downBut) == LOW && digitalRead(PowerRead) == HIGH)//if the down button is pressed and the unit is on...
  {
    tempCount--;
    delay(200);//debounce
    if(tempCount < 1)//if temp count is already LOW, stay at 1
    tempCount = 1;
  }

  if(digitalRead(PowerRead) == LOW)//If no power is read on the unit, it is off.
    blanketState = 0;
  if(digitalRead(PowerRead) == HIGH)//If power is read on the unit, it is on.
    blanketState = 1;

}


///////////////////////////////////////////////////////////////////////
int setTemp(String value)
{
  tempVal = value.toInt();

  if(tempVal < tempCount && tempVal > 0)
  {
    pinMode(downBut, OUTPUT);//set button to output to control controller
    do
    {
        digitalWrite(downBut, LOW);//simulate button presses
        delay(100);
        digitalWrite(downBut, HIGH);
        delay(100);
        tempCount--;
      }while(tempVal < tempCount);
    pinMode(downBut, INPUT);//set back to INPUT to allow manual presses
    return 1;
  }
  else if(tempVal > tempCount && tempVal < 10)
  {
    pinMode(upBut, OUTPUT);
      do
      {
        digitalWrite(upBut, LOW);
        delay(100);
        digitalWrite(upBut, HIGH);
        delay(100);
        tempCount++;
      }while(tempVal > tempCount);
    pinMode(upBut, INPUT);
    return 1;
  }
  else if(tempVal == tempCount)
  {
    //do nothing
  }
  else
  return -1;
}
///////////////////////////////////////////////////////////////////////
int checkAction(String msg)
{
  if (msg == "high" && blanketState == 1)// set to HIGH
  {
    if(tempCount < 10)//if in MED or LOW, go up to HIGH
    {
    pinMode(upBut, OUTPUT);//set button to output to control controller
      do
      {
        digitalWrite(upBut, LOW);//simulate button presses
        delay(100);
        digitalWrite(upBut, HIGH);
        delay(100);
        tempCount++;
      }while(tempCount < 10);
    pinMode(upBut, INPUT);//set back to INPUT to allow manual presses
    }

    tempState = 2;
    return 1;
  }
  //-------------------------------------------------------
  else if (msg == "med" && blanketState == 1)//Set to Medium
  {
    if(tempCount < 5)//if in LOW, go up to MED
    {
    pinMode(upBut, OUTPUT);
      do
      {
        digitalWrite(upBut, LOW);
        delay(100);
        digitalWrite(upBut, HIGH);
        delay(100);
        tempCount++;
      }while(tempCount < 5);
    pinMode(upBut, INPUT);
    }
    if(tempCount > 5)//if in HIGH, go down to MED
    {
    pinMode(downBut, OUTPUT);
      do
      {
        digitalWrite(downBut, LOW);
        delay(100);
        digitalWrite(downBut, HIGH);
        delay(100);
        tempCount--;
      }while(tempCount > 5);
    pinMode(downBut, INPUT);
    }

    tempState = 1;
    return 1;
  }
  //-------------------------------------------------------
  else if (msg == "low" && blanketState == 1)//Set to LOW
  {
    if(tempCount > 1)//if in HIGH or MED, go down to LOW
    {
    pinMode(downBut, OUTPUT);
      do
      {
        digitalWrite(downBut, LOW);
        delay(100);
        digitalWrite(downBut, HIGH);
        delay(100);
        tempCount--;
      }while(tempCount > 1);
    pinMode(downBut, INPUT);
    }

    tempState = 0;
    return 1;
  }
  //-------------------------------------------------------
  else if (msg == "off")//Set blanket to OFF
  {
    if(blanketState == 1)//only turn blanket OFF if it's already ON
    {
    pinMode(PowerSwitch, OUTPUT);
    digitalWrite(PowerSwitch, LOW);
    delay(100);
    digitalWrite(PowerSwitch, HIGH);
    pinMode(PowerSwitch, INPUT);
    }

    blanketState = 0;
    return 1;
  }
  //-------------------------------------------------------
  else if (msg == "on")//Set blanket to ON
  {
    if(blanketState == 0)//only turn blanket ON if it's already OFF
    {
    pinMode(PowerSwitch, OUTPUT);
    digitalWrite(PowerSwitch, LOW);
    delay(100);
    digitalWrite(PowerSwitch, HIGH);
    pinMode(PowerSwitch, INPUT);
    }

    blanketState = 1;
    return 1;
  }
  //-------------------------------------------------------
    else if (msg == "refresh")//Reload browser
  {
    //do nothing but refresh
    return 1;
  }
  //-------------------------------------------------------
  else if (msg == "reset")// Reset variables if blanket and Arduino become out of Sync
  {
      tempState = 1;//Med
      tempCount = 5;//Med
      return 1;
  }
  else
  return -1;
}//end checkAction()
