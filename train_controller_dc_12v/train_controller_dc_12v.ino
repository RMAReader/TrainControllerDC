/*  Arduino DC Motor Control - PWM | H-Bridge | L298N  -  Example 01

    by Dejan Nedelkovski, www.HowToMechatronics.com
*/
#include "IRremote.h"

#define receiver 11 // Signal Pin of IR receiver to Arduino Digital Pin 11
#define enA 9
#define in1 6
#define in2 7

IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

class SmoothedValue
{
public:
  SmoothedValue(float accel)
  {
    this->timeSet = 0;
    this->accel = accel;
    this->value = 0;
  }

  void forceValue(float target)
  {
    this->timeSet = 0;
    this->value = target;
  }

  void setValue(float target)
  {
    this->start_value = this->value;
    this->target_value = target;
    timeSet = millis();
  }

  float getValue()
  {
    Update();
    
    return value;  
  }

private:

  void Update()
  {
    if(timeSet == 0)
      return;
      
    unsigned long period = millis() - timeSet;
    if(target_value >= value)
    {
      value = start_value + period * accel;
      if(value >= target_value)
      {
        value = target_value;
        timeSet = 0;
      }
    }
    else if(target_value <= value)
    {
      value = start_value - period * accel;
      if(value <= target_value)
      {
        value = target_value;
        timeSet = 0;
      }
    }
    Serial.print(period);
    Serial.print(" ");
    Serial.print(period * accel);
    Serial.print(" ");
    Serial.println(value);
  }

  unsigned long timeSet; 
  float accel;
  float value;
  float target_value;
  float start_value;
};

int direction = 0;
SmoothedValue power(0.002);
int powerMin = 0;
int powerMax = 9; // max enterable value is 9, setting this higher means max pwm is never achieved, so scales down 15v input to 12v

// pwmOutput must be in range 0 to 255
int pwmOutput = 0;
int pwmOutputMin = 64;
int pwmOutputMax = (255 * 10) / 15;  //scale down 15v input to 12v

void setPwmOutput(int power)
{
  pwmOutput = (power == 0) ? 0 : map(power, powerMin, powerMax, pwmOutputMin , pwmOutputMax); 
  analogWrite(enA, pwmOutput); // Send PWM signal to L298N Enable pin
}

void setDirection(int direction)
{
  if (direction == 0)
  {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }
  else
  {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }
}

bool readSerial()
{
  bool hasChanged = false;
  while (Serial.available() > 0)
  {
    char value = Serial.read();
    if(value == '+')
    {
      direction = 0;
      hasChanged = true;
    }
    else if(value == '-')
    {
      direction = 1;
      hasChanged = true;
    }
    else if('0' <= value && value <= '9' )
    {
      power.setValue(value - '0');
      hasChanged = true;
    }
  }
  return hasChanged;
}

bool readRemote()
{
  switch(results.value)
  {
    case 0xFFA25D: power.forceValue(0); return true;
    case 0xFF22DD: direction = 1; return true;
    case 0xFFC23D: direction = 0; return true;
    case 0xFF6897: power.setValue(0); return true;
    case 0xFF30CF: power.setValue(1); return true;
    case 0xFF18E7: power.setValue(2); return true;
    case 0xFF7A85: power.setValue(3); return true;
    case 0xFF10EF: power.setValue(4); return true;
    case 0xFF38C7: power.setValue(5); return true;
    case 0xFF5AA5: power.setValue(6); return true;
    case 0xFF42BD: power.setValue(7); return true;
    case 0xFF4AB5: power.setValue(8); return true;
    case 0xFF52AD: power.setValue(9); return true;
    
    default: 
      return false;

  }// End Case
}

void printState()
{
  Serial.print("power=");
  Serial.print(power.getValue());
  Serial.print(" direction=");
  Serial.print(direction==0 ? "forward" : "backward");
  Serial.print(" pwmOutput=");
  Serial.println(pwmOutput);
}

void setup() 
{
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  irrecv.enableIRIn(); // Start the receiver

  setPwmOutput(power.getValue());
  setDirection(direction);

  Serial.begin(9600); 
  Serial.println("Controller ready.");
  printState();
}

void loop() 
{
  bool hasChanged = readSerial();
  
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    hasChanged = readRemote() || hasChanged;
    irrecv.resume(); // receive the next value
  } 
  
  setPwmOutput(power.getValue());
  setDirection(direction);

  if(hasChanged)
  {
    printState();
  }
}
