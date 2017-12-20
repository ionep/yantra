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
    shade=digitalRead(pin);
  }

  boolean state()
  {
    if(shade==0)
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
  randomSeed(analogRead(emptyPin));

  //stop image processing at first
  Serial.print("S");

  //digitalWrite(A2,HIGH);
}

//instantiate the motors
motor mRight(motor1_2,motor1_1),mLeft(motor2_1,motor2_2);

//instantiate the ir modules
irModule irFL(ir0),irFR(ir1),irBL(ir2),irBR(ir3);

//maximum sequence state
unsigned int maxSequence=65000; //3 sec approx
int maxRotation;
int sequenceCounter=0;

//serial data
int ch;
char chr;
boolean imageProcess=false;
boolean serialFound=false;
boolean rotateLeft;

//custom state
boolean stateDefined=false;

//main switch state
int switchState;

//random number for sequence
int randNo;

boolean firstTime=true;
boolean straightForward;
void loop()
{ 
  //check if main switch is on
  switchState=digitalRead(switchPin);
  if(switchState==1)
  {
    if(firstTime)
    {
      delay(2000);
      firstTime=false;
    }
    //load ir module states
    irFL.check();
    irFR.check();
    irBL.check();
    irBR.check();
    
    //ir check
    if(irFL.state() || irFR.state() || irBL.state() || irBR.state())
    {
      //fill after fixing which ir module to use
      
      if ((irFL.state() && irFR.state() && irBL.state()  && irBR.state()) || (irFL.state() && irBR.state()) || (irFR.state() && irBL.state()) || 
      (irFL.state() && irFR.state() && (irBL.state() || irBR.state())) || (irBL.state() && irBR.state() && (irFL.state() || irFR.state())) ) //all ir shown means an error
      {
        mLeft.halt();
        mRight.halt();
      }
      else if (irFL.state() && irFR.state()) // border in front
      {
        mLeft.backward();
        mRight.backward();
      }
      else if (irBL.state() && irBR.state()) //border in back
      {
        mLeft.forward();
        mRight.forward();
      }
      else if (irFL.state() && irBL.state()) //border in left
      {
        mLeft.forward();
        mRight.backward();
      }
      else if (irFR.state() && irBR.state()) //border in right
      {
        mLeft.backward();
        mRight.forward();
      }
      else if (irFL.state()) 
      {
        mLeft.backward();
        mRight.halt();
      }
      else if (irFR.state())
      {
        mLeft.halt();
        mRight.backward();
      }
      else if (irBL.state())
      {
        mLeft.forward();
        mRight.halt();
      }
      else if (irBR.state())
      {
        mLeft.halt();
        mRight.forward();
      }
      else
      {
        mLeft.halt();
        mRight.halt();
      }

      if(imageProcess)
      {
        //stop image processing
        Serial.print("S");  
        imageProcess=false;
      }
      
      serialFound=false;
      sequenceCounter=0;
      stateDefined=false;
    }
    else
    {
      //serial check
      if(Serial.available()>0)
      {
        chr=Serial.read();
        ch=chr-'0';

        if(ch>=0 && ch<=9)
        {
          serialFound=true;
          sequenceCounter=0;
          if(ch<=4)
          {
            maxRotation=maxSequence/2-ch*maxSequence/10;
            rotateLeft=true;
          }
          else
          {
            maxRotation=maxSequence/2+(9-ch)*maxSequence/10;
            rotateLeft=false;
          }
          
          straightForward=true;
          
          if(imageProcess)
          {
            //stop image processing
            Serial.print("S");  
            imageProcess=false;
          }
        }
        //have to check if pi sends random data over serial
        else if(chr=='N')
        {
          serialFound=false;
          sequenceCounter=0;
          stateDefined=false;
        }
      }
      //serial sequence
      if(serialFound && sequenceCounter<=maxSequence)
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
          if(straightForward)
          {
            sequenceCounter=0;
            straightForward=false;
          }
          //send the bot to opponent
          mLeft.forward();
          mRight.forward();
        }
        
        sequenceCounter++;
      }
      //custom sequence
      else if(!serialFound && sequenceCounter<=maxSequence)
      {
        if(!imageProcess)
        {
          //start image processing
          Serial.print("G");
          imageProcess=true;
        }
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
        serialFound=false;
        sequenceCounter=0;
        stateDefined=false;
      }
    }
  }
  else
  {
    firstTime=true;
    //migh need to bring outside so that it always stops
    //stop all motors if main switch is not on
    // dont move out
    mLeft.halt();
    mRight.halt();
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

