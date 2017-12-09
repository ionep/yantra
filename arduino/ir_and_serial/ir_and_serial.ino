class motor
{
  private:
  int v,g;
  
  public:
  
  motor(int v,int g)
  {
    this->v=v;
    this->g=g;
    this->halt();
  }
  
  void forward()
  {
    digitalWrite(v,HIGH);
    digitalWrite(g,LOW);
  }
  
  void backward()
  {
    digitalWrite(v,LOW);
    digitalWrite(g,HIGH);
  }
  
  void halt()
  {
    digitalWrite(v,LOW);
    digitalWrite(g,LOW);
  }
  
};

class irModule
{
  private:
  int pin,shade;
  
  public:
  
  irModule(int pin)
  {
    this->pin=pin;
  }
  
  boolean check()
  {
    shade=analogRead(pin);
    
    if(shade>650)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  
};

//pin to check switching
int switchPin=9;

void setup()
{
  Serial.begin(9600);
  
  //pin for motors
  pinMode(13,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(10,OUTPUT);
  
  //pin for ir
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);
  pinMode(A2,INPUT);
  pinMode(A3,INPUT);
  
  //pin for switch
  pinMode(switchPin,INPUT);
  
}

//instantiate the motors
motor mleft(13,12),mright(11,10);

//instantiate the ir modules
irModule irFL(A0),irFR(A1),irBL(A2),irBR(A3);

//serial data
int ch;

//main switch state
int switchState;

void loop()
{ 
  //check if main switch is on
  switchState=digitalRead(switchPin);
  if(switchState==1)
  {
    //ir check
    if(irFL.check() || irFR.check() || irBL.check() || irBR.check())
    {
      //fill after fixing which ir module to use
    }
    else
    {
      //serial check
      if(Serial.available()>0)
      {
        ch=Serial.read()-'0';
      }
    }
  }
  else
  {
    //migh need to bring outside so that it always stops
    //stop all motors if main switch is not on
    mleft.halt();
    mright.halt();
  }
}
