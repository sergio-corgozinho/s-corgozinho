#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

// ------------------------------------------------- Definições Gerais -----------------------------------------------------\\
#define Copo_1 7
#define Copo_2 9
uint8_t Tamanho_Copo;
String String_Tamanho_Copo;
#define Bomba 8
#define buzzer 4
char opcao;
int voltar_Menu = 0;
float tempo_Bomba_High = 0;
int espera_copo;


// ------------------------------------------------- Sensor de Temperatura -----------------------------------------------------\\

// Porta do pino de sinal do DS18B20
#define ONE_WIRE_BUS 5
 
// Define uma instancia do oneWire para comunicacao com o sensor
OneWire oneWire(ONE_WIRE_BUS);
  
DallasTemperature sensors(&oneWire);
DeviceAddress sensor1;
  
// Armazena temperaturas minima e maxima
float tempC;
float tempMin = 999;
float tempMax = 0;

 
// Inicializa o LCD
LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);

// ------------------------------------------------- Sensor Óptico reflexivo -----------------------------------------------------\\

#define A_Optical_pin_1 A0   // Pino analógico do sensor 1 
#define A_Optical_pin_2 A1   // Pino analógico do sensor 2
#define D_Optical_pin_1 12
#define D_Optical_pin_2 13
long Optical_reading_1;
long Optical_reading_2;
  
// --------------------------------------------------- Sensor Ultrassônico -------------------------------------------------------\\

#define Trig_pin 3          // Envia sinal ultrassônico
#define Echo_pin 2          // Recebe sinal ultrassônico
#define Trig2_pin 7          // Envia sinal ultrassônico
#define Echo2_pin 6          // Recebe sinal ultrassônico
#define Time_out 3000
long Duration_ultrasonic;   // Tempo em microsegundos 
float Distance_ultrasonic;  // Distância em cm
  
void Dur_ultrasonic(int Trig, int Echo);      // Protótipo de função
void Dist_ultrasonic();     // Protótipo de função

// --------------------------------------------------------------------------------------------------------------------------------\\


void setup() {
  
  Serial.begin(9600);

  //Relé Bomba 
  pinMode(Bomba, OUTPUT);
  digitalWrite(Bomba, HIGH);

  //  Sensor Ultrassônico
  pinMode(Trig_pin, OUTPUT);
  pinMode(Echo_pin,INPUT);
  pinMode(Trig2_pin, OUTPUT);
  pinMode(Echo2_pin,INPUT);
  
  //  Sensor Óptico
  pinMode(D_Optical_pin_1, OUTPUT);
  pinMode(D_Optical_pin_2, OUTPUT);
  
  // Buzzer
  pinMode(buzzer, OUTPUT);
  
  //Inicializa LCD   
  lcd.begin (16,2);
  
  // Localiza e mostra enderecos dos sensores
  sensors.begin();
  lcd.setCursor(0,0);
  lcd.print(sensors.getDeviceCount(), DEC);
  lcd.println(" sensor DS18B20");
  lcd.setCursor(0,1);
  lcd.print("   localizado   ");
  if (!sensors.getAddress(sensor1, 0)){ 
      lcd.setCursor(0,0);
      lcd.print("Sensor nao "); 
      lcd.setCursor(0,1);
      lcd.print("encontrado"); 
  }
  delay(2000);
  
}

//----------------------------------------------------------------------------------------------------------------------------\\

void loop() {
 
    opcao = '1';
    
    // Opção 1: Enchimento automático do copo
    while(opcao == '1')
    {
        
       // Le a informacao do sensor
        sensors.requestTemperatures();
        tempC = sensors.getTempC(sensor1);
        // Atualiza temperaturas minima e maxima
        if (tempC < tempMin)
        {
          tempMin = tempC;
        }
        if (tempC > tempMax)
        {
          tempMax = tempC;
        }
        
        
        //Testa mostrar temperaura
        Dur_ultrasonic(Trig_pin, Echo_pin);
        Dist_ultrasonic();
        Serial.print("SU: ");
        Serial.println(Distance_ultrasonic);
        if(Distance_ultrasonic <= 6)
        {
          opcao = '2';
        }
        
       
        
        // Verifica Nivel do Reservatorio
        Dur_ultrasonic(Trig2_pin, Echo2_pin);
        Dist_ultrasonic();
        Serial.println(Distance_ultrasonic);
        
        //Aciona Buzzer
        if(Distance_ultrasonic > 7.3)
        {
          digitalWrite(buzzer, HIGH);
          delay(600);
          digitalWrite(buzzer, LOW);
        }
        else
        {
          digitalWrite(buzzer, LOW);
        }
          
        
        //Leitura dos sensores ópticos
        lcd.clear();
        lcd.print(" Coloque o copo");
        Optical_reading_1 = analogRead(A_Optical_pin_1);
        delay(300);
        Optical_reading_2 = analogRead(A_Optical_pin_2);
       
        //Percebe a aproximação do copo
        if(Optical_reading_1 <= 150)
        {
          //Define tamanho do copo
          delay(100);
          if(Optical_reading_2 <= 150)
          {
            Tamanho_Copo = Copo_2;
            String_Tamanho_Copo = "Grande";
          }
          else
          {
            Tamanho_Copo = 7;
            String_Tamanho_Copo = "Pequeno";
          }
          lcd.clear();
          lcd.print("Copo: ");
          lcd.print(String_Tamanho_Copo);
          
         
          //Leitura do sensor ultrassônico
          Dur_ultrasonic(Trig_pin, Echo_pin);
          Dist_ultrasonic();
          lcd.clear();
          lcd.print("SU: ");
          lcd.print(Distance_ultrasonic);
          Serial.println(Distance_ultrasonic);
          //Enche copo
          if( (Distance_ultrasonic > 25 - Tamanho_Copo) )  //&& (Distance_ultrasonic >= 24.3))
          {
            digitalWrite(Bomba, LOW);
      
            do
            {
              Dur_ultrasonic(Trig_pin, Echo_pin);
              Dist_ultrasonic();
              Optical_reading_1 = analogRead(A_Optical_pin_1);
              lcd.clear();
              lcd.print(" Enchendo Copo ");
              lcd.setCursor(0,1);
              lcd.print(String_Tamanho_Copo);
              lcd.print("  SU:");
              lcd.print(Distance_ultrasonic);
              
              delay(100);
            }while((Distance_ultrasonic > 25 - Tamanho_Copo) && (Optical_reading_1 < 150));
            
            digitalWrite(Bomba, HIGH);
            
            lcd.clear();
            lcd.print("   Copo cheio   ");  
            lcd.setCursor(0,1);
            lcd.print("    Retire-o    ");
            
            while(Optical_reading_1<150)
              Optical_reading_1 = analogRead(A_Optical_pin_1);
            delay(1000);
            
         } //Fim do If enche copo
         
        } // Fim do If percebe aproximação do copo
        
      } // Fim do While Opção 1
  
    
//----------------------------------------------------------------------------------------------------------------------------\\
    
    
    //Mostrar temperatura
    while(opcao == '2')
    {
        
        // Le a informacao do sensor
        sensors.requestTemperatures();
        tempC = sensors.getTempC(sensor1);
        // Atualiza temperaturas minima e maxima
        if (tempC < tempMin)
        {
          tempMin = tempC;
        }
        if (tempC > tempMax)
        {
          tempMax = tempC;
        }
        
        // Mostra temperatura no LCD    
        lcd.clear();
        lcd.print("Temp.:       ");
        //Simbolo grau
        lcd.write(223);
        lcd.print("C");
        lcd.setCursor(7,0);
        lcd.print(tempC);
        lcd.setCursor(0,1);
        lcd.print("L: ");
        lcd.setCursor(3,1);
        lcd.print(tempMin,1);
        lcd.setCursor(8,1);
        lcd.print("H: ");
        lcd.setCursor(11,1);
        lcd.print(tempMax,1);
      
        Serial.print(tempC);
        Serial.print('\n');
        delay(200);
        Serial.print(tempMin);
        Serial.print('\n');
        delay(200);
        Serial.print(tempMax);
        Serial.print('\n');
        
        //Testa voltar opcao 1
        Dur_ultrasonic(Trig_pin, Echo_pin);
        Dist_ultrasonic();  
        if(Distance_ultrasonic <= 6)
        {
          opcao = '1';
        }
    }
    
//----------------------------------------------------------------------------------------------------------------------------\\

    //Calibrar sensores
    while(opcao == '3')
    {
      Calibracao_de_Sensores();
    }
    
//----------------------------------------------------------------------------------------------------------------------------\\
  
 // } // Fim do If (Serial Available)
  
} // Fim do void LOOP


//----------------------------------------------------------------------------------------------------------------------------\\

void Dur_ultrasonic(int Trig, int Echo)  // Sensor ultrassônico - Calcula a duração em microsegundos
{
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  Duration_ultrasonic = pulseIn(Echo, HIGH);
  if(Duration_ultrasonic == 0)
  {
    Duration_ultrasonic = Time_out;
  }
}


void Dist_ultrasonic() // Sensor ultrassônico - Calcula a distância em cm
{
  Distance_ultrasonic = Duration_ultrasonic/58.0;
  delay(100);
}


void mostra_endereco_sensor(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // Adiciona zeros se necessário
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void Calibracao_de_Sensores()
{
  
        //Testa voltar ao menu
        Dur_ultrasonic(Trig_pin, Echo_pin);
        Dist_ultrasonic();  
        if(Distance_ultrasonic <= 4)
        {
          opcao = '0';
        }
        
        Serial.println();
        Serial.println();
        Optical_reading_1 = analogRead(A_Optical_pin_1);
        Serial.print("Distância no sensor Optico 1: ");
        Serial.println(Optical_reading_1);
        Optical_reading_2 = analogRead(A_Optical_pin_2);
        Serial.print("Distância no sensor Optico 2: ");
        Serial.println(Optical_reading_2);
        Dur_ultrasonic(Trig_pin, Echo_pin);
        Dist_ultrasonic();
        Serial.print("Distância no sensor ultrassonico: ");
        Serial.println(Distance_ultrasonic);
        Dur_ultrasonic(Trig2_pin, Echo2_pin);
        Dist_ultrasonic();
        Serial.print("Distância no sensor ultrassonico: ");
        Serial.println(Distance_ultrasonic);
        delay(2000);
}
