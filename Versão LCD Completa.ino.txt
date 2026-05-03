#include <Wire.h> 
 // A biblioteca LiquidCrystal_I2C requer a biblioteca Wire para I2C
#include <LiquidCrystal_I2C.h>

// --- Configuração do Display LCD ---
// Endereço I2C do display (geralmente 0x27 ou 0x3F)
// Endereço, Número de Colunas (16), Número de Linhas (2)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// --- Configuração dos Pinos ---
const int pinoVin = A0;   // Canal A0: Tensão de entrada (Referência do divisor)
const int pinoVout = A1;  // Canal A1: Tensão no centro do divisor

// --- Valores Conhecidos ---
// Resistor 1 será o de 1.000 Ohms (1k)
const float R1 = 1000.0; 

// Topologia do Circuito Assumida:
// Vin (A0) ---> [ Resistor 1k (R1) ] ---> Ponto Central (A1) ---> [ Resistor Desconhecido (R2) ] ---> Terra (GND)

void setup() {
  Serial.begin(9600); // Inicia comunicação serial (para monitor do IDE do Arduino)
  
  // Inicialização do visor LCD 16x2
  lcd.init();
  lcd.backlight(); // Liga a retroiluminação da tela
  
  lcd.setCursor(0, 0);
  lcd.print("Ohmimetro Serie");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Efetua as leituras analógicas (ADC retorna valor entre 0 e 1023)
  int leituraA0 = analogRead(pinoVin);
  int leituraA1 = analogRead(pinoVout);

  // Sistema de Segurança 1: Tensão de entrada muita baixa
  // Evita divisão por zero ou ruídos
  if (leituraA0 < 20) {
    lcd.setCursor(0, 0);
    lcd.print("Aguardando Vin..");
    lcd.setCursor(0, 1);
    lcd.print("                ");
    delay(500);
    return;
  }

  // Sistema de Segurança 2: Circuito sem resistor 2 ou aberto.
  // A queda de tensão sobre R1 seria zero, logo Vout = Vin
  if (leituraA1 >= leituraA0) {
    lcd.setCursor(0, 0);
    lcd.print("Circuito Aberto ");
    lcd.setCursor(0, 1);
    lcd.print("Ou Rx > Infinito");
    delay(500);
    return;
  }

  // --- MATEMÁTICA DO CÁLCULO ---
  // A equação base de um divisor de tensão é: Vout = Vin * (R2 / (R1 + R2))
  // Isolando R2: R2 = R1 * (Vout / (Vin - Vout))
  // Como Leituras dos ADCs são proporcionais à tensão num mesmo instante,
  // descartamos a necessidade de conversão prévia, usamos os valores crus:
  float relacao = (float)leituraA1 / (float)(leituraA0 - leituraA1);
  float R2 = R1 * relacao;

  // --- APRESENTAÇÃO NA TELA ---
  lcd.setCursor(0, 0);
  lcd.print("Rx: ");
  // Conversão visual entre ohms e kohms para facilitar leitura
  if (R2 >= 1000.0) {
    lcd.print(R2 / 1000.0, 2); // Exibe com duas casas decimais
    lcd.print(" kohm  ");
  } else {
    lcd.print(R2, 1);          // Exibe com uma casa decimal
    lcd.print(" ohm   ");
  }

  // Medidor de Tensão (Considerando referência VCC como 5V do Arduino)
  float tensaoIn = leituraA0 * (5.0 / 1023.0);
  float tensaoOut = leituraA1 * (5.0 / 1023.0);
  
  // Apresenta Vin e Vout medidos na Segunda Linha do LCD
  lcd.setCursor(0, 1);
  lcd.print("In:");
  lcd.print(tensaoIn, 1);      // Limitado a 1 casa para economizar espaço
  lcd.print("V Vd:");
  lcd.print(tensaoOut, 1);
  lcd.print("V ");

  // Também enviamos para a interface serial do IDE
  Serial.print("ADC In: "); Serial.print(leituraA0);
  Serial.print("\tADC Out: "); Serial.print(leituraA1);
  Serial.print("\tResistência: "); Serial.println(R2);

  delay(600); // 600ms de atualização evita que o display pisque exageradamente
}