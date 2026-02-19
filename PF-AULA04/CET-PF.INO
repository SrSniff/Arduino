// Programação Fácil Arduino - Canal Eletrônica e Tecnologia 2026
// Variáveis
int x =0;    // variável que recebe o valor lido na porta A0
float xTensao = 0.0;  // variável que recebe o cálculo do lido na porta A0 e convrte para 5V
// --- Inclusão da Biblioteca LiquidCrystal ---
// Esta biblioteca é necessária para controlar displays LCD no modo paralelo.
#include <LiquidCrystal.h>
// --- Definições de Pinos para o LCD ---
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
void setup()
{
  lcd.begin(16, 2); //Inicializar com o tipo de LCD
  lcd.clear(); 		// Limpar tela
  pinMode(8, OUTPUT);  // O pino  8 é o ligado ao LED VERDE.
  pinMode(9, OUTPUT);  // O pino  9 é o ligado ao LED VERMELHO.
  digitalWrite( 8, HIGH);	// Inicia com o LED verde ligado  
  digitalWrite( 9, LOW);	// Inicia com o LED vermelho desligado
}

void loop()
{
  lcd.setCursor(0, 0); // Define o cursor para a coluna 0, linha 0
  lcd.print("Tensao:");
  lcd.setCursor(7, 0); // Move o cursor para onde a tensão será exibida
  lcd.print(xTensao, 2); // Exibe a tensão real com 2 casas decimais
  delay(333);   		// Uma pausa para ler apenas 3x por segundo.
  x=analogRead(A0);		// Le a tensão do pino A0, sendo de 0 a 1023 (0 a 5V)
  xTensao = (5.0 / 1023.0) * (float)x; // Converte o número lido (0-1023) para 5V
  xTensao = xTensao * 10; // Multiplica o resoltado por 10 para voltar a ser de 0 a 50V
  digitalWrite( 9, LOW); // Apaga o LED vermelho
  if (xTensao > 40.0)    // Pergunta se a tensão (xTensao) é maior que 40V
  {
    digitalWrite( 9, HIGH);  // Acende o LED vermelho se a tensão for maior que 40V (condição)
  }
}