//define some variable as prepocessors
#define motor1_1 13
#define motor1_2 12
#define motor2_1 11
#define motor2_2 10

#define ir0 A0
#define ir1 A1
#define ir2 A2
#define ir3 A3

#define emptyPin 0

#define switchPin 9

//get the rotation to face our bot with opponent
inline int getMaxRotation(int);

//get a random sequence
inline void haveFun(int);

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
  }

  boolean state()
  {
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


void setup()
{
  Serial.begin(9600);
  
  //pin for motors
  pinMode(motor1_1,OUTPUT);
  pinMode(motor1_2,OUTPUT);
  pinMode(motor2_1,OUTPUT);
  pinMode(motor2_2,OUTPUT);
  
  //pin for ir
  pinMode(ir0,INPUT);
  pinMode(ir1,INPUT);
  pinMode(ir2,INPUT);
  pinMode(ir3,INPUT);
  
  //pin for switch
  pinMode(switchPin,INPUT);

  //get noise for randomization
  randomSeed(analogRead(0));

  //stop image processing at first
  Serial.print("S");
}

//instantiate the motors
motor mLeft(motor1_1,motor1_2),mRight(motor2_1,motor2_2);

//instantiate the ir modules
irModule irFL(ir0),irFR(ir1),irBL(ir2),irBR(ir3);

//maximum sequence state
int maxSequence=100;
int maxRotation;
int sequenceCounter=0;

//serial data
int ch;
boolean imageProcess=false;
boolean serialFound=false;
boolean rotateLeft;

//custom state
boolean stateDefined=false;

//main switch state
int switchState;

//random number for sequence
int randNo;

void loop()
{ 
  //check if main switch is on
  switchState=digitalRead(switchPin);
  if(switchState==1)
  {
    //load ir module states
    irFL.check();
    irFR.check();
    irBL.check();
    irBR.check();
    
    //ir check
    if(irFL.state() || irFR.state() || irBL.state() || irBR.state())
    {
      //fill after fixing which ir module to use

      serialFound=false;
      sequenceCounter=0;
      stateDefined=false;
    }
    else
    {
      //serial check
      if(Serial.available()>0)
      {
        ch=Serial.read()-'0';

        if(ch>=0 && ch<=9)
        {
          serialFound=true;
          sequenceCounter=0;
          maxRotation=getMaxRotation(ch);
          if(ch<=4)
          {
            rotateLeft=true;
          }
          else
          {
            rotateLeft=false;
          }
        }
        else
        {
          serialFound=false;
          sequenceCounter=0;
          stateDefined=false;
        }
        if(imageProcess)
        {
          //stop image processing
          Serial.print("S");  
          imageProcess=false;
        }
        
      }
      //serial sequence
      else if(serialFound && sequenceCounter<=maxSequence)
      {
        //check to rotate to perfectly position the bot
        if(sequenceCounter<=maxRotation)
        {
          if(rotateLeft)
          {
            mLeft.backward();
            mRight.forward();
          }
          else
          {
            mLeft.forward();
            mRight.backward();
          }
        }
        //if positioned
        else
        {
          //send the bot to opponent
          mLeft.forward();
          mRight.forward();
        }
        
        sequenceCounter++;
      }
      //custom sequence
      else if(!serialFound && sequenceCounter<=maxSequence)
      {
        if(!stateDefined)
        {
          randNo=random(0,5); // get random number from 0 to 4
          stateDefined=true;
        }
        haveFun(randNo);
        sequenceCounter++;
      }
      //if counter sequence ends
      else
      {
        if(!imageProcess)
        {
          //start image processing
          Serial.print("G");
          imageProcess=true;
        }
      }
    }
  }
  else
  {
    //migh need to bring outside so that it always stops
    //stop all motors if main switch is not on
    mLeft.halt();
    mRight.halt();
  }
}

inline int getMaxRotation(int ch)
{
  switch(ch)
  {
    case 0:
      return;
    case 1:
      return;
    case 2:
      return; 
    case 3:
      return;
    case 4:
      return;
    case 5:
      return;
    case 6:
      return;
    case 7:
      return;
    case 8:
      return;
    case 9:
      return;
    default:
      return 0;
  }
}

inline void haveFun(int r)
{
  switch(r)
  {
    case 0: //forward
      mLeft.forward();
      mRight.forward();
      break;
    case 1: //backward
      mLeft.backward();
      mRight.backward();
      break;
    case 2: //right
      mLeft.forward();
      mRight.backward();
      break;
    case 3: //left
      mLeft.backward();
      mRight.forward();
      break;
    case 4: //stop
      mLeft.halt();
      mRight.halt();
      break;
    default:
      mLeft.halt();
      mRight.halt();
      break;
  }
}

