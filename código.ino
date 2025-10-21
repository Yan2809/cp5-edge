#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "time.h" // Para timestamp NTP

// === CONFIGURAÇÕES EDITÁVEIS ===
const char* SSID = "Wokwi-GUEST";                  // Rede Wi-Fi (2.4GHz)
const char* PASSWORD = "";                         // Senha Wi-Fi
const char* BROKER_MQTT = "test.mosquitto.org";    // IP do Broker MQTT
const int BROKER_PORT = 1883;                      // Porta do Broker MQTT

// === PINOS ===
#define LDR_PIN 34       // Pino analógico do LDR
#define DHTPIN 15        // Pino do DHT22
#define DHTTYPE DHT22    // Tipo do sensor DHT

// === TÓPICO BASE NGSI (FIWARE) ===
const char* TOPICO_BASE = "/TEF/device001/attrs";  // Envia tudo junto
const char* TOPICO_TIMESTAMP = "/TEF/device001/attrs/timestamp";
const char* TOPICO_LUZ = "/TEF/device001/attrs/luminosidade";
const char* TOPICO_TEMP = "/TEF/device001/attrs/temperatura";
const char* TOPICO_UMID = "/TEF/device001/attrs/umidade";
const char* ID_MQTT = "fiware_001";

// === OBJETOS ===
WiFiClient espClient;
PubSubClient MQTT(espClient);
DHT dht(DHTPIN, DHTTYPE);

// === CONFIGURAÇÃO NTP (para timestamp) ===
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600;  // Horário de Brasília
const int daylightOffset_sec = 0;

// === FUNÇÕES ===
void initSerial() {
  Serial.begin(115200);
  Serial.println("Iniciando ESP32...");
}

void initWiFi() {
  Serial.print("Conectando-se à rede: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Wi-Fi conectado! IP: ");
  Serial.println(WiFi.localIP());
}

void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Tentando conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao Broker MQTT!");
    } else {
      Serial.println("Falha na conexão. Tentando novamente em 2s...");
      Serial.println(MQTT.state());
      delay(2000);
    }
  }
}

void initTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Sincronizando horário via NTP...");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter horário!");
    return;
  }
  Serial.println("Horário sincronizado com sucesso!");
}

String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "SemTempo";
  }
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

void verificaConexoes() {
  if (WiFi.status() != WL_CONNECTED) {
    initWiFi();
  }
  if (!MQTT.connected()) {
    reconnectMQTT();
  }
}

void enviaSensoresMQTT() {
  // === LDR ===
  int valorLDR = analogRead(LDR_PIN);
  // Inversão: quanto mais luz, maior o valor de luminosidade (%)
  float luminosidade = (1.0 - (float)valorLDR / 4095.0) * 100.0;

  // === DHT22 ===
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Falha ao ler o sensor DHT22!");
    return;
  }

  // === Timestamp ===
  String timestamp = getTimestamp();

  // === Monta mensagem JSON (padrão NGSI/FIWARE) ===
  String mensagem = "{";
  mensagem += "\"timestamp\":\"" + timestamp + "\",";
  mensagem += "\"temperatura\":" + String(temperatura, 1) + ",";
  mensagem += "\"umidade\":" + String(umidade, 1) + ",";
  mensagem += "\"luminosidade\":" + String(luminosidade, 1);
  mensagem += "}";

  // === Publica tudo em um único tópico ===
  MQTT.publish(TOPICO_BASE, mensagem.c_str());

  // === Debug no Serial ===
  Serial.println("📡 Dados publicados no Broker MQTT:");
  Serial.println(mensagem);
  Serial.println("-----------------------------");

  // === Publica valores individualmente ===

  // Timestamp
  mensagem = timestamp;
  MQTT.publish(TOPICO_TIMESTAMP, mensagem.c_str());
  Serial.print("Timestamp publicado: ");
  Serial.println(mensagem.c_str());

  // Luminosidade
  mensagem = String(luminosidade, 1);
  MQTT.publish(TOPICO_LUZ, mensagem.c_str());
  Serial.print("Luminosidade publicada: ");
  Serial.println(mensagem.c_str());

  // Temperatura
  mensagem = String(temperatura, 1);
  MQTT.publish(TOPICO_TEMP, mensagem.c_str());
  Serial.print("Temperatura publicada: ");
  Serial.println(mensagem.c_str());

  // Umidade
  mensagem = String(umidade, 1);
  MQTT.publish(TOPICO_UMID, mensagem.c_str());
  Serial.print("Umidade publicada: ");
  Serial.println(mensagem.c_str());

  Serial.println("-----------------------------");
}

void setup() {
  initSerial();
  initWiFi();
  initMQTT();
  dht.begin();
  initTime();  // Inicia sincronização do relógio NTP
}

void loop() {
  verificaConexoes();
  MQTT.loop();

  enviaSensoresMQTT();
  delay(4000); // Envia a cada 4 segundos
}
