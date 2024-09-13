#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Stepper.h>
// Pin ID
const int PingUlt=6; // Ultra sonic / PWM
const int EchoUlt=7; // Ultra sonic / PWM  // infrared / PWM
const int EchoSoil=8; // soil moisture / ANALOG
const int EchoServo=3; // Micro Servo / PWM
const int EchoBuz=4; // Buzzer / PWM
const int EchoInd=8; // inductive / PWM
// LCD SDA=20 / SCL=21 | UNO SDA=A4 / SCL=A5
// Settings
const int Ultrange=7; // -2 for drift
const int Soilrange=350; // wet value || 500-460 normal / <350 wet 
// Variable
int Route=0;
int WETT=0;
Stepper StepperW = Stepper(2048, 10, 12, 11, 13); // step Per Angle = 0.1758 || Angle/0.1758 = step
LiquidCrystal_I2C lcd(0x27,16,2);
Servo ServoW;

void reset() {
  lcd.init();
  lcd.backlight();
  lcd.print("[Reset Mode]");
  lcd.setCursor(0,1);
  lcd.print("Please Wait...");
  Serial.println("Wait a sec...");
  StepperW.setSpeed(10);
  StepperW.step(1); // << here
  Serial.println("Ok");
  lcd.setCursor(0,1);
  lcd.print("Done.         ");
  delay(100);
  exit(0);
}

void setup() {
  Serial.begin(9600);
  // For reset pos
//  reset();
  Serial.println("SETUP pin");
  pinMode(PingUlt,OUTPUT);
  pinMode(EchoUlt,INPUT);
  pinMode(EchoBuz, OUTPUT);
  pinMode(EchoInd, INPUT);
  Serial.println("SETUP lcd");
  lcd.init();
  lcd.backlight();
  Serial.println("SETUP Servo");
  ServoW.attach(EchoServo);
  ServoW.write(15); // reset set to 45
  Serial.println("SETUP Stepper");
  StepperW.setSpeed(10);
  // StepperW.step(-2048); // reset to default
  tone(EchoBuz,500,100);
  Serial.println("All Done");
}

int CheckCm() {
  digitalWrite(PingUlt,LOW);
  delayMicroseconds(2);
  digitalWrite(PingUlt,HIGH);
  delayMicroseconds(5);
  digitalWrite(PingUlt,LOW);
  return (pulseIn(EchoUlt,HIGH)/2)/29.1;
}

void loop() {
  // return;
  WETT=0;
  lcd.clear();
  lcd.print("Ready");
  Serial.println("Ready.");
  while (true) { // check if has trash
    delay(100);
    // For Debug
    Serial.print("Metal : ");
    Serial.print(digitalRead(EchoInd));
    // Serial.print(" Ultra sonic : ");
    // Serial.print(CheckCm());
    Serial.print(" Soil : ");
    Serial.print(analogRead(EchoSoil));
    Serial.println();
    // if (digitalRead(EchoIr)) StepperW.step(stepsPerRevolution);
    const int cmm=CheckCm();
    if (cmm==0) {
      Serial.println("ERROR (Check Ultra sonic wire)");
      continue;
    }
    const int WWW=(cmm <= Ultrange || cmm >= 50);
    WETT=(analogRead(EchoSoil) < Soilrange);
    Serial.print("Checking ");
    Serial.println(cmm);
    if (WWW==1||WETT==1) break;
  }
  lcd.clear();
  lcd.print("Please Wait...");
  tone(EchoBuz,1000,1000);
  delay(2000);
  // check trash type
  if (digitalRead(EchoInd)==0){ // metal
    Route=0; // 0
    Serial.println("Found metal");
  } else if (analogRead(EchoSoil) < Soilrange || WETT==1){ // wet waste
    Route=711; // 125
    Serial.println("Found wet");
  } else if (CheckCm()<3) { // less than 10 (Ult)
    Route=1900; // 245
    Serial.println("Found Dry");
  } else { // idk type route to ?
    Route=1900; // 245
    Serial.println("Found Unknown (Route to Dry)");
    // return;
  }
  Serial.print("Route To ");
  Serial.println(Route);
  // Route bin/bucket
  StepperW.step(Route);
  delay(250); // wait for bucket stable
  Serial.println("Open Bucket");
  ServoW.write(180); // open door
  delay(1500); // wait for trash drop in to bin/bucket
  ServoW.write(15); // close door
  Serial.println("Close Bucket");
  delay(100); // make sure bucket close
  // Route bin/bucket back to default
  Serial.println("Route Back...");
  
  Route=0;
  delay(250); // make sure everything is setup/ready.
  Serial.println();
  Serial.println();
}