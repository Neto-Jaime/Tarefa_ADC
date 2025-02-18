# Controle de Joystick com Raspberry Pi Pico W

## 📌 Descrição  
Este projeto utiliza um **Raspberry Pi Pico W** para ler os valores de um **joystick analógico** e controlar **LEDs RGB** via **PWM**, além de exibir as coordenadas no **display OLED** via **I2C**.  

Os LEDs ajustam seu brilho conforme a posição do **joystick** e o **botão do joystick** altera o estado das bordas no **display OLED**.  

---

## 📝 Informações do Autor  

- **Nome:** Jaime Neto  
- **Grupo:** 0  
- **Matrícula:** TIC370100555  

---

## 🛠️ Componentes Utilizados  

- 🖥 **Raspberry Pi Pico W**  
- 🎮 **Joystick analógico** (Eixos X e Y + botão)  
- 🌈 **LED RGB (3 canais PWM - Vermelho, Verde e Azul)**  
- 📟 **Display OLED SSD1306 (I2C)**  
- 🛠 **Resistores e Jumpers**  

---

## 🔧 Configuração dos Pinos  

### **Pinos do LED RGB**  
| Cor do LED | Pino GPIO |
|------------|----------|
| Verde      | 11       |
| Azul       | 12       |
| Vermelho   | 13       |

### **Pinos do Joystick**  
| Função | Pino GPIO |
|--------|----------|
| Eixo X | 27       |
| Eixo Y | 26       |
| Botão  | 22       |

### **Pinos do Display OLED**  
| Função | Pino GPIO |
|--------|----------|
| SDA    | 14       |
| SCL    | 15       |

---

## 🖥️ Bibliotecas Utilizadas  

Este código faz uso das seguintes bibliotecas do **Raspberry Pi Pico SDK**:  

- **hardware/i2c.h** → Comunicação I2C com o display OLED  
- **hardware/adc.h** → Leitura do joystick via ADC  
- **hardware/pwm.h** → Controle de brilho dos LEDs via PWM  
- **hardware/gpio.h** → Manipulação dos botões  
- **hardware/timer.h** → Controle de tempo e debounce  

Além disso, utiliza as bibliotecas externas:  
- **ssd1306.h** → Controla o display OLED  
- **font.h** → Gerencia fontes para exibição de texto  

---

## ⚙️ Funcionamento  

### **1️⃣ Leitura do Joystick**  
O Raspberry Pi Pico lê os valores analógicos dos **eixos X e Y** usando o **ADC** e interpreta os dados para controlar os LEDs e o display.  

### **2️⃣ Controle dos LEDs RGB via PWM**  
- O brilho do **LED vermelho** é baseado na posição do **eixo X**  
- O brilho do **LED azul** é baseado na posição do **eixo Y**  
- O **LED verde** é acionado ao pressionar o **botão do joystick**  

### **3️⃣ Exibição no Display OLED**  
- O quadrado se move de acordo com os valores lidos do joystick  
- O botão do joystick ativa/desativa **bordas na tela**  

### **4️⃣ Botões de Controle**  
- O **botão A (GPIO 5)** ativa/desativa os LEDs RGB  
- O **botão do joystick (GPIO 22)** alterna os estilos de borda do display  

---

## ▶️ Como Usar  

1. **Monte o circuito** conforme o esquema de pinos acima.  
2. **Compile e carregue** o código para o **Raspberry Pi Pico W** usando o SDK do Pico.  
3. **Conecte o Pico ao PC** e abra um monitor serial (exemplo: `minicom`, `putty` ou `screen`).  
4. **Movimente o joystick** para ver os LEDs mudando de brilho e o quadrado se movendo no **display OLED**.  
5. **Pressione os botões** para interagir com os LEDs e bordas do display.  
