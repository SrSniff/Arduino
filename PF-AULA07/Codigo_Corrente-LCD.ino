/*
 * Programa: Leitor de Corrente com Display LCD 16x2 I2C
 * Placa: Arduino Uno R3
 * Pino analógico: A0
 * Display I2C: SDA -> A4, SCL -> A5
 * Canal Eletronica e Tecnologia
 * Este programa lê a tensão no pino A0 e exibe como Corrente no display LCD 16x2
 */

#include <Wire.h>              // Biblioteca para comunicação I2C
#include <LiquidCrystal_I2C.h> // Biblioteca para controle do LCD I2C

// Configuração do LCD I2C
// Endereço I2C comum: 0x27 ou 0x3F (teste qual funciona no seu módulo)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Endereço 0x27, 16 colunas, 2 linhas

// Pino onde será feita a leitura analógica
const int pinoSensor = A0;

// Variáveis para armazenar os valores
int valorAnalogico = 0;      // Valor digital de 0 a 1023
float xCorrente = 0.0;          // Tensão calculada em Volts

void setup() {
  // Inicializa o display LCD
  lcd.init();           // Inicializa o LCD
  lcd.backlight();      // Liga a luz de fundo do LCD
  lcd.clear();          // Limpa qualquer caractere residual
  
  // Exibe mensagem inicial no LCD
  lcd.setCursor(2, 0);        // Coluna 2, Linha 0 (primeira linha)
  lcd.print("Leitor de");     // Linha 1: "Leitor de"
  lcd.setCursor(0, 1);        // Coluna 0, Linha 1 (segunda linha)
  lcd.print("Corrente ");     // Linha 2: "Corrente"
  
  // Aguarda 2 segundos para o usuário ler a mensagem inicial
  delay(2000);
  
  // Limpa a tela para começar as leituras
  lcd.clear();
}

void loop() {
  // Passo 1: Ler o valor analógico do pino A0
  // O analogRead() retorna um valor entre 0 e 1023 (10 bits de resolução)
  valorAnalogico = analogRead(pinoSensor);
  
  // Passo 2: Converter o valor lido para tensão (0V a 5V)
  // Fórmula: tensão = (valorAnalogico * 5.0) / 1023.0
  xCorrente = (valorAnalogico * 5.0) / 1023.0;
  
  // Passo 3: Exibir a corente no display LCD
  
  // Posiciona o cursor na primeira linha, coluna 0
  lcd.setCursor(0, 0);
  lcd.print("Corrente: ");
  lcd.print(xCorrente, 2);      // Exibe com 2 casas decimais
  lcd.print(" A");           // Adiciona a unidade "A" (Ampères)
  
  // Passo 4: Aguardar um pequeno intervalo antes da próxima leitura
  // Atraso de 500ms (0,5 segundo) para não sobrecarregar o display
  delay(500);
}