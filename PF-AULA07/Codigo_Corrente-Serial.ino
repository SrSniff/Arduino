/*
 * Programa: Leitura de Tensão Analógica para medir corrente
 * Placa: Arduino Uno R3
 * Pino: A0 (entrada analógica)
 * 
 * Este programa lê continuamente o valor de tensão no pino A0
 * e mostra o resultado no Monitor Serial (em A)
 */

// A função setup() executa uma vez quando o Arduino é ligado
void setup() 
  {
  int valorAnalogico = 0;
  float xCorrente = 0.0;
  // Inicializa a comunicação serial com velocidade de 9600 baud
  // 9600 é a taxa de transmissão de dados por segundo
  Serial.begin(9600);

  // Pequeno delay para estabilizar a comunicação serial
  delay(500);
  }

// A função loop() executa repetidamente enquanto o Arduino estiver ligado
void loop() {
  // Variável para armazenar o valor lido do pino analógico
  // O valor analógico varia de 0 a 1023 (10 bits de resolução)
  valorAnalogico = analogRead(A0);
  
  // Converte o valor analógico (0-1023) para tensão (0-5V)
  // Fórmula: tensão = (valorAnalogico * 5.0) / 1023.0
  xCorrente = (valorAnalogico * 5.0) / 1023.0;
  
  // Exibe os valores no Monitor Serial
  Serial.print("Valor digital (0-1023): ");
  Serial.print(valorAnalogico);
  Serial.print("  |  Corrente (A): ");
  Serial.println(xCorrente, 2);  // 2 = número de casas decimais
  
  // Aguarda 500 milissegundos (0,5 segundo) antes da próxima leitura
  // Isso evita que o Serial fique muito rápido e difícil de ler
  delay(500);
}