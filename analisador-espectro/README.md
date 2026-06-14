# Analisador de Espectro de Áudio

## Visão Geral do Projeto

Este projeto implementa um analisador de espectro de áudio em tempo real utilizando:
- **Raspberry Pi Pico** como microcontrolador principal
- **Display Matriz LED 8x32** com driver MAX7219 para visualização
- **Microfone de Eletreto** para captura de áudio

O sistema analisa o sinal de áudio em tempo real e exibe 32 bandas de frequência com até 8 níveis de altura no display LED.

---

## Lista de Materiais

### Componentes Principais

| Item | Quantidade | Descrição |
|------|------------|-----------|
| Raspberry Pi Pico | 1 | Microcontrolador com RP2040 |
| Display MAX7219 8x32 | 1 | Matriz de LED (4 módulos 8x8 em cascata) |
| Microfone de Eletreto | 1 | Com módulo pré-amplificador |
| Jumpers | Vários | Para conexões |
| Protoboard | 1 | Para montagem do circuito |

### Especificações Técnicas

- **Microcontrolador**: Raspberry Pi Pico (RP2040, dual-core ARM Cortex-M0+)
- **ADC**: 12 bits, até 500 kS/s
- **Display**: 32 colunas x 8 linhas (256 LEDs)
- **Bandas de Frequência**: 32 bandas distribuídas de 50Hz a 5kHz
- **Taxa de Amostragem**: 10 kHz
- **FFT Size**: 256 pontos

---

## Diagrama de Conexões

### Display MAX7219 → Raspberry Pi Pico

```
MAX7219          Pico
───────          ────
VCC         →    5V (ou 3.3V)*
GND         →    GND
DIN         →    GPIO 19 (MOSI)
CS          →    GPIO 17
CLK         →    GPIO 18
```

*Verifique a tensão de operação do seu módulo MAX7219

### Microfone → Raspberry Pi Pico

```
Microfone      Pico
─────────      ────
VCC        →   3.3V (ou VSYS)**
GND        →   GND
OUT        →   GPIO 26 (ADC0)
```

**Se usar módulo com amplificador operacional, verifique a tensão adequada

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
          ├────→ GPIO 26 (ADC)
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

#### Pico SDK / Arduino-Pico
- Certifique-se de ter o core do Raspberry Pi Pico instalado
- Vá em **Arquivo** → **Preferências**
- Adicione em "URLs adicionais de placas":
  ```
  https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
  ```
- Vá em **Ferramentas** → **Placa** → **Gerenciador de Placas**
- Procure por "Raspberry Pi Pico" e instale

#### CMSIS-DSP
- Já incluída no core do Arduino-Pico
- Fornece funções otimizadas de FFT para ARM

---

## Configuração da Placa no Arduino IDE

1. **Selecione a placa**: Ferramentas → Placa → Raspberry Pi Pico
2. **Configure as opções**:
   - CPU Speed: 125 MHz ou 250 MHz
   - Upload Method: Default (UF2) ou Picotool
   - Debug Port: Disabled (para maior performance)
   - USB Stack: Pico SDK
   - Variant: Raspberry Pi Pico

---

## Estrutura do Código

### Seções Principais

1. **Configurações e Definições** (linhas 1-70)
   - Pinagem do hardware
   - Parâmetros de áudio e FFT
   - Constantes do sistema

2. **Variáveis Globais** (linhas 71-90)
   - Buffers de áudio
   - Estado do espectro
   - Controles de timing

3. **Funções de Setup** (linhas 91-180)
   - `setupDisplay()`: Inicializa o MAX7219
   - `setupADC()`: Configura o conversor A/D
   - `setupFFT()`: Prepara a biblioteca FFT
   - `setup()`: Função principal de inicialização

4. **Processamento de Áudio** (linhas 181-320)
   - `readADC()`: Lê amostras do microfone
   - `collectAudioSamples()`: Coleta buffer de áudio
   - `applyHammingWindow()`: Aplica janela para precisão
   - `calculateFFT()`: Calcula transformada de Fourier
   - `mapToBands()`: Converte FFT em bandas visuais
   - `updatePeaks()`: Gerencia indicadores de pico

5. **Exibição no Display** (linhas 321-420)
   - `clearDisplay()`: Limpa todos os LEDs
   - `drawBar()`: Desenha barra vertical
   - `updateDisplay()`: Atualiza todo o espectro
   - `showStartupAnimation()`: Animação de boot

6. **Loop Principal** (linhas 421-450)
   - Executa ciclo contínuo de aquisição e exibição

7. **Funções de Debug** (linhas 451-520)
   - `printSpectrum()`: Imprime dados no Serial
   - `testAllLEDs()`: Teste de hardware

---

## Como Funciona o Processamento de Áudio

### 1. Amostragem (Sampling)

O sinal analógico do microfone é convertido em digital pelo ADC:
- Taxa: 10.000 amostras por segundo
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

### 4. Mapeamento para Bandas

As 128 saídas da FFT são agrupadas em 32 bandas visuais:
- Distribuição logarítmica (mais bandas em frequências baixas)
- Faixa: 50 Hz a 5.000 Hz
- Cada banda = média de múltiplos bins da FFT

### 5. Exibição

Cada banda controla uma coluna do display:
- Altura da barra = intensidade da frequência
- Indicador de pico = valor máximo recente
- Atualização contínua em tempo real

---

## Ajustes e Calibração

### Sensibilidade do Áudio

No código, ajuste a constante `SENSITIVITY`:

```cpp
#define SENSITIVITY   2.0f  // Aumente para mais sensibilidade
```

- Valores maiores (ex: 3.0-5.0): Mais sensível, reage a sons fracos
- Valores menores (ex: 0.5-1.0): Menos sensível, requer sons fortes

### Brilho do Display

Na função `setupDisplay()`:

```cpp
lc.setIntensity(i, 8);  // Valores de 0 a 15
```

- 0-5: Brilho baixo (ambiente escuro)
- 6-10: Brilho médio (recomendado)
- 11-15: Brilho máximo (pode ofuscar)

### Faixa de Frequências

Para alterar as frequências mínimas e máximas:

```cpp
#define MIN_FREQ      50    // Frequência mínima em Hz
#define MAX_FREQ      5000  // Frequência máxima em Hz
```

Exemplos:
- Foco em graves: 20 Hz - 500 Hz
- Foco em agudos: 1000 Hz - 10000 Hz
- Full range: 20 Hz - 20000 Hz (limitado pelo Nyquist)

### Decay dos Picos

No código `updatePeaks()`, ajuste o tempo:

```cpp
if (currentTime - peakTimers[i] > 500) {  // 500ms
```

- 200-300ms: Decaimento rápido
- 500-800ms: Decaimento moderado (padrão)
- 1000-2000ms: Decaimento lento

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

### Teste 3: Resposta a Frequências Conhecidas

Use um gerador de tons (app de smartphone) para testar:
- 100 Hz: Deve acender as primeiras bandas
- 1000 Hz: Deve acender bandas do meio
- 4000 Hz: Deve acender as últimas bandas

---

## Solução de Problemas

### Display não acende

1. Verifique conexões VCC e GND
2. Confirme pinos DIN, CLK, CS corretos
3. Teste tensão nos pinos do MAX7219
4. Execute `testAllLEDs()` para diagnóstico

### Leitura do microfone incorreta

1. Verifique polarização do microfone
2. Meça tensão no pino ADC (deve ser ~1.65V sem som)
3. Ajuste ganho do pré-amplificador
4. Verifique se o ADC está no pino correto (GPIO 26)

### FFT não processa corretamente

1. Confirme que as bibliotecas estão instaladas
2. Verifique se SAMPLE_RATE é adequado
3. Certifique-se de que FFT_SIZE é potência de 2
4. Monitore valores no Serial Monitor

### Performance lenta

1. Reduza SAMPLE_RATE (ex: 8000 Hz)
2. Diminua FFT_SIZE (ex: 128 pontos)
3. Reduza NUM_BANDS (ex: 16 bandas)
4. Aumente clock do Pico para 250 MHz

---

## Expansões Possíveis

### 1. Adicionar Botões de Controle

Implemente botões para ajustar:
- Sensibilidade em tempo real
- Brilho do display
- Faixa de frequências

### 2. Modo Estéreo

Use dois microfones para criar:
- Visualizador estéreo (L/R separados)
- Análise de fase entre canais

### 3. Efeitos Visuais

Adicione modos de exibição:
- Modo picos (apenas indicadores)
- Modo gradiente (cores diferentes por altura)
- Modo animação (ondas, partículas)

### 4. Saída de Dados

Implemente comunicação:
- USB Serial para PC
- Bluetooth para smartphone
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

- [Raspberry Pi Pico Datasheet](https://datasheets.raspberrypi.com/pico/Pico-R3-A4-Public.pdf)
- [MAX7219 Datasheet](https://cdn.sparkfun.com/datasheets/Components/General/MAX7219.pdf)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)

### Bibliotecas

- [Arduino-Pico Core](https://github.com/earlephilhower/arduino-pico)
- [CMSIS-DSP Documentation](https://arm-software.github.io/CMSIS_5/DSP/html/index.html)
- [LedControl Library](https://github.com/wayoda/LedControl)

---

## Créditos

**Projeto Didático** - 2024

Desenvolvido para fins educacionais, demonstrando:
- Processamento digital de sinais
- Uso de FFT em microcontroladores
- Interface com hardware externo
- Programação embarcada em C/C++

---

## Licença

Este projeto é fornecido para fins educacionais. Sinta-se livre para modificar, distribuir e utilizar em seus próprios projetos.

---

## Suporte

Para dúvidas e sugestões:
1. Consulte os comentários no código fonte
2. Verifique o Serial Monitor para mensagens de debug
3. Teste cada componente individualmente
4. Consulte fóruns especializados em Arduino e Raspberry Pi

**Boa aprendizagem!** 🎵📊
