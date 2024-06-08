#include <WiFi.h>
#include <PubSubClient.h> // by Nick O'Leary
#include <WiFiManager.h> // by tzapu
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h> // by Benoit Blanchon
#include <OneWire.h> // by Jim Studt
#include <DallasTemperature.h> // by Miles Burton

// Constantes
#define ONE_WIRE_BUS 23
#define RESET_BUTTON_PIN 19
#define RESET_BUTTON_TIME 3000
#define MQTT_SEND_TIME 60000
#define TEMPERATURE_OFFSET 0.4

// Configurações iniciais MQTT
char mqtt_server[60] = "broker.mqtt.cool";
char mqtt_port[6] = "1883";  // Porta como string para compatibilidade com WiFiManager
char mqtt_topic[60] = "sensores/temperatura_corporal";

// Variáveis
unsigned long resetButtonTimer = 0;
unsigned long mqttTimer = 0;
boolean isResetButtonPressed = false;

WiFiClient wifiClient;
PubSubClient client(wifiClient);
WiFiManager wifiManager;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

void callback(char* topic, byte* payload, unsigned int length) {
  // Callback para verificar se as mensagens foram recebidas se necessário
}

void setup() {
  Serial.begin(115200);

  // Inicializa a SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Falha ao montar o sistema de arquivos");
    return;
  }

  // Lê o configurações salvas no arquivo config.json
  loadConfig();

  // Adiciona parâmetros personalizados para MQTT
  WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqtt_server, 60);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", mqtt_port, 6);
  WiFiManagerParameter custom_mqtt_topic("topic", "MQTT Topic", mqtt_topic, 60);

  // Adiciona os parâmetros ao WiFiManager
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_topic);

  // Se o ESP32 não conseguir conectar, ele abre um portal de configuração
  if (!wifiManager.autoConnect("ESP32_AP")) {
    Serial.println("Falha na conexão e timeout!");
    // Resetar o ESP
    ESP.restart();
  }
  Serial.println(F("Conectado na rede com sucesso!"));

  // Obtém os valores dos campos personalizados
  if (String(custom_mqtt_server.getValue()) != "") strcpy(mqtt_server, custom_mqtt_server.getValue());
  if (String(custom_mqtt_port.getValue()) != "") strcpy(mqtt_port, custom_mqtt_port.getValue());
  if (String(custom_mqtt_topic.getValue()) != "") strcpy(mqtt_topic, custom_mqtt_topic.getValue());

  // Salva a configuração
  saveConfig();

  // Reseta as configurações caso nova configuração não tenha algum dos valores obrigatórios
  verifyConfigs(true);

  client.setServer(mqtt_server, atoi(mqtt_port));  // Converte a porta para int
  client.setCallback(callback);

  // Verifica a conexão WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conexão WiFi falhou!");
    return;
  }
  Serial.println("Conectado ao WiFi com sucesso!");

  // Iniciando o sensor
  ds18b20.begin();
  ds18b20.setResolution(12);

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  mqttSend();
  checkResetButton();
}

void mqttSend() {
  if (mqttTimer < millis()) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    ds18b20.requestTemperatures();
    float temperature = ds18b20.getTempCByIndex(0);
    if (temperature != DEVICE_DISCONNECTED_C) {
      temperature += TEMPERATURE_OFFSET; // Adiciona offset de temperatura quando sensor necessita de algum
      char tempString[8];
      dtostrf(temperature, 5, 2, tempString);

      Serial.print("Publicando temperatura no tópico ");
      Serial.print(mqtt_topic);
      Serial.print(": ");
      Serial.println(tempString);

      if (client.publish(mqtt_topic, tempString, true)) {
        Serial.println("Mensagem publicada com sucesso!");
        mqttTimer = millis() + MQTT_SEND_TIME;
      } else {
        Serial.println("Falha ao publicar a mensagem. Tentando novamente...");
        delay(500);
      }
      
    } else {
      Serial.println("Erro de leitura! Verifique a conexão do sensor DS18B20. Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void loadConfig() {
  Serial.println(F("Iniciando leitura das configurações..."));
  if (SPIFFS.begin(true)) {
    if (SPIFFS.exists("/config.json")) {
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, buf.get());
        if (!doc.isNull()) {
          Serial.print(F("MQTT Server: "));
          strcpy(mqtt_server, doc["mqtt_server"]);
          Serial.println((strlen(mqtt_server) != 0) ? String(mqtt_server) : "Não há registro!");
          Serial.print(F("MQTT Port: "));
          strcpy(mqtt_port, doc["mqtt_port"]);
          Serial.println((strlen(mqtt_port) != 0) ? String(mqtt_port) : "Não há registro!");
          Serial.print(F("MQTT Topic: "));
          strcpy(mqtt_topic, doc["mqtt_topic"]);
          Serial.println((strlen(mqtt_topic) != 0) ? String(mqtt_topic) : "Não há registro!");
          verifyConfigs(false);
          Serial.println(F("Arquivo de configurações lido com sucesso!"));
        } else {
          Serial.println(F("Arquivo de configurações vazio!"));
        }
        configFile.close();
      }
    } else {
      Serial.println(F("Não existe um arquivo de configurações!"));
    }
  } else {
    Serial.println(F("Falha ao abrir o sistema de arquivos!"));
  }
}

void saveConfig() {
  Serial.println(F("Iniciando gravação das configurações..."));
  if (SPIFFS.begin(true)) {
    DynamicJsonDocument doc(1024);
    doc["mqtt_server"] = mqtt_server;
    doc["mqtt_port"] = mqtt_port;
    doc["mqtt_topic"] = mqtt_topic;
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println(F("Falha ao criar arquivo de configurações!"));
    } else {
      serializeJson(doc, configFile);
      configFile.close();
      Serial.println(F("Arquivo de configurações salvo com sucesso!"));
    }
  } else {
    Serial.println(F("Falha ao abrir o sistema de arquivos!"));
  }
}

void verifyConfigs(boolean isRestartNeeded) {
  if (strlen(mqtt_server) == 0 || strlen(mqtt_port) == 0 || strlen(mqtt_topic) == 0) {
    Serial.println(F("Algumas configurações não foram encontradas, resetando as configurações e voltando para o estado de portal!"));
    wifiManager.resetSettings();
    delay(500);
    if (isRestartNeeded) {
      ESP.restart();
    }
  }
}

void reconnect() {
  int retry = 0;
  while (!client.connected()) {
    Serial.println("Tentando conectar ao broker MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado no broker MQTT!");
    } else {
      retry += 1;
      Serial.print("Falha ao conectar ao broker MQTT! (rc = ");
      Serial.print(client.state());
      Serial.println(")");
      if (retry >= 10) {
        Serial.println("Muitas falhas de conectar ao broker, possível configuração inválida!");
        Serial.println("O ESP32 reiniciará em 5 segundos no modo de configuração.");
        wifiManager.resetSettings();
        delay(5000);
        ESP.restart();
      }
      Serial.print("Tentando novamente em 5 segundos (tentativa ");
      Serial.print(retry);
      Serial.println(" de 10).");
      delay(5000);
    }
  }
}

void checkResetButton() {
  if (!digitalRead(RESET_BUTTON_PIN )) {
    if (!isResetButtonPressed) {
      resetButtonTimer = millis() + RESET_BUTTON_TIME;
      isResetButtonPressed = true;
    }
    if (resetButtonTimer < millis()) {
      Serial.println(F("Resetando as configurações e voltando ao estado de portal!"));
      wifiManager.resetSettings();
      delay(500);
      ESP.restart();
    }
  }
  if (digitalRead(RESET_BUTTON_PIN ) && isResetButtonPressed) {
    isResetButtonPressed = false;
  }
}
