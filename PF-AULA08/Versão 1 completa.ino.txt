// --- Configuração dos Pinos ---
const int pinoVin = A0;   // Canal A0: Tensão de entrada (Referência do divisor)
const int pinoVout = A1;  // Canal A1: Tensão no centro do divisor

// --- Valores Conhecidos ---
// Resistor 1 será o de 1.000 Ohms (1k)
const float R1 = 1000.0; 
float R2 = 1000.0; 
float R1 = 1000.0; 
float relacao = 0.0;
float tensaoIn = 0.0;
float tensaoOut = 0.0;
int leituraA0 = 0;
int leituraA1 = 0;


// Topologia do Circuito Assumida:
// Vin (A0) ---> [ Resistor 1k (R1) ] ---> Ponto Central (A1) ---> [ Resistor Desconhecido (R2) ] ---> Terra (GND)

void setup() 
{
  Serial.begin(9600); // Inicia comunicação serial (para monitor do IDE do Arduino)
}

void loop() 
{
  // Efetua as leituras analógicas (ADC retorna valor entre 0 e 1023)
  leituraA0 = analogRead(pinoVin);
  leituraA1 = analogRead(pinoVout);

  // Sistema de Segurança 1: Tensão de entrada muita baixa
  // Evita divisão por zero ou ruídos
  if (leituraA0 < 20) 
  {
    Serial.println("Aguardando Vin..");
    delay(500);
    return;
  }

  // Sistema de Segurança 2: Circuito sem resistor 2 ou aberto.
  // A queda de tensão sobre R1 seria zero, logo Vout = Vin
  if (leituraA1 >= leituraA0) 
  {
    Serial.println("Circuito Aberto ");
    delay(500);
    return;
  }

  // --- MATEMÁTICA DO CÁLCULO ---
  // A equação base de um divisor de tensão é: Vout = Vin * (R2 / (R1 + R2))
  // Isolando R2: R2 = R1 * (Vout / (Vin - Vout))
  // Como Leituras dos ADCs são proporcionais à tensão num mesmo instante,
  // descartamos a necessidade de conversão prévia, usamos os valores crus:
  relacao = (float)leituraA1 / (float)(leituraA0 - leituraA1);
  R2 = R1 * relacao;

  // Conversão visual entre ohms e kohms para facilitar leitura
  if (R2 >= 1000.0) 
  {
    Serial.print(R2 / 1000.0, 2); // Exibe com duas casas decimais
    Serial.print(" kohm  ");
  } 
  else
  {
    Serial.print(R2, 1);          // Exibe com uma casa decimal
    Serial.print(" ohm   ");
  }

  // Medidor de Tensão (Considerando referência VCC como 5V do Arduino)
  tensaoIn = leituraA0 * (5.0 / 1023.0);
  tensaoOut = leituraA1 * (5.0 / 1023.0);
  
// Também enviamos para a interface serial do IDE
  Serial.print("ADC In: "); Serial.print(leituraA0);
  Serial.print("\tADC Out: "); Serial.print(leituraA1);
  Serial.print("\tResistência: "); Serial.println(R2);

  delay(600); // 600ms de atualização evita que o display pisque exageradamente
}