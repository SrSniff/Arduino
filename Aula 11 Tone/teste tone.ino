//  Entrada Analógica (A0): O Arduino lê o potenciômetro e transforma a tensão física em um número digital entre 0 e 1023
//  Saída de Áudio (Pino 5): Usaremos a função nativa tone(). Ela gera uma onda quadrada na frequência que escolhermos (em Hertz). Como a audição humana e a maioria dos buzzers funcionam bem entre 20Hz e 20.000Hz, precisamos converter aquela leitura do potenciômetro para essa faixa de som. Mas o Arduino pode gerar de 31Hz até 
// 
// Projeto: Controle de Frequência de Buzzer via Potenciômetro
// IDE: Arduino IDE 2.x

// --- Definições de Pinos ---
const int pinoPotenciometro = A0;  // Pino analógico onde o potenciômetro está conectado
const int pinoBuzzer = 5;          // Pino digital onde o buzzer está conectado
int leituraLeve = 0;               // Leitura do potenciometro  
int frequencia  = 0;               // Calculo da frequencia 

void setup() 
{
  // Configura o pino do buzzer como SAÍDA
  pinMode(pinoBuzzer, OUTPUT);
}

void loop() 
{
  // 1. Lê o valor analógico do potenciômetro (Retorna um valor entre 0 e 1023)
  leituraLeve = analogRead(pinoPotenciometro);

  // 2. Mapeia o valor lido para uma faixa de frequência audível (20Hz a 2.000Hz)
  // Sintaxe da função map: map(valorAtual, minAtual, maxAtual, minNovo, maxNovo)
  frequencia = map(leituraLeve, 0, 1023, 20, 2000);

  // 3. Envia a frequência gerada para o buzzer no pino 5
  tone(pinoBuzzer, frequencia);

  // Um pequeno atraso de 10 milissegundos para estabilizar as leituras
  delay(10);
}