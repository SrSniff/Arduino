# Guia Rápido - Analisador de Espectro Vertical

## 🚀 Primeiros Passos

### 1. Instalar Dependências

No Arduino IDE 2.0:

```
Ferramentas → Gerenciar Bibliotecas → Buscar "LedControl" → Instalar
Ferramentas → Gerenciar Bibliotecas → Buscar "ESP-DSP" → Instalar
```

Para o ESP32-C3:
```
Arquivo → Preferências → URLs Adicionais:
https://espressif.github.io/arduino-esp32/package_esp32_index.json

Ferramentas → Placa → Gerenciador → Buscar "ESP32" → Instalar
```

### 2. Conectar Hardware

**Display MAX7219:**
- VCC → 5V (ou 3.3V)
- GND → GND  
- DIN → GPIO 7
- CLK → GPIO 6
- CS  → GPIO 10

**Microfone:**
- VCC → 3.3V
- GND → GND
- OUT → GPIO 0

⚠️ **Atenção**: O GPIO 0 deve estar livre durante o boot do ESP32!

### 3. Configurar Arduino IDE

```
Ferramentas → Placa → ESP32S3 Dev Module ou ESP32C3 Dev Module
Ferramentas → CPU Frequency → 160 MHz
Ferramentas → Upload Speed → 921600
Ferramentas → Port → Selecionar porta COM do ESP32
```

### 4. Carregar Código

1. Abra `analisador_espectro_vertical.ino`
2. Clique em **Upload** (seta →)
3. Segure BOOT no ESP32 se necessário
4. Aguarde compilação e upload

---

## ⚙️ Ajustes Rápidos

### Se não reagir ao som:
```cpp
#define SENSITIVITY   5.0f  // Aumente de 3.0 para 5.0 ou mais
```

### Se display estiver muito escuro:
```cpp
lc.setIntensity(i, 12);  // Mude de 10 para 12 (máx: 15)
```

### Para faixas de frequência diferentes:
```cpp
#define MIN_FREQ        50    // Reduza para mais graves
#define MAX_FREQ        8000  // Aumente para mais agudos
```

### Para scroll mais rápido:
```cpp
#define SCROLL_SPEED    30    // Reduza de 50 para 30
```

### Para picos mais lentos:
```cpp
#define PEAK_DECAY_MS   200   // Aumente de 100 para 200
```

---

## 🐛 Problemas Comuns

| Problema | Solução |
|----------|---------|
| Display piscando | Verifique conexão de alimentação |
| Sem resposta ao som | Aumente SENSITIVITY |
| LEDs aleatórios | Verifique pinos DIN/CLK/CS |
| Compilação falha | Instale bibliotecas LedControl e ESP-DSP |
| Upload falha | Segure BOOT enquanto conecta USB |
| GPIO 0 conflitante | Desconecte microfone durante boot |

---

## 📊 Entendendo o Resultado

O display mostra **8 bandas verticais**:

```
Colunas 0-3:    Banda 0 (Graves ~100-200 Hz)
Colunas 4-7:    Banda 1 (Graves-Médios ~200-400 Hz)
Colunas 8-11:   Banda 2 (Médios-Graves ~400-800 Hz)
Colunas 12-15:  Banda 3 (Médios ~800-1500 Hz)
Colunas 16-19:  Banda 4 (Médios-Agudos ~1500-2500 Hz)
Colunas 20-23:  Banda 5 (Agudos ~2500-3200 Hz)
Colunas 24-27:  Banda 6 (Agudos Altos ~3200-3800 Hz)
Colunas 28-31:  Banda 7 (Agudos Máximos ~3800-4000 Hz)
```

**Altura da barra** = intensidade do som naquela frequência (0-32 níveis)

**Scroll automático** = ajusta a visualização para mostrar picos altos

**Linha separada** = pico recente (decai lentamente)

---

## 🔧 Debug

Para ver dados no Serial Monitor (115200 baud):

```cpp
// No final do loop(), descomente:
printSpectrum();
```

Isso mostrará:
- Valor atual do scroll (0-24)
- Valores das 8 bandas (0-32 cada)
- Gráfico ASCII vertical das linhas visíveis

Exemplo de saída:
```
Scroll: 5 | Bandas: [12, 8, 15, 20, 18, 10, 6, 4]
27|    ####                            
26|    ####                            
25|    ####        ####                
24|    ####        ####                
23|    ####        ####                
22|    ####        ####                
21|    ####        ####                
20|    ####        ####    ####        
+--------------------------------
 Bandas: 0    1    2    3    4    5    6    7
 Freq:  Graves -> Médios -> Agudos
```

---

## 💡 Dicas Didáticas

1. **Teste com tons conhecidos**: Use app gerador de frequências no smartphone
2. **Observe diferentes sons**: Voz, música, assobio, palmas
3. **Ajuste em tempo real**: Mude SENSITIVITY e faça upload
4. **Monitore via Serial**: Veja valores numéricos e scroll
5. **Teste o scroll**: Gere sons fortes para ver o scroll subir

---

## 📚 Conceitos Ensinados

- ✅ Conversão Analógico-Digital (ADC) no ESP32
- ✅ Transformada de Fourier (FFT)
- ✅ Processamento Digital de Sinais (DSP)
- ✅ Comunicação SPI (MAX7219)
- ✅ Tempo Real em Sistemas Embarcados
- ✅ Filtragem e Janelamento de Sinais
- ✅ Técnicas de Scroll em Displays Limitados
- ✅ Visualização de Dados em Tempo Real

---

## 🆚 Comparação: Horizontal vs Vertical

| Característica | Horizontal | Vertical |
|----------------|------------|----------|
| Hardware | Raspberry Pi Pico | ESP32-C3-Zero |
| Bandas | 32 | 8 |
| Altura | 8 níveis fixos | 32 níveis com scroll |
| Visualização | Frequência nas colunas | Amplitude vertical |
| Melhor para | Análise espectral detalhada | Dinâmica de intensidade |

---

**Próximos passos**: Consulte o `README.md` para detalhes completos!
