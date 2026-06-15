/*
 * ============================================================================
 * ANALISADOR DE ESPECTRO DE ÁUDIO - 32 BANDAS COM DISPLAY MATRIZ LED 8x32
 * ============================================================================
 * 
 * OBJETIVO DIDÁTICO:
 * Este projeto demonstra como criar um analisador de espectro de áudio em tempo
 * real usando um Raspberry Pi Pico, um microfone de eletreto e um display de
 * matriz de LED 8x32 controlado pelo driver MAX7219.
 * 
 * FUNCIONAMENTO:
 * - O microfone capta o sinal de áudio analógico
 * - O Pico processa o sinal usando FFT (Transformada Rápida de Fourier)
 * - O resultado é exibido em 32 bandas de frequência com até 8 níveis de altura
 * 
 * HARDWARE NECESSÁRIO:
 * 1. Raspberry Pi Pico (ou compatível)
 * 2. Display Matriz LED 8x32 com MAX7219
 * 3. Microfone de Eletreto com pré-amplificador
 * 
 * CONEXÕES:
 * 
 * DISPLAY MAX7219 -> RASPBERRY PI PICO:
 * ----------------------------------------
 * VCC  -> 5V (ou 3.3V dependendo do módulo)
 * GND  -> GND
 * DIN  -> GPIO 19 (MOSI)
 * CS   -> GPIO 17 (Chip Select)
 * CLK  -> GPIO 18 (Clock)
 * 
 * MICROFONE ELETRETO -> RASPBERRY PI PICO:
 * ----------------------------------------
 * VCC  -> 3.3V (ou VSYS se usar amplificador externo)
 * GND  -> GND
 * OUT  -> GPIO 26 (ADC0 - Entrada Analógica)
 * 
 * NOTA: O microfone de eletreto requer um circuito de polarização adequado.
 * Recomenda-se usar um módulo pré-pronto com amplificador operacional.
 * 
 * BIBLIOTECAS NECESSÁRIAS:
 * - LedControl (para controlar o MAX7219)
 * - arm_math.h (biblioteca CMSIS-DSP para FFT no Pico)
 * 
 * AUTOR: Projeto Didático
 * DATA: 2024
 * VERSÃO: 1.0
 * ============================================================================
 */

// ============================================================================
// INCLUSÃO DE BIBLIOTECAS
// ============================================================================

#include <LedControl.h>
#include <hardware/adc.h>
#include <pico/stdlib.h>
#include <arm_math.h>

// ============================================================================
// DEFINIÇÕES DE PINOS E CONFIGURAÇÕES
// ============================================================================

// Pinos do display MAX7219
#define MAX7219_DIN   19    // Data In (MOSI)
#define MAX7219_CLK   18    // Clock
#define MAX7219_CS    17    // Chip Select

// Pino do microfone (entrada analógica)
#define MIC_PIN       26    // ADC0
#define ADC_PIN       26    // Mesmo pino, nome alternativo

// Configurações do ADC
#define ADC_RESOLUTION 12   // Resolução de 12 bits do Pico (0-4095)
#define ADC_MAX_VALUE  4095 // Valor máximo do ADC
#define ADC_REF_VOLT   3.3f // Tensão de referência em volts

// Configurações de áudio
#define SAMPLE_RATE   10000 // Taxa de amostragem em Hz (10 kHz)
#define AUDIO_SAMPLES 256   // Número de amostras para FFT (deve ser potência de 2)
#define FFT_SIZE      256   // Tamanho da FFT (igual ao número de amostras)

// Configurações do display
#define NUM_DEVICES   4     // Número de módulos MAX7219 em cascata (8x8 cada = 32 LEDs)
#define DISPLAY_COLS  32    // Total de colunas (8 x 4 módulos)
#define DISPLAY_ROWS  8     // Total de linhas por módulo

// Configurações do analisador de espectro
#define NUM_BANDS     32    // Número de bandas de frequência a exibir
#define MIN_FREQ      50    // Frequência mínima em Hz
#define MAX_FREQ      5000  // Frequência máxima em Hz
#define PEAK_DECAY    0.95f // Fator de decaimento do pico (0-1)
#define SENSITIVITY   2.0f  // Fator de sensibilidade do áudio

// ============================================================================
// VARIÁVEIS GLOBAIS
// ============================================================================

// Instância do controle do display MAX7219
// Parâmetros: DIN, CLK, CS, número de dispositivos
LedControl lc = LedControl(MAX7219_DIN, MAX7219_CLK, MAX7219_CS, NUM_DEVICES);

// Buffers para processamento de áudio
float32_t audioBuffer[FFT_SIZE];           // Buffer de entrada (amostras)
float32_t fftOutput[FFT_SIZE * 2];         // Buffer de saída da FFT (complexo)
arm_rfft_fast_instance_f32 fftInstance;    // Instância da FFT rápida
float32_t magnitude[FFT_SIZE / 2];         // Magnitude das frequências

// Array para armazenar os valores das bandas
uint8_t spectrumBands[NUM_BANDS];          // Valores atuais das bandas
uint8_t peakValues[NUM_BANDS];             // Valores de pico das bandas
unsigned long peakTimers[NUM_BANDS];       // Temporizadores para decaimento dos picos

// Variáveis de controle
unsigned long lastSampleTime = 0;          // Tempo da última amostra
unsigned long lastDisplayUpdate = 0;       // Tempo da última atualização do display
bool adcReady = false;                     // Flag de prontidão do ADC

// ============================================================================
// FUNÇÕES DE CONFIGURAÇÃO INICIAL
// ============================================================================

/**
 * @brief Configura o display MAX7219
 * 
 * Esta função inicializa todos os módulos MAX7219 conectados em cascata.
 * Configura brilho, modo de teste e ativa todos os displays.
 */
void setupDisplay() {
  Serial.println("Configurando display MAX7219...");
  
  // Loop através de todos os dispositivos MAX7219
  for (int i = 0; i < NUM_DEVICES; i++) {
    // Desliga o modo de teste (normal operation)
    lc.shutdown(i, false);
    
    // Define o brilho (0-15, sendo 15 o máximo)
    // Para uso didático, começamos com brilho médio para evitar ofuscamento
    lc.setIntensity(i, 8);
    
    // Limpa todos os LEDs inicialmente
    lc.clearDisplay(i);
  }
  
  Serial.println("Display configurado com sucesso!");
}

/**
 * @brief Configura o ADC do Raspberry Pi Pico
 * 
 * Inicializa o conversor analógico-digital para leitura do microfone.
 * O Pico possui ADC de 12 bits com taxa de amostragem configurável.
 */
void setupADC() {
  Serial.println("Configurando ADC...");
  
  // Inicializa o hardware do ADC
  adc_init();
  
  // Seleciona o pino GPIO como entrada ADC
  // gpio_set_function configura o pino para função ADC
  adc_gpio_init(MIC_PIN);
  
  // Seleciona o canal ADC correspondente ao pino
  // Canais: 0=GPIO26, 1=GPIO27, 2=GPIO28, 3=GPIO29 (temp interno)
  adc_select_input(ADC_PIN - 26);
  
  Serial.print("ADC configurado no GPIO ");
  Serial.println(MIC_PIN);
}

/**
 * @brief Configura a FFT (Fast Fourier Transform)
 * 
 * Inicializa a estrutura da biblioteca CMSIS-DSP para cálculo da FFT.
 * A FFT converte o sinal do domínio do tempo para o domínio da frequência.
 */
void setupFFT() {
  Serial.println("Configurando FFT...");
  
  // Inicializa a instância da FFT rápida
  // Esta função prepara as tabelas de twiddle factors necessárias
  arm_rfft_fast_init_f32(&fftInstance, FFT_SIZE);
  
  Serial.print("FFT configurada para ");
  Serial.print(FFT_SIZE);
  Serial.println(" pontos");
}

/**
 * @brief Função de configuração geral (chamada uma vez na inicialização)
 * 
 * Configura todas as periféricos e variáveis iniciais do sistema.
 */
void setup() {
  // Inicializa a comunicação serial para debug
  Serial.begin(115200);
  
  // Aguarda a porta serial estar pronta (útil para debug)
  while (!Serial) {
    sleep_ms(10);
  }
  
  Serial.println("\n========================================");
  Serial.println("ANALISADOR DE ESPECTRO - INICIANDO");
  Serial.println("========================================");
  
  // Configura todos os componentes
  setupDisplay();
  setupADC();
  setupFFT();
  
  // Inicializa arrays de estado
  for (int i = 0; i < NUM_BANDS; i++) {
    spectrumBands[i] = 0;
    peakValues[i] = 0;
    peakTimers[i] = 0;
  }
  
  Serial.println("\nSistema pronto! Aguardando áudio...");
  Serial.println("========================================\n");
}

// ============================================================================
// FUNÇÕES DE PROCESSAMENTO DE ÁUDIO
// ============================================================================

/**
 * @brief Lê uma amostra do ADC
 * 
 * @return uint16_t Valor da amostra (0-4095)
 * 
 * Esta função lê o valor atual do conversor analógico-digital.
 * O valor representa a amplitude instantânea do sinal de áudio.
 */
uint16_t readADC() {
  // Inicia a conversão ADC
  adc_run(true);
  
  // Lê o valor convertido (12 bits = 0 a 4095)
  uint16_t value = adc_read();
  
  return value;
}

/**
 * @brief Coleta amostras de áudio para processamento
 * 
 * Preenche o buffer de áudio com amostras coletadas em intervalos regulares.
 * Aplica remoção de DC (centra o sinal em zero) para melhor processamento.
 */
void collectAudioSamples() {
  // Calcula o intervalo entre amostras baseado na sample rate
  uint32_t sampleInterval = 1000000 / SAMPLE_RATE; // em microssegundos
  
  uint32_t startTime = time_us_32();
  
  // Coleta FFT_SIZE amostras
  for (int i = 0; i < FFT_SIZE; i++) {
    // Lê a amostra do ADC
    uint16_t rawValue = readADC();
    
    // Converte para float e remove o offset DC (centra em zero)
    // O valor médio do ADC é 2048 (metade de 4095)
    audioBuffer[i] = ((float32_t)rawValue - 2048.0f) / 2048.0f;
    
    // Aguarda o próximo intervalo de amostragem
    uint32_t currentTime = time_us_32();
    uint32_t elapsedTime = currentTime - startTime;
    
    // Delay preciso para manter a taxa de amostragem constante
    if (elapsedTime < sampleInterval * (i + 1)) {
      sleep_us(sampleInterval * (i + 1) - elapsedTime);
    }
  }
}

/**
 * @brief Aplica uma janela de Hamming às amostras
 * 
 * @param buffer Ponteiro para o buffer de amostras
 * @param size Tamanho do buffer
 * 
 * A janela de Hamming reduz vazamento espectral (spectral leakage)
 * melhorando a precisão da análise de frequência.
 */
void applyHammingWindow(float32_t* buffer, int size) {
  for (int i = 0; i < size; i++) {
    // Fórmula da janela de Hamming: w(n) = 0.54 - 0.46 * cos(2πn/N)
    float32_t window = 0.54f - 0.46f * arm_cos_f32(2.0f * ARM_PI * i / size);
    buffer[i] *= window;
  }
}

/**
 * @brief Calcula a FFT das amostras de áudio
 * 
 * Processa o buffer de áudio e calcula a magnitude das frequências.
 * O resultado é armazenado no array magnitude[].
 */
void calculateFFT() {
  // Aplica janela de Hamming para melhor precisão
  applyHammingWindow(audioBuffer, FFT_SIZE);
  
  // Calcula a FFT rápida
  // A função espera dados no formato [real, imag, real, imag, ...]
  // Como temos apenas dados reais, os imaginários são zero
  arm_rfft_fast_f32(&fftInstance, audioBuffer, fftOutput, 0);
  
  // Calcula a magnitude de cada componente de frequência
  // A magnitude é sqrt(real² + imag²)
  arm_cmplx_mag_f32(fftOutput, magnitude, FFT_SIZE / 2);
}

/**
 * @brief Mapeia as magnitudes da FFT para bandas de frequência
 * 
 * Distribui as 128 saídas da FFT em 32 bandas visuais,
 * considerando uma escala logarítmica mais adequada à percepção humana.
 */
void mapToBands() {
  // Frequência de cada bin da FFT
  float32_t freqPerBin = (float32_t)SAMPLE_RATE / FFT_SIZE; // ~39 Hz/bin
  
  // Para cada banda visual
  for (int band = 0; band < NUM_BANDS; band++) {
    // Calcula as frequências inicial e final desta banda
    // Usamos escala logarítmica para melhor distribuição perceptual
    float32_t startFreq = MIN_FREQ * powf((float32_t)MAX_FREQ / MIN_FREQ, 
                                          (float32_t)band / NUM_BANDS);
    float32_t endFreq = MIN_FREQ * powf((float32_t)MAX_FREQ / MIN_FREQ, 
                                        (float32_t)(band + 1) / NUM_BANDS);
    
    // Converte para índices da FFT
    int startBin = (int)(startFreq / freqPerBin);
    int endBin = (int)(endFreq / freqPerBin);
    
    // Garante limites válidos
    startBin = constrain(startBin, 0, FFT_SIZE / 2 - 1);
    endBin = constrain(endBin, 0, FFT_SIZE / 2 - 1);
    
    // Calcula a média das magnitudes nesta faixa de frequência
    float32_t sum = 0.0f;
    int count = 0;
    
    for (int bin = startBin; bin <= endBin && bin < FFT_SIZE / 2; bin++) {
      sum += magnitude[bin];
      count++;
    }
    
    // Evita divisão por zero
    if (count > 0) {
      sum /= count;
    }
    
    // Aplica sensibilidade e converte para nível (0-7)
    float32_t level = sum * SENSITIVITY * 10.0f;
    
    // Converte para valor inteiro de 0 a 7 (8 linhas do display)
    uint8_t value = (uint8_t)constrain((int)level, 0, 7);
    
    // Atualiza o valor da banda
    spectrumBands[band] = value;
  }
}

/**
 * @brief Atualiza os valores de pico das bandas
 * 
 * Mantém temporariamente o valor máximo atingido por cada banda,
 * criando o efeito visual de "peak hold".
 */
void updatePeaks() {
  unsigned long currentTime = millis();
  
  for (int i = 0; i < NUM_BANDS; i++) {
    // Se o valor atual é maior que o pico, atualiza
    if (spectrumBands[i] >= peakValues[i]) {
      peakValues[i] = spectrumBands[i];
      peakTimers[i] = currentTime;
    } else {
      // Decai o pico após um tempo sem atingir novo máximo
      if (currentTime - peakTimers[i] > 500) { // 500ms de delay
        if (peakValues[i] > 0) {
          peakValues[i]--;
          peakTimers[i] = currentTime;
        }
      }
    }
  }
}

// ============================================================================
// FUNÇÕES DE EXIBIÇÃO NO DISPLAY
// ============================================================================

/**
 * @brief Limpa todo o display
 * 
 * Desliga todos os LEDs em todos os módulos MAX7219.
 */
void clearDisplay() {
  for (int i = 0; i < NUM_DEVICES; i++) {
    lc.clearDisplay(i);
  }
}

/**
 * @brief Desenha uma barra vertical no display
 * 
 * @param column Coluna onde desenhar (0-31)
 * @param height Altura da barra (0-7)
 * @param isPeak Se true, desenha apenas o LED de pico
 * 
 * Cada coluna do display representa uma banda de frequência.
 * A altura representa a intensidade daquela frequência.
 */
void drawBar(uint8_t column, uint8_t height, bool isPeak = false) {
  // Determina qual módulo MAX7219 contém esta coluna
  int device = column / 8;
  int colInDevice = column % 8;
  
  // Desenha a barra (ou o pico)
  for (int row = 0; row < 8; row++) {
    bool ledOn = false;
    
    if (isPeak) {
      // Acende apenas o LED do pico
      ledOn = (row == (7 - height));
    } else {
      // Acende LEDs até a altura da barra
      ledOn = (row >= (8 - height));
    }
    
    // Define o estado do LED
    // Nota: LedControl usa coordenadas (device, row, col)
    lc.setLed(device, row, colInDevice, ledOn);
  }
}

/**
 * @brief Atualiza o display com o espectro atual
 * 
 * Desenha todas as 32 bandas no display, incluindo os picos.
 */
void updateDisplay() {
  // Limpa o display antes de redesenhar
  clearDisplay();
  
  // Desenha cada banda
  for (int i = 0; i < NUM_BANDS; i++) {
    // Desenha a barra principal
    drawBar(i, spectrumBands[i], false);
    
    // Desenha o indicador de pico (se diferente da barra)
    if (peakValues[i] > spectrumBands[i]) {
      drawBar(i, peakValues[i], true);
    }
  }
}

/**
 * @brief Exibe uma animação de inicialização
 * 
 * Mostra um padrão visual para indicar que o sistema está funcionando.
 */
void showStartupAnimation() {
  Serial.println("Executando animação de inicialização...");
  
  // Animação de varredura
  for (int pass = 0; pass < 2; pass++) {
    for (int col = 0; col < DISPLAY_COLS; col++) {
      clearDisplay();
      
      // Desenha uma linha vertical na coluna atual
      for (int row = 0; row < DISPLAY_ROWS; row++) {
        int device = col / 8;
        int colInDevice = col % 8;
        lc.setLed(device, row, colInDevice, true);
      }
      
      delay(30);
    }
  }
  
  clearDisplay();
}

// ============================================================================
// FUNÇÃO PRINCIPAL (LOOP)
// ============================================================================

/**
 * @brief Função principal executada em loop
 * 
 * Executa continuamente o ciclo de:
 * 1. Coletar amostras de áudio
 * 2. Processar FFT
 * 3. Mapear para bandas
 * 4. Atualizar display
 */
void loop() {
  // Coleta e processa amostras de áudio
  collectAudioSamples();
  
  // Calcula a FFT
  calculateFFT();
  
  // Mapeia as frequências para as bandas do display
  mapToBands();
  
  // Atualiza os indicadores de pico
  updatePeaks();
  
  // Atualiza o display
  updateDisplay();
  
  // Pequeno delay para estabilidade
  delay(10);
  
  // Opcional: Envia dados para o Serial Monitor para debug
  // printSpectrum(); // Descomente para ver dados no serial
}

// ============================================================================
// FUNÇÕES AUXILIARES DE DEBUG
// ============================================================================

/**
 * @brief Imprime o espectro atual no Serial Monitor
 * 
 * Útil para debug e visualização dos dados brutos.
 * Mostra o valor de cada banda em formato de gráfico ASCII.
 */
void printSpectrum() {
  static unsigned long lastPrint = 0;
  
  // Limita a impressão a 10 vezes por segundo
  if (millis() - lastPrint < 100) {
    return;
  }
  lastPrint = millis();
  
  Serial.print("Espectro: [");
  for (int i = 0; i < NUM_BANDS; i++) {
    Serial.print(spectrumBands[i]);
    if (i < NUM_BANDS - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("]");
  
  // Imprime gráfico ASCII simples
  for (int row = 7; row >= 0; row--) {
    Serial.print(row + 1);
    Serial.print("|");
    for (int col = 0; col < NUM_BANDS; col++) {
      if (spectrumBands[col] >= row + 1) {
        Serial.print("##");
      } else if (peakValues[col] >= row + 1) {
        Serial.print("--");
      } else {
        Serial.print("  ");
      }
    }
    Serial.println();
  }
  Serial.println("+--------------------------------");
  Serial.println(" Freq ->");
  Serial.println();
}

/**
 * @brief Testa individualmente cada LED do display
 * 
 * Função de debug para verificar se todos os LEDs estão funcionando.
 */
void testAllLEDs() {
  Serial.println("Testando todos os LEDs...");
  
  for (int device = 0; device < NUM_DEVICES; device++) {
    for (int row = 0; row < 8; row++) {
      for (int col = 0; col < 8; col++) {
        lc.setLed(device, row, col, true);
        delay(10);
      }
    }
  }
  
  delay(1000);
  clearDisplay();
}

/*
 * ============================================================================
 * NOTAS PARA EXPANSÃO E MODIFICAÇÃO
 * ============================================================================
 * 
 * 1. AJUSTE DE SENSIBILIDADE:
 *    - Modifique a constante SENSITIVITY para ajustar a resposta ao áudio
 *    - Valores maiores = mais sensível, Valores menores = menos sensível
 * 
 * 2. TAXA DE AMOSTRAGEM:
 *    - SAMPLE_RATE define a máxima frequência detectável (Nyquist: max = rate/2)
 *    - Aumentar sample rate = detectar frequências mais altas
 *    - Diminuir sample rate = melhor resolução em baixas frequências
 * 
 * 3. NÚMERO DE BANDAS:
 *    - NUM_BANDS pode ser ajustado (recomendado: 16, 32 ou 64)
 *    - Mais bandas = maior resolução frequencial
 *    - Menos bandas = barras mais largas no display
 * 
 * 4. BRILHO DO DISPLAY:
 *    - Ajuste em setupDisplay() com lc.setIntensity(device, 0-15)
 *    - 0 = mínimo, 15 = máximo
 * 
 * 5. DECAY DOS PICOS:
 *    - Ajuste o tempo em updatePeaks() (atualmente 500ms)
 *    - Tempos maiores = picos descem mais lentamente
 * 
 * 6. FILTRAGEM ADICIONAL:
 *    - Pode-se adicionar filtros passa-baixa/alta no sinal de áudio
 *    - Útil para remover ruído ou focar em faixas específicas
 * 
 * 7. CALIBRAÇÃO DO MICROFONE:
 *    - O circuito do microfone deve fornecer sinal centrado em ~1.65V
 *    - Ajuste o offset em collectAudioSamples() se necessário
 * 
 * ============================================================================
 * REFERÊNCIAS TÉCNICAS
 * ============================================================================
 * 
 * - FFT (Fast Fourier Transform): Algoritmo eficiente para calcular a DFT
 * - Teorema de Nyquist: Frequência máxima detectável = Sample Rate / 2
 * - Janela de Hamming: Reduz spectral leakage em análise de Fourier
 * - MAX7219: Driver serial para displays de matriz LED 8x8
 * - CMSIS-DSP: Biblioteca de processamento de sinal da ARM
 * 
 * ============================================================================
 */
