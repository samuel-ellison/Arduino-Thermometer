#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int analogPin = 0;
float Vout = 0;
float R1 = 10000;
float R2 = 0;
float logR2 = 0;
float Told = 0;
float Tf = 0;
float c1 = 0.000896051217230039, c2 = 0.000257883695711852, c3 = 1.06023156926913e-07;
float Beta = 3472.99959338122;
float Tref = 297.65;
float Rref = 10197.24;
float Tbeta = 0;
float Temps [5] = { 5, 4, 3, 2, 1 };
float T0,T1,T2,T3,T4 = 0;
int p = 4;
float Tpb,Tpc,TP;
float term1,term2,term3,term4 = 0;
float h;
bool r = 1;

int on = 0;
int off = 0;
int buttonPin1 = 7;
int buttonPin2 = 6;
int ledPin = 8;
int buttonStatus1 = 0;
int buttonStatus2 = 0;
int eqLight = 13;

bool rateSet = 0;
float sRate = 0;
long lims = 80;
float t1 = 0;
float t2 = 0;
float t11 = 0;
float t22 = 0;
float corr = 48;

float TL = 25;
float TH = 100;

void setup()
{
  lcd.begin(16, 2);
  analogReference(EXTERNAL);
  pinMode(eqLight, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  Serial.begin(9600);
  Serial.println("Enter Device Sampling Rate (Hz) above, Press ENTER");
}

void loop()
{
  if (r)
  {
  if (Serial.available()>0)
  {
    // set sampling rate
    sRate = Serial.parseFloat();
    if (sRate > lims || sRate <= 0)
    {
      Serial.println("Error: Sampling Rate must be positive and below 80 Hz. Please reupload and try again.");
    }
    else
    {
      Serial.print("Sampling Rate set to ");
      Serial.print(sRate);
      Serial.println(" Hz. Press START button to begin sampling.");
      rateSet = 1;
    }
  }
  if (rateSet)
  {
    // read outputs from VD and buttons
    Vout = analogRead(analogPin);
    buttonStatus1 = digitalRead(buttonPin1);
    buttonStatus2 = digitalRead(buttonPin2);
    if (Vout) //its plugged in
    {
      // determine button status
      if (buttonStatus1 == HIGH && buttonStatus2 == LOW) //on button on, off button off
      {
        on = 1;
        off = 0;
        digitalWrite(ledPin,HIGH);
      }
      
      if (buttonStatus1 == LOW && buttonStatus2 == HIGH)//on button off, off button on
      {
        on = 0;
        off = 1;
        digitalWrite(ledPin,LOW);
      }

      // start measurements
      if (on == 1 && off == 0)
      {
        // start timers
        if (p == 4)
        {
          t11 = micros();
        }
        t1 = micros();

        // thermistor resistance calculation
        R2 = R1*((1024/Vout)-1);

        // Steinhart-Hart model to calculate temperature
        logR2 = log(R2);
        Told = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
        // Beta model to calculate temperature
        Tbeta = 1/((1/Tref)+(1/Beta)*log(R2/Rref));
        // Celsius Conversions
        Tbeta = Tbeta - 273.15;
        Told = Told - 273.15;

        // store last 5 temperatures
        if (p > -1)
        {
          // assign new temp
          Temps[p] = Told;
        }
        else
        {
          // create array of temperatures
          T0 = Temps[0];
          T1 = Temps[1];
          T2 = Temps[2];
          T3 = Temps[3];
          T4 = Temps[4];
          // shift one down
          Temps[0] = Told;
          Temps[1] = T0;
          Temps[2] = T1;
          Temps[3] = T2;
          Temps[4] = T3;

          // calculate derivative
          h = 1/sRate;
          // Backward Difference
          Tpb = (Told-T0)/h;
          // Central Difference
          Tpc = (Told-T1)/(2*h);
          // 5-point stencil
          TP = (-Told+8*T0-8*T2+T3)/(12*h);
          //Serial.print("Derivative = ");
          //Serial.print(TP);
          //Serial.println(" C/s");

          // Stead-State conditions
          t22 = micros();
          if ((((t22-t11)/1000000) > 5) && (abs(TP) <= 0.001))
          {
            digitalWrite(eqLight,HIGH);
            //uncomment r = 0 to stop sampling when SS reached
            //r = 0;
          }
          else
          {
            digitalWrite(eqLight,LOW);
          }
        }

        // S-H Fahrenheit conversion
        Tf = (Told*9/5)+32;
        
        // print on LCD
        lcd.setCursor(0, 1);
        lcd.print("Temp = ");
        lcd.print(Told);
        lcd.setCursor(12, 1);  
        lcd.print(" C");
        lcd.setCursor(0, 0);
        lcd.print("Temp = ");
        lcd.print(Tf);
        lcd.print(" F");
        //lcd.setCursor(12, 0);  
        //lcd.print(" Ohm");

        // fix sampling rate
        
        while(true)
        {
          t2 = micros();
          if ((1000000/(t2-t1)) < (sRate))
          {
            break;
          }
        }
        
        p = p-1;

        // print S-H, Beta, and Resistance in Serial Monitor
        Serial.print("R = ");
        Serial.println(R2);
        Serial.print("T (SH) = ");
        Serial.println(Told);
        Serial.print("T (B) = ");
        Serial.println(Tbeta);

        // sampling rate limit
        /*
        if (p == 4)
        {
          t22 = micros();
          Serial.println(t22-t11);
        }
        */
        
      }
    }
  }
  }
}
