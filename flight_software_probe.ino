#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Servo.h>
#include <Adafruit_GPS.h> 
#include <SoftwareSerial.h>
#include <EEPROM.h> 

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11  
#define BMP_CS 10
#define degconvert 57.2957786

Servo myservo;
Servo myservo1;  

int addr,pos = 0, pos1=0,volti; 
int state = 0,sw=0;
int i=0, flag=0,t=0,count=0, checki=0;
float c_min,c_sec,b_min,b_sec=0;
float cmin,csec,m_min,m_sec,m_time=0;
int p,s;
float height[10];
char c;       
double clvl;
int v=0,y=0,u=0;
float frcast=1013.25;


Adafruit_BMP280 bmp(BMP_CS); 


const int MPU_addr=0x68;
double AcX,AcY,AcZ;
uint32_t timer; 


SoftwareSerial mySerial(3, 2); 
Adafruit_GPS GPS(&mySerial); 

void setup() {
  
  pinMode(A0,OUTPUT);  // buzzer
  pinMode(9,OUTPUT);   // buzzer+BMP gnd
  pinMode(8,OUTPUT);   // BMP+GPS VCC
  pinMode(A1,INPUT);   // Voltage sensor aka 'volti'
  digitalWrite(9, LOW);
  digitalWrite(8, HIGH);

  
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  myservo.attach(A2);
  myservo1.attach(A3);
  Wire.begin();
  #if ARDUINO >= 157
  Wire.setClock(400000UL); 
  #else
    TWBR = ((F_CPU / 400000UL) - 16) / 2; 
  #endif

  bmp.begin();
  GPS.begin(9600);         
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); 
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_2HZ);   
  delay(10);  
  
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);

  delay(100);


  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); 
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  
  AcX=Wire.read()<<8|Wire.read();       
  AcY=Wire.read()<<8|Wire.read();  
  AcZ=Wire.read()<<8|Wire.read();  


  double roll = atan2(AcY, AcZ)*degconvert;
  double pitch = atan2(-AcX, AcZ)*degconvert;
  double yaw = atan2(AcY, AcX)*degconvert;

  
}
void mtime()     //timer to maintain mission time
  {
     if(state!=0)
    {
      if(count==0)
      {
      b_min=c_min; //setting base time ---> time when probe starts ASCENDING
      b_sec=c_sec;
      }
      cmin=c_min;
      csec=c_sec;
      m_min=m_min+c_min-b_min;
      m_sec=m_sec+c_sec-b_sec;
      m_time=(m_min*60)+m_sec;
      
      if(state==-1)
      {
      Serial.print(";");
      Serial.print(m_time); //mission time in seconds
      }
      count++;
    }

  }


void loop() {

  EEPROM.write(addr, state);
  s = EEPROM.read(addr);
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  
  AcX=Wire.read()<<8|Wire.read();       
  AcY=Wire.read()<<8|Wire.read();  
  AcZ=Wire.read()<<8|Wire.read();  


  double roll = atan2(AcY, AcZ)*degconvert;
  double pitch = atan2(-AcX, AcZ)*degconvert;
  double yaw = atan2(AcY, AcX)*degconvert;

  if(checki==1)
  { 
    p=9;
    checki=0;
  }
  else
  p=i-1;         //for comparing previous value of altitude
  readGPS();
  level();
  mtime();      //mission time

  volti = analogRead(A1);
  
  if(s==-1)
  {
    Serial.print(";");
    Serial.print(bmp.readTemperature());
    Serial.print(";");
    
    Serial.print(bmp.readPressure());
 
    Serial.print(";");
    Serial.print(bmp.readAltitude(frcast));
    
  Serial.print(";");
  Serial.print(volti);
  Serial.print(";");
  
  Serial.print(roll);
  Serial.print(";");
  Serial.print(pitch);
  Serial.print(";");
  Serial.print(yaw);
  }
  if(i==9)
  {
  i=0;
  checki=1;  //  checki = check 'i' ---> variable to remove the loss of data after every 9 iterations 
  }
  else
  {
    i++;
  }
 
}
void readGPS(){  
  t=t+1;
  clearGPS();   
  while(!GPS.newNMEAreceived()) { 
  c=GPS.read(); 
  }
GPS.parse(GPS.lastNMEA());  
    
while(!GPS.newNMEAreceived()) {  
  c=GPS.read();
  }
GPS.parse(GPS.lastNMEA());

    if(s==-1)              // change 's' to 'state' if anything is fishy
    {
    Serial.print("\n");
    Serial.print(";:;");
    Serial.print(t); Serial.print(";");
    Serial.print(GPS.hour, DEC); Serial.print(";");
    Serial.print(GPS.minute, DEC); Serial.print(";");
    Serial.print(GPS.seconds, DEC); Serial.print(";");
    Serial.print(GPS.milliseconds);Serial.print(";");
    
    Serial.print((int)GPS.fix);Serial.print(";");
    Serial.print((int)GPS.fixquality);Serial.print(";"); 

      
      Serial.print(GPS.latitude, 4); 
      Serial.print(GPS.lat);
      Serial.print(";"); 
      Serial.print(GPS.longitude, 4); 
      Serial.print(GPS.lon);
      Serial.print(";");
      Serial.print(GPS.latitudeDegrees, 4);
      Serial.print(";"); 
      Serial.print(GPS.longitudeDegrees, 4);
      Serial.print(";");
      
      Serial.print(GPS.speed);
      Serial.print(";");
      Serial.print(GPS.angle);
      Serial.print(";");
      Serial.print(GPS.altitude);
      Serial.print(";");
      Serial.print((int)GPS.satellites);    
    
    }
      clvl = GPS.altitude-214.0;  //normalizing ground level to ZERO[Change according to venue's ground level altitude]
      height[i]= clvl;
      c_min=GPS.minute;     //current minutes
      c_sec=GPS.seconds;   //current seconds

}
void clearGPS() {  
while(!GPS.newNMEAreceived()) {
  c=GPS.read();
  }
GPS.parse(GPS.lastNMEA());
while(!GPS.newNMEAreceived()) {
  c=GPS.read();
  }
GPS.parse(GPS.lastNMEA());
 
}

void level() 
{
                                 //heat-shield detachment algorithm
  if(clvl >=300.0)               
  {
  v=1;
  y=0;
  }
  if(clvl<300.0 && clvl!=0)
  {
  y=1;
  }
  if(y==1 && v==1)
  {
  u = 1;
  }
  if(u==1)
  {
  pos=180;
  myservo.write(pos); //servo for the release mechanism
  digitalWrite(4,LOW);//Camera ON 
  delay(510);         
  digitalWrite(4,HIGH);
  }
  if(sw==1)  //open flaps when sw = 1, ie. when DESCENDING starts 
  {
  pos1=180;
  myservo1.write(pos1);   //servo for the flaps
  }
  if(clvl<2 && state==-1) //condition to check LANDING on ground
  {
  state=0;               // setting STATE = 0 after LANDING(IDLE)--->helps to set timer condition for mission time
  digitalWrite(4,LOW); //Camera OFF
  delay(510);
  digitalWrite(4,HIGH);
  tone(A0,1000,4000000); //BUZZER ON
  }
  if(i==0)
  {
  flag++;  //dummy variable for i=0 case since there is no previous altitude value
  }
  else
  {
  if(height[i]>height[p]) // setting STATE = 1 while ASCENDING
  {
    state=1;
  }
  if(height[i]<height[p]) // setting STATE = -1 while DESCENDING
  {
    sw++;        //switch goes to 1 for the first time when it starts DESCENDING, and increments in later loops
    state=-1;
  }
  }

}

