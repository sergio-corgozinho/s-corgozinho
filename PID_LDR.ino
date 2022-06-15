#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inicializa o LCD
LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);

double P, I = 0, pi = 0;
double erro;
double pot;
double ldr;
double last, delta;
double led = 9;
double fotoresistor = A0;
double potenciometro = A1;


void setup() {
  Serial.begin(9600);
  pinMode(fotoresistor, INPUT);
  pinMode(potenciometro, INPUT);
  pinMode(led, OUTPUT);

  //Inicializa LCD   
  lcd.begin (16,2);
  
}

void loop() {
  erro = 0;
  pot = 0;
  ldr = 0;

  for (int ii = 0; ii < 50; ii++) {
    pot += (double) (analogRead(potenciometro) >> 2)/50;
    ldr += (double) (analogRead(fotoresistor) >> 2) /50;
    erro += (pot - ldr) / 50.0;
  }

  delta = (millis() - last) / 1000;
  last = millis();
  P += (erro * 0.9) ;
  I += ((erro * 15) * delta );
  pi = P + I ;


  pi = pi < 0 ? 0 : pi; // se pi<0 ele recebe 0
  pi = pi > 255 ? 255 : pi; // se pi>255 ele recebe 255

  analogWrite(led, pi);

  Serial.print(pot);
  Serial.print(" ");
  Serial.print(ldr);
  Serial.print(" ");
//  Serial.print(pi);
 // Serial.print(" ");
  Serial.println(erro);
  //Serial.print(" ");
  //Serial.println(I);
  lcd.setCursor(0,0);
  lcd.print("POT: ");  
  lcd.print(pot);  
  lcd.setCursor(0,1);
  lcd.print("LDR: ");  
  lcd.print(ldr);  

}

