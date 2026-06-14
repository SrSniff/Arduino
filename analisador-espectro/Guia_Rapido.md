# Guia Rápido - Analisador de Espectro

## 🚀 Primeiros Passos

### 1. Instalar Dependências

No Arduino IDE 2.0:

```
Ferramentas → Gerenciar Bibliotecas → Buscar "LedControl" → Instalar
```

Para o Raspberry Pi Pico:
```
Arquivo → Preferências → URLs Adicionais:
https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

Ferramentas → Placa → Gerenciador → Buscar "Raspberry Pi Pico" → Instalar
```

### 2. Conectar Hardware

**Display MAX7219:**
- VCC → 5V
- GND → GND  
- DIN → GPIO 19
- CLK → GPIO 18
- CS  → GPIO 17

**Microfone:**
- VCC → 3.3V
- GND → GND
- OUT → GPIO 26

### 3. Configurar Arduino IDE

```
Ferramentas → Placa → Raspberry Pi Pico
Ferramentas → CPU Speed → 250 MHz (opcional, mais rápido)
Ferramentas → Port → Selecionar porta COM do Pico
```

### 4. Carregar Código

1. Abra `analisador_espectro.ino`
2. Clique em **Upload** (seta →)
3. Aguarde compilação e upload

---

## ⚙️ Ajustes Rápidos

### Se não reagir ao som:
```cpp
#define SENSITIVITY   3.0f  // Aumente de 2.0 para 3.0 ou mais
```

### Se display estiver muito escuro:
```cpp
lc.setIntensity(i, 12);  // Mude de 8 para 12 (máx: 15)
```

### Para mais bandas (menos resolução):
```cpp
#define NUM_BANDS     16    // Reduza de 32 para 16
```

### Para menos bandas (mais detalhe):
```cpp
#define NUM_BANDS     64    // Aumente para 64 (requer display maior)
```

---

## 🐛 Problemas Comuns

| Problema | Solução |
|----------|---------|
| Display piscando | Verifique conexão de alimentação |
| Sem resposta ao som | Aumente SENSITIVITY |
| LEDs aleatórios | Verifique pinos DIN/CLK/CS |
| Compilação falha | Instale biblioteca LedControl |
| Upload falha | Segure BOOTSEL enquanto conecta USB |

---

## 📊 Entendendo o Resultado

Cada coluna = uma faixa de frequência
- **Esquerda**: Graves (50-200 Hz)
- **Centro**: Médios (200-2000 Hz)  
- **Direita**: Agudos (2000-5000 Hz)

Altura da barra = intensidade do som naquela frequência

Línea separada = pico recente (decai lentamente)

---

## 🔧 Debug

Para ver dados no Serial Monitor (115200 baud):

```cpp
// No final do loop(), descomente:
printSpectrum();
```

Isso mostrará um gráfico ASCII das bandas.

---

## 💡 Dicas Didáticas

1. **Teste com tons conhecidos**: Use app gerador de frequências
2. **Observe diferentes sons**: Voz, música, assobio
3. **Ajuste em tempo real**: Mude SENSITIVITY e faça upload
4. **Monitore via Serial**: Veja valores numéricos das bandas

---

## 📚 Conceitos Ensinados

- ✅ Conversão Analógico-Digital (ADC)
- ✅ Transformada de Fourier (FFT)
- ✅ Processamento Digital de Sinais
- ✅ Comunicação SPI (MAX7219)
- ✅ Tempo Real em Sistemas Embarcados
- ✅ Filtragem e Janelamento de Sinais

---

**Próximos passos**: Consulte o `README.md` para detalhes completos!
