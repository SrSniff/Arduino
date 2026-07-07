# Analisador de Espectro Vertical de Áudio

## Visão Geral do Projeto

Este projeto implementa um analisador de espectro de áudio em tempo real com visualização **VERTICAL** utilizando:
- **ESP32-C3-Zero** como microcontrolador principal
- **Display Matriz LED 8x32** com driver MAX7219 para visualização
- **Microfone de Eletreto** para captura de áudio

O sistema analisa o sinal de áudio em tempo real e exibe **8 bandas verticais de frequência** com até **32 linhas de altura** (usando scroll automático para mostrar toda a faixa dinâmica).

---

## Diferença Entre Versões

### Versão Horizontal (analisador_espectro.ino)
- Hardware: Raspberry Pi Pico
- Display: **32 bandas × 8 linhas**
- Frequência distribuída nas colunas
- Amplitude fixa em 8 níveis
- Ideal para ver muitas bandas de frequência simultaneamente

### Versão Vertical (este projeto)
- Hardware: ESP32-C3-Zero
- Display: **8 bandas × 32 linhas** (com scroll)
- Frequência distribuída em 8 bandas
- Amplitude vertical com alta resolução (até 32 níveis)
- Ideal para ver dinâmica e variação de intensidade

---

## Lista de Materiais

### Componentes Principais

| Item | Quantidade | Descrição |
|------|------------|-----------|
| ESP32-C3-Zero | 1 | Microcontrolador com WiFi/Bluetooth |
| Display MAX7219 8x32 | 1 | Matriz de LED (4 módulos 8x8 em cascata) |
| Microfone de Eletreto | 1 | Com módulo pré-amplificador |
| Jumpers | Vários | Para conexões |
| Protoboard | 1 | Para montagem do circuito |

### Especificações Técnicas

- **Microcontrolador**: ESP32-C3-Zero (RISC-V de 32 bits, 160 MHz)
- **ADC**: 12 bits, 0-3.3V
- **Display**: 32 colunas × 8 linhas (256 LEDs)
- **Bandas de Frequência**: 8 bandas distribuídas de 100Hz a 4kHz
- **Taxa de Amostragem**: 8 kHz
- **FFT Size**: 256 pontos
- **Altura Máxima**: 32 níveis (com scroll automático)

---

## Diagrama de Conexões

### Display MAX7219 → ESP32-C3-ZERO

```
MAX7219          ESP32-C3
───────          ────────
VCC         →    5V (ou 3.3V)*
GND         →    GND
DIN         →    GPIO 7 (SPI MOSI)
CS          →    GPIO 10
CLK         →    GPIO 6
```

*Verifique a tensão de operação do seu módulo MAX7219

### Microfone → ESP32-C3-ZERO

```
Microfone      ESP32-C3
─────────      ────────
VCC        →   3.3V
GND        →   GND
OUT        →   GPIO 0 (ADC1_CH0)
```

**Importante**: O GPIO 0 do ESP32-C3 deve estar livre (não conectado a nada durante o boot).

---

## Montagem do Circuito do Microfone

### Opção 1: Módulo Pronto (Recomendado para Iniciantes)

Utilize um módulo de microfone de eletreto já com amplificação, como:
- KY-037
- MAX4466
- MAX9814

Estes módulos já incluem:
- Polarização correta do eletreto
- Amplificação do sinal
- Filtro de ruído
- Saída compatível com ADC

### Opção 2: Circuito Discreto

Para fins didáticos, segue esquema básico:

```
         3.3V
          │
         10kΩ (Resistor de pull-up)
          │
          ├────→ GPIO 0 (ADC)
          │
         1uF (Capacitor de acoplamento)
          │
     ┌────┴────┐
     │         │
Microfone   1kΩ
Eletreto     │
     │         │
    GND       GND
```

**Importante**: O sinal de áudio deve estar centrado em aproximadamente 1.65V (metade da tensão de referência do ADC) para correta leitura.

---

## Instalação das Bibliotecas

### No Arduino IDE 2.0

1. Abra o Arduino IDE
2. Vá em **Ferramentas** → **Gerenciar Bibliotecas**
3. Instale as seguintes bibliotecas:

#### LedControl
- Nome: `LedControl`
- Autor: Eberhard Fahle
- Finalidade: Controlar displays MAX7219

#### ESP32 Arduino Core
- Vá em **Arquivo** → **Preferências**
- Adicione em "URLs adicionais de placas":
  ```
  https://espressif.github.io/arduino-esp32/package_esp32_index.json
  ```
- Vá em **Ferramentas** → **Placa** → **Gerenciador de Placas**
- Procure por "ESP32" e instale o core da Espressif

#### ESP-DSP
- Nome: `ESP-DSP`
- Autor: Espressif Systems
- Finalidade: Biblioteca otimizada de processamento digital de sinais para ESP32

---

## Configuração da Placa no Arduino IDE

1. **Selecione a placa**: Ferramentas → Placa → ESP32S3 Dev Module ou ESP32C3 Dev Module
2. **Configure as opções**:
   - CPU Frequency: 160 MHz
   - Upload Speed: 921600
   - Flash Size: 4MB
   - Partition Scheme: Default 4MB with spiffs

---

## Estrutura do Código

### Seções Principais

1. **Configurações e Definições** (linhas 1-70)
   - Pinagem do hardware (ESP32-C3)
   - Parâmetros de áudio e FFT
   - Constantes do sistema

2. **Variáveis Globais** (linhas 71-95)
   - Buffers de áudio
   - Estado do espectro (8 bandas × 32 níveis)
   - Controles de timing e scroll

3. **Funções de Setup** (linhas 96-180)
   - `setupDisplay()`: Inicializa o MAX7219
   - `setupADC()`: Configura o conversor A/D do ESP32
   - `setupFFT()`: Prepara a biblioteca esp_dsp
   - `setup()`: Função principal de inicialização

4. **Processamento de Áudio** (linhas 181-320)
   - `readADC()`: Lê amostras do microfone
   - `collectAudioSamples()`: Coleta buffer de áudio
   - `applyHammingWindow()`: Aplica janela para precisão
   - `calculateFFT()`: Calcula transformada de Fourier
   - `mapToBands()`: Converte FFT em 8 bandas verticais
   - `updatePeaks()`: Gerencia indicadores de pico

5. **Exibição no Display - Modo Vertical** (linhas 321-450)
   - `clearDisplay()`: Limpa todos os LEDs
   - `drawVerticalBar()`: Desenha barra vertical
   - `updateScroll()`: Gerencia scroll automático
   - `updateDisplay()`: Atualiza todo o espectro vertical
   - `showStartupAnimation()`: Animação de boot

6. **Loop Principal** (linhas 451-480)
   - Executa ciclo contínuo de aquisição e exibição

7. **Funções de Debug** (linhas 481-550)
   - `printSpectrum()`: Imprime dados no Serial
   - `testAllLEDs()`: Teste de hardware

---

## Como Funciona o Processamento de Áudio

### 1. Amostragem (Sampling)

O sinal analógico do microfone é convertido em digital pelo ADC:
- Taxa: 8.000 amostras por segundo
- Resolução: 12 bits (0-4095)
- Buffer: 256 amostras

### 2. Preparação do Sinal

Antes da FFT, aplicamos:
- **Remoção de DC**: Centraliza o sinal em zero
- **Janela de Hamming**: Reduz vazamento espectral

### 3. Transformada FFT

A FFT converte o sinal do domínio do tempo para o domínio da frequência:
- Entrada: 256 amostras no tempo
- Saída: 128 bins de frequência
- Cada bin representa uma faixa específica

### 4. Mapeamento para 8 Bandas

As 128 saídas da FFT são agrupadas em 8 bandas visuais:
- Distribuição logarítmica (mais bandas em frequências baixas)
- Faixa: 100 Hz a 4.000 Hz
- Cada banda = média de múltiplos bins da FFT
- Cada banda pode ter altura de 0 a 32 níveis

### 5. Scroll Automático

Como o display tem apenas 8 linhas físicas mas suportamos 32 níveis:
- O scroll ajusta automaticamente a região visível
- Segue o topo das barras mais altas
- Retorna à base quando o sinal está baixo

### 6. Exibição

Cada banda controla 4 colunas do display:
- Largura da barra = 4 colunas (32 colunas / 8 bandas)
- Altura da barra = intensidade da frequência (0-32)
- Indicador de pico = valor máximo recente
- Atualização contínua em tempo real

---

## Ajustes e Calibração

### Sensibilidade do Áudio

No código, ajuste a constante `SENSITIVITY`:

```cpp
#define SENSITIVITY     3.0f  // Aumente para mais sensibilidade
```

- Valores maiores (ex: 4.0-6.0): Mais sensível, reage a sons fracos
- Valores menores (ex: 1.0-2.0): Menos sensível, requer sons fortes

### Brilho do Display

Na função `setupDisplay()`:

```cpp
lc.setIntensity(i, 10);  // Valores de 0 a 15
```

- 0-5: Brilho baixo (ambiente escuro)
- 6-10: Brilho médio (recomendado)
- 11-15: Brilho máximo (pode ofuscar)

### Faixa de Frequências

Para alterar as frequências mínimas e máximas:

```cpp
#define MIN_FREQ        100   // Frequência mínima em Hz
#define MAX_FREQ        4000  // Frequência máxima em Hz
```

Exemplos:
- Foco em graves: 50 Hz - 500 Hz
- Foco em agudos: 1000 Hz - 8000 Hz
- Full range: 100 Hz - 8000 Hz (limitado pelo Nyquist)

### Decay dos Picoss

No código `updatePeaks()`, ajuste o tempo:

```cpp
#define PEAK_DECAY_MS   100   // 100ms
```

- 50-100ms: Decaimento rápido
- 100-200ms: Decaimento moderado (padrão)
- 200-500ms: Decaimento lento

### Velocidade do Scroll

Para ajustar a velocidade do scroll vertical:

```cpp
#define SCROLL_SPEED    50    // 50ms por passo
```

- 20-30ms: Scroll rápido e suave
- 50-80ms: Scroll moderado (padrão)
- 100-200ms: Scroll lento

---

## Testes e Validação

### Teste 1: Verificação do Display

Execute a função `testAllLEDs()` no setup para verificar se todos os LEDs funcionam:

```cpp
void setup() {
  // ... configurações ...
  testAllLEDs();  // Descomente para teste
}
```

### Teste 2: Monitor Serial

Ative a função `printSpectrum()` para visualizar dados brutos:

```cpp
void loop() {
  // ... processamento ...
  printSpectrum();  // Descomente para debug
}
```

Isso mostrará:
- Valor atual do scroll
- Valores das 8 bandas
- Gráfico ASCII vertical das bandas visíveis

### Teste 3: Resposta a Frequências Conhecidas

Use um gerador de tons (app de smartphone) para testar:
- 100-200 Hz: Deve acender as primeiras bandas (graves)
- 500-1000 Hz: Deve acender bandas do meio (médios)
- 3000-4000 Hz: Deve acender as últimas bandas (agudos)

---

## Solução de Problemas

### Display não acende

1. Verifique conexões VCC e GND
2. Confirme pinos DIN, CLK, CS corretos (GPIO 7, 6, 10)
3. Teste tensão nos pinos do MAX7219
4. Execute `testAllLEDs()` para diagnóstico

### Leitura do microfone incorreta

1. Verifique polarização do microfone
2. Meça tensão no pino ADC (deve ser ~1.65V sem som)
3. Ajuste ganho do pré-amplificador
4. Verifique se o ADC está no pino correto (GPIO 0)
5. **Importante**: GPIO 0 deve estar livre durante o boot

### FFT não processa corretamente

1. Confirme que as bibliotecas estão instaladas (LedControl, ESP-DSP)
2. Verifique se SAMPLE_RATE é adequado
3. Certifique-se de que FFT_SIZE é potência de 2
4. Monitore valores no Serial Monitor

### Performance lenta

1. Reduza SAMPLE_RATE (ex: 4000 Hz)
2. Diminua FFT_SIZE (ex: 128 pontos)
3. Aumente clock do ESP32 para 240 MHz (se suportado)

### Scroll não funciona

1. Verifique se as bandas estão recebendo valores > 8
2. Aumente SENSITIVITY para testar
3. Monitore scrollOffset no Serial Monitor

---

## Expansões Possíveis

### 1. Adicionar Botões de Controle

Implemente botões para ajustar:
- Sensibilidade em tempo real
- Brilho do display
- Faixa de frequências
- Velocidade do scroll

### 2. Modo Estéreo

Use dois microfones para criar:
- Visualizador estéreo (L/R separados)
- Análise de fase entre canais

### 3. Efeitos Visuais

Adicione modos de exibição:
- Modo picos (apenas indicadores)
- Modo gradiente (padrões diferentes por altura)
- Modo animação (ondas, partículas)

### 4. Saída de Dados

Implemente comunicação:
- USB Serial para PC
- Bluetooth LE (nativo do ESP32-C3)
- WiFi para rede local

### 5. Gravação de Dados

Adicione armazenamento:
- Cartão SD para logging
- Memória Flash interna
- Exportação via USB

---

## Referências Técnicas

### Teoria

- **FFT (Fast Fourier Transform)**: Algoritmo eficiente para análise espectral
- **Teorema de Nyquist**: Frequência máxima = Sample Rate / 2
- **Janela de Hamming**: Minimiza vazamento espectral
- **Escala Logarítmica**: Melhor representação da percepção humana

### Datasheets

- [ESP32-C3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [MAX7219 Datasheet](https://cdn.sparkfun.com/datasheets/Components/General/MAX7219.pdf)

### Bibliotecas

- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [ESP-DSP Library](https://github.com/espressif/esp-dsp)
- [LedControl Library](https://github.com/wayoda/LedControl)

---

## Créditos

**Projeto Didático** - 2024

Desenvolvido para fins educacionais, demonstrando:
- Processamento digital de sinais
- Uso de FFT em microcontroladores
- Interface com hardware externo
- Programação embarcada em C/C++
- Técnicas de visualização com scroll

---

## Licença

Este projeto é fornecido para fins educacionais. Sinta-se livre para modificar, distribuir e utilizar em seus próprios projetos.

---

## Suporte

Para dúvidas e sugestões:
1. Consulte os comentários no código fonte
2. Verifique o Serial Monitor para mensagens de debug
3. Teste cada componente individualmente
4. Consulte fóruns especializados em ESP32 e Arduino

**Boa aprendizagem!** 🎵📊⬆️
