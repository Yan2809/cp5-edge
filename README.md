# 🌡️ Projeto IoT com ESP32 — Monitoramento de Temperatura, Umidade e Luminosidade via MQTT (FIWARE)

## 📘 Descrição
Este projeto utiliza um **ESP32** conectado a sensores **DHT22** e **LDR** para monitorar **temperatura, umidade e luminosidade** em tempo real.  
Os dados são enviados via **protocolo MQTT** para o **broker público `test.mosquitto.org`**, seguindo o **padrão NGSIv2 do FIWARE**, o que permite integração futura com **Orion Context Broker** e **dashboards inteligentes**.

---

## 🧠 Objetivos
- Coletar dados ambientais (temperatura, umidade e luminosidade).  
- Transmitir as informações a um **broker MQTT**.  
- Seguir o padrão **NGSIv2** do ecossistema **FIWARE**.  
- Facilitar a criação de **dashboards individuais** para cada sensor.

---

## ⚙️ Componentes Utilizados

| Componente | Quantidade | Função |
|-------------|-------------|--------|
| ESP32 DevKit | 1 | Microcontrolador principal |
| Sensor DHT22 | 1 | Mede temperatura e umidade |
| Sensor LDR (fotoresistor) | 1 | Mede luminosidade ambiente |
| Resistor 10kΩ | 1 | Pull-up do DHT22 |
| Breadboard + Jumpers | — | Montagem dos circuitos |

---

## 🔌 Esquema de Ligação

| Componente | Pino ESP32 | Observação |
|-------------|-------------|-------------|
| **DHT22 - VCC** | 3V3 | Alimentação |
| **DHT22 - DATA** | GPIO 15 | Leitura digital |
| **DHT22 - GND** | GND | Terra |
| **LDR - VCC** | 3V3 | Alimentação |
| **LDR - GND** | GND | Terra |
| **LDR - OUT (analógico)** | GPIO 34 | Entrada analógica |

🖼️ **Diagrama de Montagem (Wokwi / Fritzing):**  
![Circuito ESP32 com DHT22 e LDR](./image.png)
---

## 🛰️ Fluxo de Dados IoT

- [DHT22 + LDR] → [ESP32] → [MQTT Broker (test.mosquitto.org)] → [FIWARE Orion / Dashboard]
- O ESP32 coleta os dados dos sensores.  
- Gera um **timestamp NTP** com horário de Brasília.  
- Envia os dados em formato **JSON** para o tópico MQTT.  
- O FIWARE (ou qualquer dashboard MQTT) pode consumir esses dados para exibição e análise.

---

## 🧩 Estrutura JSON Enviada (Padrão NGSIv2)

```json
{
  "timestamp": "2025-10-20 21:15:00",
  "temperatura": 26.5,
  "umidade": 58.2,
  "luminosidade": 47.8
}
```

---

## 📡 Tópicos MQTT

| Tópico | Conteúdo Publicado |
|--------|--------------------|
| `/TEF/device001/attrs` | JSON completo (todas as medições) |
| `/TEF/device001/attrs/timestamp` | Horário atual (NTP) |
| `/TEF/device001/attrs/temperatura` | Temperatura (°C) |
| `/TEF/device001/attrs/umidade` | Umidade (%) |
| `/TEF/device001/attrs/luminosidade` | Luminosidade (%) |

---

## 🧠 Configuração do Ambiente

1. **Plataforma:** Arduino IDE ou PlatformIO  
2. **Bibliotecas Necessárias:**
   - `WiFi.h`
   - `PubSubClient.h`
   - `Adafruit_Sensor.h`
   - `DHT.h`
   - `DHT_U.h`
3. **Broker MQTT:** `test.mosquitto.org` (porta `1883`)  
4. **Rede Wi-Fi:**  
   - SSID: `Wokwi-GUEST`  
   - Senha: *(vazia)*  

---

## 🧾 Testes e Validação

- Testado com o aplicativo **MyMQTT (Android)** e o **MQTT Explorer (Desktop)**.  
- Dados publicados corretamente no broker a cada **4 segundos**.  
- Timestamp sincronizado com o **horário de Brasília (GMT-3)**.  
- Leituras estáveis e coerentes com as condições do ambiente.

🖼️ *Print do teste real:*  
![Publicação MQTT](./mqtt_teste.png)

---

## 🚀 Próximos Passos

- Criar **dashboards separados**:
  - 🌡️ Temperatura  
  - 💧 Umidade  
  - 💡 Luminosidade  
- Integrar com o **FIWARE Orion Context Broker**.  
- Armazenar histórico no **STH-Comet (MongoDB)**.  
- Implementar **alertas automáticos** (ex: alta temperatura ou baixa luminosidade).

---

## 👨‍💻 Autor

**Yan Barutti**  
FIAP — Engenharia de Software  
Projeto IoT com ESP32 e FIWARE  
📅 2025
