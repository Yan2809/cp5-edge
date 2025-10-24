#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <time.h> // Para timestamp NTP

// === CONFIGURAÇÕES EDITÁVEIS ===
const char* SSID = "Wokwi-GUEST";                  // Rede Wi-Fi (2.4GHz)
const char* PASSWORD = "";                         // Senha Wi-Fi
const char* BROKER_MQTT = "test.mosquitto.org";    // IP do Broker MQTT
const int BROKER_PORT = 1883;                      // Porta do Broker MQTT

// === PINOS ===
#define LDR_PIN 34       // Pino analógico do LDR
#define DHTPIN 15        // Pino do DHT22
#define DHTTYPE DHT22    // Tipo do sensor DHT
#define LED_PIN 2

// === TÓPICO BASE NGSI (FIWARE) ===
const char* TOPICO_PUBLISH = "/TEF/device001/attrs";  // Envia tudo junto
const char* TOPICO_CMD = "/TEF/device001/cmd"; 
const char* ID_MQTT = "fiware_001";

// === OBJETOS ===
WiFiClient espClient;
PubSubClient MQTT(espClient);
DHT dht(DHTPIN, DHTTYPE);

// === CONFIGURAÇÃO NTP (para timestamp) ===
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600;  // Horário de Brasília
const int daylightOffset_sec = 0;

// === VARIÁVEL DE ESTADO DO LED ===
char estadoLED = '0'; // '0' desligado, '1' ligado

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
  MQTT.setCallback(callback);
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Tentando conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao Broker MQTT!");

      // Inscreve-se no tópico de controle
      MQTT.subscribe(TOPICO_CMD);
      Serial.print("🛰 Assinado no tópico: ");
      Serial.println(TOPICO_CMD);
    } else {
      Serial.println("Falha na conexão. Código de erro: ");
      Serial.println(MQTT.state());
      delay(2000);
    }
  }
}

void initTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("⏳ Sincronizando horário via NTP...");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter horário!");
    return;
  }
  Serial.println("\n⏰Horário sincronizado com sucesso!");
}

String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("❌ Erro ao obter horário local");
    return "SemTempo";
  }

  char time_str[64];
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.print("Data/Hora Formatada: ");
  Serial.println(time_str);
  return String(time_str);

  //strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
}

void verificaConexoes() {
  if (WiFi.status() != WL_CONNECTED) {
    initWiFi();
  }
  if (!MQTT.connected()) {
    reconnectMQTT();
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  msg.trim();
  Serial.print("📩 Mensagem recebida: ");
  Serial.println(msg);

  if (msg.equals("device001@on|")) {
    digitalWrite(LED_PIN, HIGH);
    estadoLED = '1';
    MQTT.publish(TOPICO_PUBLISH, "s|on");
    Serial.println("💡 LED LIGADO e status publicado!");
  } 
  else if (msg.equals("device001@off|")) {
    digitalWrite(LED_PIN, LOW);
    estadoLED = '0';
    MQTT.publish(TOPICO_PUBLISH, "s|off");
    Serial.println("💤 LED DESLIGADO e status publicado!");
  } 
  else {
    Serial.println("Comando desconhecido.");
  }
}

void enviaSensoresMQTT() {
  // === LDR ===
  int valorLDR = analogRead(LDR_PIN);
  // Inversão: quanto mais luz, maior o valor de luminosidade (%)
  float luminosidade = map(valorLDR, 4095, 0, 100, 0);

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
  MQTT.publish(TOPICO_PUBLISH, mensagem.c_str());

  // === Debug no Serial ===
  Serial.println("📡 Dados publicados no Broker MQTT:");
  Serial.println(mensagem);
  Serial.println("-----------------------------");

}

void setup() {
  initSerial();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // LED inicia desligado

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
