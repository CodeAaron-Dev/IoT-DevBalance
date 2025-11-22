#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================
//  CONFIGURAÇÕES WI-FI
// ============================
const char* WIFI_SSID     = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

// ============================
//  CONFIGURAÇÕES MQTT / FIWARE
// ============================

const char* MQTT_SERVER = "135.237.151.16";
const int   MQTT_PORT   = 1883;

// Dados compatíveis com o IoT Agent UL
const char* API_KEY   = "4jggokgpepnvsb2uv4s40d59ov";
const char* DEVICE_ID = "productivity001";

// Tópicos MQTT (padrão IoT Agent UL MQTT)
// /ul/<apiKey>/<deviceId>/attrs   -> medidas (device -> IoT Agent)
// /<apiKey>/<deviceId>/cmd        -> comandos (IoT Agent -> device)
// /<apiKey>/<deviceId>/cmdexe     -> ack de comando (device -> IoT Agent)
String topic_attrs  = String("/ul/") + API_KEY + "/" + DEVICE_ID + "/attrs";
String topic_cmd    = String("/")   + API_KEY + "/" + DEVICE_ID + "/cmd";
// *** Corrigido: sem /ul no cmdexe ***
String topic_cmdexe = String("/")   + API_KEY + "/" + DEVICE_ID + "/cmdexe";

// ============================
//  DISPLAY OLED (SSD1306 I2C)
// ============================
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ============================
//  BOTÕES (pinos do Wokwi)
// ============================
struct Button {
  int pin;
  bool currentState;
  bool previousState;
  unsigned long lastChangeMs;
};

Button greenBtn = {5, HIGH, HIGH, 0}; // start / resume
Button redBtn   = {4, HIGH, HIGH, 0}; // pause / stop

const unsigned long DEBOUNCE_MS = 50;

// ============================
//  ESTADOS DA LÂMPADA
// ============================
enum DeviceState { IDLE, WORK, PAUSED };
DeviceState currentState = IDLE;

unsigned long lastStateChangeMs = 0;
unsigned long lastWorkTickMs    = 0;
unsigned long lastPublishMs     = 0;

unsigned long workSeconds       = 0;
unsigned int  cyclesCompleted   = 0;

// ============================
//  MQTT / WiFi clients
// ============================
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ============================
//  FUNÇÕES DE DISPLAY
// ============================
void oledMessage(const char* l1, const char* l2 = "", const char* l3 = "") {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  int y = 10;
  display.setCursor(0, y);       display.println(l1);
  display.setCursor(0, y + 12);  display.println(l2);
  display.setCursor(0, y + 24);  display.println(l3);

  display.display();
}

void updateDisplayStatus() {
  const char* stateStr =
    (currentState == IDLE)  ? "IDLE" :
    (currentState == WORK)  ? "WORK" :
                              "PAUSED";

  char line1[20];
  char line2[20];
  char line3[20];

  snprintf(line1, sizeof(line1), "State: %s", stateStr);
  snprintf(line2, sizeof(line2), "Work: %lus", workSeconds);
  snprintf(line3, sizeof(line3), "Cycles: %u", cyclesCompleted);

  oledMessage(line1, line2, line3);
}

// ============================
//  WI-FI
// ============================
void connectWiFi() {
  Serial.print("Conectando ao WiFi ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ============================
//  PUBLICAÇÃO ULTRALIGHT
// ============================
// UL payload: state|WORK|work_seconds|123|cycles_completed|2|uptime_seconds|456
void publishStatus() {
  unsigned long nowMs = millis();
  unsigned long uptimeSeconds = nowMs / 1000;

  const char* stateStr =
    (currentState == IDLE)  ? "IDLE" :
    (currentState == WORK)  ? "WORK" :
                              "PAUSED";

  String payload = "";
  payload += "state|";             payload += stateStr;
  payload += "|work_seconds|";     payload += String(workSeconds);
  payload += "|cycles_completed|"; payload += String(cyclesCompleted);
  payload += "|uptime_seconds|";   payload += String(uptimeSeconds);

  bool ok = mqttClient.publish(topic_attrs.c_str(), payload.c_str());
  Serial.print("Publish attrs [");
  Serial.print(topic_attrs);
  Serial.print("] = ");
  Serial.print(payload);
  Serial.println(ok ? "  OK" : "  FAIL");
}

// ============================
//  MÁQUINA DE ESTADOS
// ============================
void changeState(DeviceState newState, const char* reason) {
  if (newState == currentState) return;

  Serial.print("Estado: ");
  if (currentState == IDLE)  Serial.print("IDLE");
  if (currentState == WORK)  Serial.print("WORK");
  if (currentState == PAUSED)Serial.print("PAUSED");
  Serial.print(" -> ");
  if (newState == IDLE)      Serial.print("IDLE");
  if (newState == WORK)      Serial.print("WORK");
  if (newState == PAUSED)    Serial.print("PAUSED");
  Serial.print(" [");
  Serial.print(reason);
  Serial.println("]");

  // Regra de ciclo:
  // se estávamos em WORK ou PAUSED, voltamos para IDLE
  // e já houve tempo de trabalho, conta 1 ciclo
  if ((currentState == WORK || currentState == PAUSED) &&
      newState == IDLE &&
      workSeconds > 0) {
    cyclesCompleted++;
  }

  currentState = newState;
  lastStateChangeMs = millis();
  updateDisplayStatus();
  publishStatus();
}

// ============================
//  CALLBACK MQTT (comandos)
// ============================
// Esperado no tópico /<apiKey>/<deviceId>/cmd
// Exemplo de payload UL:
//   productivity001@start|
//   productivity001@pause|
//   productivity001@reset|
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String t = String(topic);
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("MQTT msg em [");
  Serial.print(t);
  Serial.print("]: ");
  Serial.println(msg);

  if (t != topic_cmd) {
    return;
  }

  int atPos   = msg.indexOf('@');
  int pipePos = msg.indexOf('|', atPos + 1);

  if (atPos < 0 || pipePos < 0) {
    Serial.println("Comando mal formatado (esperado device@cmd|)");
    return;
  }

  String dev = msg.substring(0, atPos);
  String cmd = msg.substring(atPos + 1, pipePos);

  if (dev != String(DEVICE_ID)) {
    Serial.println("Comando para outro device: " + dev);
    return;
  }

  // Executa o comando
  if (cmd == "start") {
    changeState(WORK, "remote_start");
  } else if (cmd == "pause") {
    if (currentState == WORK) {
      changeState(PAUSED, "remote_pause");
    }
  } else if (cmd == "resume") {
    if (currentState == PAUSED) {
      changeState(WORK, "remote_resume");
    }
  } else if (cmd == "stop") {
    changeState(IDLE, "remote_stop");
  } else if (cmd == "reset") {
    workSeconds     = 0;
    cyclesCompleted = 0;
    changeState(IDLE, "remote_reset");
  } else {
    Serial.println("Comando desconhecido: " + cmd);
  }

  // Envia ACK pro IoT Agent UL (conteúdo vira <cmd>_info)
  String ack = "";
  ack += DEVICE_ID;
  ack += "@";
  ack += cmd;
  ack += "|OK";

  bool ok = mqttClient.publish(topic_cmdexe.c_str(), ack.c_str());
  Serial.print("ACK comando [");
  Serial.print(topic_cmdexe);
  Serial.print("] = ");
  Serial.print(ack);
  Serial.println(ok ? "  OK" : "  FAIL");
}

// ============================
//  CONEXÃO MQTT
// ============================
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Conectando ao MQTT em ");
    Serial.print(MQTT_SERVER);
    Serial.print(":");
    Serial.println(MQTT_PORT);

    String clientId = "ESP32-" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("MQTT conectado!");
      mqttClient.subscribe(topic_cmd.c_str());
      Serial.print("Assinado em: ");
      Serial.println(topic_cmd);
      // Publica um status inicial
      publishStatus();
    } else {
      Serial.print("Falha MQTT, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" tentando novamente em 2s...");
      delay(2000);
    }
  }
}

// ============================
//  LEITURA DOS BOTÕES
// ============================
void checkButton(Button &btn) {
  bool reading = digitalRead(btn.pin);
  unsigned long now = millis();

  if (reading != btn.previousState) {
    btn.lastChangeMs = now;
    btn.previousState = reading;
  }

  if ((now - btn.lastChangeMs) > DEBOUNCE_MS) {
    if (reading != btn.currentState) {
      btn.currentState = reading;

      // Botões com pull-up: LOW = pressionado
      if (btn.currentState == LOW) {
        if (btn.pin == greenBtn.pin) {
          // Botão verde: start/resume
          if (currentState == IDLE) {
            changeState(WORK, "local_start");
          } else if (currentState == PAUSED) {
            changeState(WORK, "local_resume");
          }
        } else if (btn.pin == redBtn.pin) {
          // Botão vermelho: pause/stop
          if (currentState == WORK) {
            changeState(PAUSED, "local_pause");
          } else if (currentState == PAUSED || currentState == IDLE) {
            changeState(IDLE, "local_stop");
          }
        }
      }
    }
  }
}

// ============================
//  SETUP
// ============================
void setup() {
  Serial.begin(115200);
  delay(500);

  // Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Falha ao iniciar o display SSD1306");
    for (;;); // trava
  }
  display.clearDisplay();
  display.display();

  oledMessage("DevBalance", "Inicializando...", "");

  // Botões (pull-up interno)
  pinMode(greenBtn.pin, INPUT_PULLUP);
  pinMode(redBtn.pin,   INPUT_PULLUP);

  // WiFi
  connectWiFi();

  // MQTT
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  connectMQTT();

  currentState = IDLE;
  lastStateChangeMs = millis();
  lastWorkTickMs    = millis();
  updateDisplayStatus();
}

// ============================
//  LOOP PRINCIPAL
// ============================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    connectMQTT();
  }

  mqttClient.loop();

  // Atualiza workSeconds enquanto está em WORK
  unsigned long now = millis();
  if (currentState == WORK) {
    if (now - lastWorkTickMs >= 1000) {
      workSeconds++;
      lastWorkTickMs = now;
    }
  } else {
    lastWorkTickMs = now;
  }

  // Checa botões
  checkButton(greenBtn);
  checkButton(redBtn);

  // Publica status a cada 5 segundos
  if (now - lastPublishMs >= 5000) {
    lastPublishMs = now;
    publishStatus();
    updateDisplayStatus();
  }

  delay(10);
}
