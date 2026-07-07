/*
 * Controle de brilho de LED com potenciômetro
 * Arduino Uno R3
 * Canal eletrônica e Tecnologia 2026
 * O potenciômetro no pino A0 define o ciclo de trabalho do PWM
 * LED no pino 9 (pwm ~) 
 */

// Pinos que usaremos
const int pinoPot = A0;      // potenciômetro na entrada analógica 0
const int pinoLED = 9;       // LED no pino PWM 9

// Variáveis para armazenar os valores
int valorPot = 0;            // valor cru do potenciômetro (0 a 1023)
int valorPWM = 0;            // valor convertido para PWM (0 a 255)

void setup() 
{
  pinMode(pinoLED, OUTPUT);   // pino do LED como saída
}

void loop() 
{
  // PASSO 1: LER O POTENCIÔMETRO
  valorPot = analogRead(pinoPot);
  
  // PASSO 2: CONVERTER (mapear) para a faixa do PWM
  // analogRead retorna 0 a 1023
  // analogWrite espera 0 a 255
  valorPWM = map(valorPot, 0, 1023, 0, 255);
  
  // PASSO 3: APLICAR O PWM NO LED
  analogWrite(pinoLED, valorPWM);
  
  // Pequeno delay 
  delay(50);
}
