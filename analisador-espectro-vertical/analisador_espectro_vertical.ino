/*
 * ============================================================================
 * ANALISADOR DE ESPECTRO VERTICAL DE ÁUDIO - 8 BANDAS COM DISPLAY MATRIZ LED 8x32
 * ============================================================================
 * 
 * OBJETIVO DIDÁTICO:
 * Este projeto demonstra como criar um analisador de espectro de áudio em tempo
 * real usando um ESP32-C3-Zero, um microfone de eletreto e um display de
 * matriz de LED 8x32 controlado pelo driver MAX7219.
 * 
 * FUNCIONAMENTO:
 * - O microfone capta o sinal de áudio analógico
 * - O ESP32-C3 processa o sinal usando FFT (Transformada Rápida de Fourier)
 * - O resultado é exibido em 8 bandas verticais de frequência com até 32 níveis
 *   de altura (usando scroll horizontal para mostrar toda a faixa dinâmica)
 * 
 * DIFERENÇAS DA VERSÃO HORIZONTAL:
 * - Versão horizontal: 32 bandas x 8 linhas (frequência nas colunas)
 * - Versão vertical: 8 bandas x 32 linhas (frequência nas linhas, amplitude vertical)
 * 
 * HARDWARE NECESSÁRIO:
 * 1. ESP32-C3-Zero (ou compatível ESP32-C3)
 * 2. Display Matriz LED 8x32 com MAX7219
 * 3. Microfone de Eletreto com pré-amplificador
 * 
 * CONEXÕES:
 * 
 * DISPLAY MAX7219 -> ESP32-C3-ZERO:
 * ----------------------------------------
 * VCC  -> 5V (ou 3.3V dependendo do módulo)
 * GND  -> GND
 * DIN  -> GPIO 7 (SPI MOSI)
 * CS   -> GPIO 10 (Chip Select)
 * CLK  -> GPIO 6 (SPI Clock)
 * 
 * MICROFONE ELETRETO -> ESP32-C3-ZERO:
 * ----------------------------------------
 * VCC  -> 3.3V
 * GND  -> GND
 * OUT  -> GPIO 0 (ADC1_CH0 - Entrada Analógica)
 * 
 * NOTA: O microfone de eletreto requer um circuito de polarização adequado.
 * Recomenda-se usar um módulo pré-pronto com amplificador operacional.
 * 
 * BIBLIOTECAS NECESSÁRIAS:
 * - LedControl (para controlar o MAX7219)
 * - esp_dsp (biblioteca oficial da Espressif para DSP/FFT)
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
#include <esp_dsp.h>
#include <Arduino.h>

// ============================================================================
// DEFINIÇÕES DE PINOS E CONFIGURAÇÕES
// ============================================================================

// Pinos do display MAX7219 (ESP32-C3)
#define MAX7219_DIN   7     // Data In (MOSI)
#define MAX7219_CLK   6     // Clock
#define MAX7219_CS    10    // Chip Select

// Pino do microfone (entrada analógica)
#define MIC_PIN       0     // ADC1_CH0 (GPIO 0)

// Configurações do ADC
#define ADC_ATTENUATION ADC_ATTEN_DB_11  // Atenuação para maior faixa dinâmica
#define ADC_MAX_VALUE   4095             // Valor máximo do ADC (12 bits)
#define ADC_MIN_VALUE   0                // Valor mínimo do ADC

// Configurações de áudio
#define SAMPLE_RATE     8000      // Taxa de amostragem em Hz (8 kHz)
#define AUDIO_SAMPLES   256       // Número de amostras para FFT (potência de 2)
#define FFT_SIZE        256       // Tamanho da FFT

// Configurações do display
#define NUM_DEVICES     4         // Número de módulos MAX7219 em cascata (8x8 cada = 32 LEDs)
#define DISPLAY_COLS    32        // Total de colunas (8 x 4 módulos)
#define DISPLAY_ROWS    8         // Total de linhas por módulo

// Configurações do analisador de espectro VERTICAL
#define NUM_BANDS       8         // Número de bandas de frequência (8 bandas verticais)
#define MIN_FREQ        100       // Frequência mínima em Hz
#define MAX_FREQ        4000      // Frequência máxima em Hz
#define PEAK_DECAY_MS   100       // Tempo de decaimento do pico em ms
#define SENSITIVITY     3.0f      // Fator de sensibilidade do áudio
#define SCROLL_SPEED    50        // Velocidade de scroll em ms por linha

// ============================================================================
// VARIÁVEIS GLOBAIS
// ============================================================================

// Instância do controle do display MAX7219
// Parâmetros: DIN, CLK, CS, número de dispositivos
LedControl lc = LedControl(MAX7219_DIN, MAX7219_CLK, MAX7219_CS, NUM_DEVICES);

// Buffers para processamento de áudio
float audioBuffer[FFT_SIZE];           // Buffer de entrada (amostras)
float fftOutput[FFT_SIZE];             // Buffer de saída da FFT (magnitudes)
dsp_fft_complex_instance_t fftInst;    // Instância da FFT

// Array para armazenar os valores das bandas
uint8_t spectrumBands[NUM_BANDS];      // Valores atuais das bandas (0-32)
uint8_t peakValues[NUM_BANDS];         // Valores de pico das bandas (0-32)
unsigned long peakTimers[NUM_BANDS];   // Temporizadores para decaimento dos picos

// Variáveis de controle
unsigned long lastUpdate = 0;          // Tempo da última atualização
bool initialized = false;              // Flag de inicialização

// Scroll vertical para exibir faixas maiores que 8 linhas
int scrollOffset = 0;                  // Offset atual do scroll (0-24 para 32-8)
unsigned long lastScroll = 0;          // Tempo do último scroll

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
    lc.setIntensity(i, 10);
    
    // Limpa todos os LEDs inicialmente
    lc.clearDisplay(i);
  }
  
  Serial.println("Display configurado com sucesso!");
}

/**
 * @brief Configura o ADC do ESP32-C3
 * 
 * Inicializa o conversor analógico-digital para leitura do microfone.
 * O ESP32-C3 possui ADC de 12 bits (0-4095).
 */
void setupADC() {
  Serial.println("Configurando ADC...");
  
  // Configura atenuação do ADC para maior faixa de tensão (0-3.3V)
  analogSetAttenuation(ADC_ATTENUATION);
  
  // Configura resolução do ADC (12 bits)
  analogReadResolution(12);
  
  Serial.print("ADC configurado no GPIO ");
  Serial.println(MIC_PIN);
}

/**
 * @brief Configura a FFT (Fast Fourier Transform)
 * 
 * Inicializa a estrutura da biblioteca esp_dsp para cálculo da FFT.
 * A FFT converte o sinal do domínio do tempo para o domínio da frequência.
 */
void setupFFT() {
  Serial.println("Configurando FFT...");
  
  // Inicializa a instância da FFT
  // A biblioteca esp_dsp é otimizada para ESP32
  dsp_fft_init();
  
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
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("ANALISADOR DE ESPECTRO VERTICAL");
  Serial.println("========================================");
  Serial.println("Hardware: ESP32-C3-Zero + MAX7219 8x32");
  Serial.println("Bandas: 8 verticais com até 32 linhas");
  Serial.println("========================================\n");
  
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
  
  // Mostra animação de inicialização
  showStartupAnimation();
  
  initialized = true;
  
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
  // Lê o valor convertido (12 bits = 0 a 4095)
  uint16_t value = analogRead(MIC_PIN);
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
  
  uint32_t startTime = micros();
  
  // Coleta FFT_SIZE amostras
  for (int i = 0; i < FFT_SIZE; i++) {
    // Lê a amostra do ADC
    uint16_t rawValue = readADC();
    
    // Converte para float e remove o offset DC (centra em zero)
    // O valor médio esperado é ~2048 (metade de 4095)
    audioBuffer[i] = ((float)rawValue - 2048.0f) / 2048.0f;
    
    // Delay preciso para manter a taxa de amostragem constante
    uint32_t elapsed = micros() - startTime;
    uint32_t expected = sampleInterval * (i + 1);
    
    if (elapsed < expected) {
      delayMicroseconds(expected - elapsed);
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
void applyHammingWindow(float* buffer, int size) {
  for (int i = 0; i < size; i++) {
    // Fórmula da janela de Hamming: w(n) = 0.54 - 0.46 * cos(2πn/N)
    float window = 0.54f - 0.46f * cosf(2.0f * PI * i / size);
    buffer[i] *= window;
  }
}

/**
 * @brief Calcula a FFT das amostras de áudio
 * 
 * Processa o buffer de áudio e calcula a magnitude das frequências.
 * O resultado é armazenado no array fftOutput[].
 */
void calculateFFT() {
  // Aplica janela de Hamming para melhor precisão
  applyHammingWindow(audioBuffer, FFT_SIZE);
  
  // Prepara buffer complexo (intercala partes real e imaginária)
  float complexBuffer[FFT_SIZE * 2];
  for (int i = 0; i < FFT_SIZE; i++) {
    complexBuffer[i * 2] = audioBuffer[i];      // Parte real
    complexBuffer[i * 2 + 1] = 0.0f;            // Parte imaginária (zero)
  }
  
  // Calcula a FFT usando a biblioteca esp_dsp
  // Resultado fica em complexBuffer no formato [real, imag, real, imag, ...]
  dsps_fft_float(complexBuffer, FFT_SIZE);
  
  // Calcula a magnitude de cada componente de frequência
  // Magnitude = sqrt(real² + imag²)
  for (int i = 0; i < FFT_SIZE / 2; i++) {
    float real = complexBuffer[i * 2];
    float imag = complexBuffer[i * 2 + 1];
    fftOutput[i] = sqrtf(real * real + imag * imag);
  }
}

/**
 * @brief Mapeia as magnitudes da FFT para bandas de frequência
 * 
 * Distribui as 128 saídas da FFT em 8 bandas visuais,
 * considerando uma escala logarítmica mais adequada à percepção humana.
 * Cada banda pode ter até 32 níveis de altura.
 */
void mapToBands() {
  // Frequência de cada bin da FFT
  float freqPerBin = (float)SAMPLE_RATE / FFT_SIZE; // ~31.25 Hz/bin
  
  // Para cada banda visual (8 bandas totais)
  for (int band = 0; band < NUM_BANDS; band++) {
    // Calcula as frequências inicial e final desta banda
    // Usamos escala logarítmica para melhor distribuição perceptual
    float startFreq = MIN_FREQ * powf((float)MAX_FREQ / MIN_FREQ, 
                                      (float)band / NUM_BANDS);
    float endFreq = MIN_FREQ * powf((float)MAX_FREQ / MIN_FREQ, 
                                    (float)(band + 1) / NUM_BANDS);
    
    // Converte para índices da FFT
    int startBin = (int)(startFreq / freqPerBin);
    int endBin = (int)(endFreq / freqPerBin);
    
    // Garante limites válidos
    startBin = constrain(startBin, 0, FFT_SIZE / 2 - 1);
    endBin = constrain(endBin, 0, FFT_SIZE / 2 - 1);
    
    // Calcula a média das magnitudes nesta faixa de frequência
    float sum = 0.0f;
    int count = 0;
    
    for (int bin = startBin; bin <= endBin && bin < FFT_SIZE / 2; bin++) {
      sum += fftOutput[bin];
      count++;
    }
    
    // Evita divisão por zero
    if (count > 0) {
      sum /= count;
    }
    
    // Aplica sensibilidade e converte para nível (0-32)
    // Multiplicamos por um fator para expandir a faixa dinâmica
    float level = sum * SENSITIVITY * 50.0f;
    
    // Converte para valor inteiro de 0 a 32 (32 linhas de altura máxima)
    uint8_t value = (uint8_t)constrain((int)level, 0, 32);
    
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
      if (currentTime - peakTimers[i] > PEAK_DECAY_MS) {
        if (peakValues[i] > 0) {
          peakValues[i]--;
          peakTimers[i] = currentTime;
        }
      }
    }
  }
}

// ============================================================================
// FUNÇÕES DE EXIBIÇÃO NO DISPLAY (MODO VERTICAL)
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
 * @brief Desenha uma barra vertical no display com scroll
 * 
 * @param band Índice da banda (0-7)
 * @param height Altura da barra (0-32)
 * @param isPeak Se true, desenha apenas o LED de pico
 * 
 * No modo vertical:
 * - Cada banda ocupa 4 colunas do display (32 colunas / 8 bandas = 4 colunas/banda)
 * - A altura varia de 0 a 32 linhas
 * - Usamos scroll vertical para mostrar toda a faixa quando height > 8
 */
void drawVerticalBar(uint8_t band, uint8_t height, bool isPeak = false) {
  // Cada banda ocupa 4 colunas horizontais
  int colStart = band * 4;
  
  // Determina qual região do scroll estamos mostrando
  // O scroll permite visualizar alturas maiores que 8 linhas
  int visibleStart = scrollOffset;
  int visibleEnd = scrollOffset + DISPLAY_ROWS; // 8 linhas visíveis
  
  // Desenha em cada coluna da banda
  for (int col = colStart; col < colStart + 4 && col < DISPLAY_COLS; col++) {
    int device = col / 8;          // Qual módulo MAX7219 (0-3)
    int colInDevice = col % 8;     // Coluna dentro do módulo
    
    // Desenha em cada linha visível (0-7 no módulo)
    for (int row = 0; row < 8; row++) {
      // Calcula a posição absoluta da linha (0-31)
      int absoluteRow = visibleStart + row;
      
      bool ledOn = false;
      
      if (isPeak) {
        // Acende apenas o LED do pico
        // O pico é desenhado na posição correspondente à sua altura
        ledOn = (absoluteRow == (32 - height));
      } else {
        // Acende LEDs da base até a altura da barra
        // Barras crescem de baixo para cima
        ledOn = (absoluteRow >= (32 - height));
      }
      
      // Define o estado do LED
      lc.setLed(device, row, colInDevice, ledOn);
    }
  }
}

/**
 * @brief Atualiza o scroll vertical para mostrar diferentes faixas de altura
 * 
 * Implementa scroll automático para visualizar barras com altura > 8 linhas.
 */
void updateScroll() {
  unsigned long currentTime = millis();
  
  // Encontra a altura máxima atual
  uint8_t maxHeight = 0;
  for (int i = 0; i < NUM_BANDS; i++) {
    if (spectrumBands[i] > maxHeight) {
      maxHeight = spectrumBands[i];
    }
  }
  
  // Ajusta o scroll para mostrar a região mais relevante
  // Se a altura máxima for > 8, faz scroll para mostrar o topo
  if (maxHeight > DISPLAY_ROWS) {
    // Scroll segue o topo das barras mais altas
    int targetScroll = maxHeight - DISPLAY_ROWS;
    targetScroll = constrain(targetScroll, 0, 32 - DISPLAY_ROWS); // 0-24
    
    // Move o scroll gradualmente
    if (currentTime - lastScroll > SCROLL_SPEED) {
      if (scrollOffset < targetScroll) {
        scrollOffset++;
      } else if (scrollOffset > targetScroll) {
        scrollOffset--;
      }
      lastScroll = currentTime;
    }
  } else {
    // Se todas as barras são baixas, mostra a base (scroll = 0)
    if (scrollOffset > 0 && currentTime - lastScroll > SCROLL_SPEED) {
      scrollOffset--;
      lastScroll = currentTime;
    }
  }
}

/**
 * @brief Atualiza o display com o espectro vertical atual
 * 
 * Desenha todas as 8 bandas verticais no display, incluindo os picos.
 */
void updateDisplay() {
  // Atualiza o scroll primeiro
  updateScroll();
  
  // Limpa o display antes de redesenhar
  clearDisplay();
  
  // Desenha cada banda vertical
  for (int i = 0; i < NUM_BANDS; i++) {
    // Desenha a barra principal
    drawVerticalBar(i, spectrumBands[i], false);
    
    // Desenha o indicador de pico (se diferente da barra)
    if (peakValues[i] > spectrumBands[i]) {
      drawVerticalBar(i, peakValues[i], true);
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
  
  // Animação de preenchimento vertical
  for (int pass = 0; pass < 2; pass++) {
    for (int level = 0; level < 8; level++) {
      clearDisplay();
      
      // Acende LEDs progressivamente de baixo para cima
      for (int device = 0; device < NUM_DEVICES; device++) {
        for (int row = 7 - level; row < 8; row++) {
          for (int col = 0; col < 8; col++) {
            lc.setLed(device, row, col, true);
          }
        }
      }
      
      delay(50);
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
 * 3. Mapear para bandas verticais
 * 4. Atualizar display com scroll
 */
void loop() {
  // Coleta e processa amostras de áudio
  collectAudioSamples();
  
  // Calcula a FFT
  calculateFFT();
  
  // Mapeia as frequências para as 8 bandas verticais
  mapToBands();
  
  // Atualiza os indicadores de pico
  updatePeaks();
  
  // Atualiza o display (com scroll vertical)
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
 * Mostra o valor de cada banda em formato de gráfico ASCII vertical.
 */
void printSpectrum() {
  static unsigned long lastPrint = 0;
  
  // Limita a impressão a 5 vezes por segundo
  if (millis() - lastPrint < 200) {
    return;
  }
  lastPrint = millis();
  
  Serial.print("Scroll: ");
  Serial.print(scrollOffset);
  Serial.print(" | Bandas: [");
  for (int i = 0; i < NUM_BANDS; i++) {
    Serial.print(spectrumBands[i]);
    if (i < NUM_BANDS - 1) {
      Serial.print(", ");
    }
  }
  Serial.println("]");
  
  // Imprime gráfico ASCII vertical simplificado
  // Mostra apenas as 8 linhas visíveis no momento
  for (int row = 7; row >= 0; row--) {
    int absoluteRow = scrollOffset + row;
    Serial.print(32 - absoluteRow);
    Serial.print("|");
    
    for (int band = 0; band < NUM_BANDS; band++) {
      if (spectrumBands[band] >= absoluteRow + 1) {
        Serial.print("####");
      } else if (peakValues[band] >= absoluteRow + 1) {
        Serial.print("----");
      } else {
        Serial.print("    ");
      }
    }
    Serial.println();
  }
  Serial.println("+--------------------------------");
  Serial.println(" Bandas: 0    1    2    3    4    5    6    7");
  Serial.println(" Freq:  Graves -> Médios -> Agudos");
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
        delay(5);
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
 *    - Para ESP32-C3, 8000 Hz é suficiente para voz e música básica
 * 
 * 3. NÚMERO DE BANDAS:
 *    - NUM_BANDS está fixo em 8 para o modo vertical
 *    - Cada banda ocupa 4 colunas do display (32/8 = 4)
 * 
 * 4. ALTURA MÁXIMA:
 *    - O sistema suporta até 32 linhas de altura
 *    - O scroll automático ajusta a visualização
 * 
 * 5. BRILHO DO DISPLAY:
 *    - Ajuste em setupDisplay() com lc.setIntensity(device, 0-15)
 *    - 0 = mínimo, 15 = máximo
 * 
 * 6. DECAY DOS PICOS:
 *    - Ajuste PEAK_DECAY_MS em milliseconds
 *    - Valores maiores = picos descem mais lentamente
 * 
 * 7. VELOCIDADE DO SCROLL:
 *    - Ajuste SCROLL_SPEED em ms
 *    - Valores menores = scroll mais rápido e suave
 * 
 * 8. FAIXA DE FREQUÊNCIAS:
 *    - MIN_FREQ e MAX_FREQ definem o range analisado
 *    - Padrão: 100 Hz a 4000 Hz (focado em voz e música)
 * 
 * ============================================================================
 * REFERÊNCIAS TÉCNICAS
 * ============================================================================
 * 
 * - FFT (Fast Fourier Transform): Algoritmo eficiente para calcular a DFT
 * - Teorema de Nyquist: Frequência máxima detectável = Sample Rate / 2
 * - Janela de Hamming: Reduz spectral leakage em análise de Fourier
 * - MAX7219: Driver serial para displays de matriz LED 8x8
 * - esp_dsp: Biblioteca de processamento de sinal da Espressif
 * 
 * ============================================================================
 * DIFERENÇAS ENTRE VERSÕES
 * ============================================================================
 * 
 * VERSÃO HORIZONTAL (analisador_espectro.ino):
 * - Hardware: Raspberry Pi Pico
 * - Display: 32 bandas x 8 linhas
 * - Frequência nas colunas, amplitude na altura (fixa em 8 níveis)
 * - Ideal para ver muitas bandas de frequência simultaneamente
 * 
 * VERSÃO VERTICAL (este arquivo):
 * - Hardware: ESP32-C3-Zero
 * - Display: 8 bandas x 32 linhas (com scroll)
 * - Frequência nas bandas, amplitude vertical (até 32 níveis)
 * - Ideal para ver dinâmica e variação de intensidade com alta resolução
 * 
 * ============================================================================
 */
