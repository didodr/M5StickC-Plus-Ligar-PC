#include <M5StickCPlus.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ESP32Ping.h>
#include <NTPClient.h>

// ============================================
// CONFIGURACOES E VERSIONAMENTO
// ============================================
#define FIRMWARE_VERSION "1.0.0"
// #define BUILD_DATE __DATE__ " " __TIME__

const char* ssid = "SeuWiFi";
const char* password = "SuaSenha";
const char* pc_mac = "AA:BB:CC:DD:EE:FF";
const char* pc_ip = "192.168.1.101";

const char* mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_username = "emqx";
const char* mqtt_password = "public";
const char* mqtt_topic_comando = "pc/command";
const char* mqtt_topic_status = "pc/status";

const char* ntp_server = "pool.ntp.org";
const int timezone_offset = -3;

// ============================================
// VARIAVEIS GLOBAIS
// ============================================
WiFiUDP udp;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntp_server, timezone_offset * 3600, 60000);

byte mac[6];
bool pcOnline = false;
unsigned long lastActivity = 0;
const unsigned long screenTimeout = 30000;
bool modoCompleto = true;

unsigned long lastClockUpdate = 0;
const unsigned long clockUpdateInterval = 5000;
unsigned long lastNtpUpdate = 0;
unsigned long lastMqttAttempt = 0;
const unsigned long mqttRetryInterval = 5000;

bool waitingForBoot = false;
unsigned long wolStartTime = 0;
unsigned long lastWolCheck = 0;
const unsigned long wolTimeout = 60000;
const unsigned long wolCheckInterval = 3000;
bool wolUdpInitialized = false;

#define COR_FUNDO TFT_BLACK
#define COR_TEXTO TFT_WHITE
#define COR_LIGADO TFT_GREEN
#define COR_DESLIGADO TFT_RED
#define COR_CYAN TFT_CYAN
#define COR_DESTAQUE TFT_YELLOW

// ============================================
// FUNCOES AUXILIARES
// ============================================
void parseMacAddress(const char* mac_str) {
    unsigned int values[6];
    int result = sscanf(mac_str, "%x:%x:%x:%x:%x:%x", 
                       &values[0], &values[1], &values[2], 
                       &values[3], &values[4], &values[5]);
    
    if (result == 6) {
        for(int i = 0; i < 6; i++) {
            mac[i] = (byte)values[i];
        }
        Serial.println("MAC parseado com sucesso!");
    } else {
        Serial.println("ERRO ao parsear MAC!");
    }
}

void sendWOL() {
    if (waitingForBoot) {
        Serial.println("WoL já enviado, aguardando boot...");
        return;
    }
    
    byte magicPacket[102];
    memset(magicPacket, 0xFF, 6);
    for (int i = 0; i < 16; i++) {
        memcpy(&magicPacket[6 + (i * 6)], mac, 6);
    }
    
    Serial.println("Enviando Magic Packet (3x)...");
    
    // Porta local 5678 evita conflito com porta de destino WoL (9)
    if (!wolUdpInitialized) {
        if (!udp.begin(5678)) {
            Serial.println("Falha ao inicializar UDP para WoL");
            return;
        }
        wolUdpInitialized = true;
    }
    
    for(int k = 0; k < 3; k++) {
        udp.beginPacket(IPAddress(255, 255, 255, 255), 9);
        udp.write(magicPacket, 102);
        if (!udp.endPacket()) {
            Serial.println("Falha ao enviar pacote WoL");
        }
        delay(100);
    }
    
    Serial.println("Pacotes WoL enviados!");
    waitingForBoot = true;
    wolStartTime = millis();
    lastWolCheck = 0;
}

bool isPCOnline() {
    IPAddress ip;
    if (!ip.fromString(pc_ip)) {
        Serial.println("IP inválido");
        return false;
    }
    return Ping.ping(ip, 3);
}

String getTimeString() {
    time_t rawtime = timeClient.getEpochTime();
    struct tm * timeinfo = localtime(&rawtime);
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
    return String(buffer);
}

String getDateString() {
    time_t rawtime = timeClient.getEpochTime();
    struct tm * timeinfo = localtime(&rawtime);
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y", timeinfo);
    return String(buffer);
}

// ============================================
// TELAS COM INDICADOR VISUAL MQTT
// ============================================
void mostrarModoRelogio() {
    M5.Lcd.fillScreen(COR_FUNDO);
    
    // Indicador visual MQTT (canto superior direito)
    if (mqttClient.connected()) {
        M5.Lcd.fillCircle(220, 10, 5, TFT_GREEN);
    } else {
        M5.Lcd.fillCircle(220, 10, 5, TFT_RED);
    }
    
    M5.Lcd.setTextColor(COR_DESTAQUE);
    M5.Lcd.setTextSize(4);
    M5.Lcd.setCursor(10, 35);
    M5.Lcd.println(getTimeString());
    
    M5.Lcd.setTextColor(COR_TEXTO);
    M5.Lcd.setTextSize(1.5);
    M5.Lcd.setCursor(10, 85);
    M5.Lcd.println(getDateString());
    
    M5.Lcd.setTextSize(1.5);
    M5.Lcd.setCursor(10, 110);
    if (pcOnline) {
        M5.Lcd.setTextColor(COR_LIGADO);
        M5.Lcd.println("PC: LIGADO");
    } else {
        M5.Lcd.setTextColor(COR_DESLIGADO);
        M5.Lcd.println("PC: DESLIGADO");
    }
    
    M5.Lcd.setTextColor(COR_TEXTO);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 135);
    if (WiFi.status() == WL_CONNECTED) {
        M5.Lcd.setTextColor(COR_CYAN);
        M5.Lcd.print("WiFi: ");
        M5.Lcd.println(WiFi.localIP().toString());
    } else {
        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.println("WiFi Offline");
    }
    
    M5.Lcd.setCursor(10, 150);
    if (mqttClient.connected()) {
        M5.Lcd.setTextColor(COR_CYAN);
        M5.Lcd.println("MQTT: OK");
    } else {
        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.println("MQTT: Offline");
    }
}

void atualizarTela(bool ligado) {
    M5.Lcd.fillScreen(COR_FUNDO);
    
    // Indicador visual MQTT redesenhado após limpeza da tela
    if (mqttClient.connected()) {
        M5.Lcd.fillCircle(220, 10, 5, TFT_GREEN);
    } else {
        M5.Lcd.fillCircle(220, 10, 5, TFT_RED);
    }
    
    M5.Lcd.setTextColor(COR_DESTAQUE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 5);
    M5.Lcd.println("PC do DidoDR");
    M5.Lcd.drawLine(5, 30, 230, 30, COR_TEXTO);
    
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 40);
    if (ligado) {
        M5.Lcd.setTextColor(COR_LIGADO);
        M5.Lcd.println("PC: LIGADO");
    } else {
        M5.Lcd.setTextColor(COR_DESLIGADO);
        M5.Lcd.println("PC: DESLIGADO");
    }
    
    M5.Lcd.setTextColor(COR_TEXTO);
    M5.Lcd.setTextSize(1.5);
    M5.Lcd.setCursor(10, 65);
    M5.Lcd.print("IP: ");
    M5.Lcd.println(pc_ip);
    
    M5.Lcd.setCursor(10, 82);
    M5.Lcd.print("MAC: ");
    M5.Lcd.println(pc_mac);
    
    M5.Lcd.setCursor(10, 99);
    if (WiFi.status() == WL_CONNECTED) {
        M5.Lcd.setTextColor(COR_CYAN);
        M5.Lcd.print("WiFi: ");
        M5.Lcd.println(WiFi.localIP().toString());
    } else {
        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.println("WiFi Offline");
    }
    
    M5.Lcd.setCursor(10, 116);
    if (mqttClient.connected()) {
        M5.Lcd.setTextColor(COR_CYAN);
        M5.Lcd.println("MQTT: OK");
    } else {
        M5.Lcd.setTextColor(TFT_RED);
        M5.Lcd.println("MQTT: Offline");
    }
    
    // Versão do firmware
   // M5.Lcd.setTextSize(1);
   // M5.Lcd.setTextColor(TFT_DARKGREY);
   // M5.Lcd.setCursor(10, 130);
   // M5.Lcd.printf("v%s | %s", FIRMWARE_VERSION, BUILD_DATE);
    
    lastActivity = millis();
    modoCompleto = true;
}

void entrarModoRelogio() {
    if (modoCompleto) {
        modoCompleto = false;
        mostrarModoRelogio();
        Serial.println("Entrou em modo relogio");
    }
}

void verificarEAtualizarStatus() {
    bool novoStatus = isPCOnline();
    if (novoStatus != pcOnline) {
        pcOnline = novoStatus;
        Serial.print("Status mudou para: ");
        Serial.println(pcOnline ? "LIGADO" : "DESLIGADO");
        if (modoCompleto) atualizarTela(pcOnline);
        else mostrarModoRelogio();
        publicarStatus();
    }
}

bool conectarMQTT() {
    if (mqttClient.connected()) return true;
    
    Serial.println("Conectando ao EMQX...");
    String clientId = "M5StickPC-" + String(WiFi.macAddress());
    
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
        Serial.println("Conectado ao EMQX!");
        mqttClient.subscribe(mqtt_topic_comando);
        publicarStatus();
        return true;
    } else {
        Serial.print("Falha MQTT, rc=");
        Serial.println(mqttClient.state());
        return false;
    }
}

void publicarStatus() {
    if (!mqttClient.connected()) return;
    
    String status = pcOnline ? "LIGADO" : "DESLIGADO";
    if (mqttClient.publish(mqtt_topic_status, status.c_str())) {
        Serial.print("Status publicado: ");
        Serial.println(status);
    } else {
        Serial.println("Falha ao publicar status");
    }
}

// Callback MQTT com proteção de buffer e comando de reinício remoto
void callbackMQTT(char* topic, byte* payload, unsigned int length) {
    if (length > 64) {
        Serial.println("MQTT: Mensagem muito longa, ignorada");
        return;
    }
    
    char mensagem[65];
    memcpy(mensagem, payload, length);
    mensagem[length] = '\0';
    
    Serial.print("MQTT recebido: ");
    Serial.println(mensagem);
    
    if (!modoCompleto) {
        modoCompleto = true;
        atualizarTela(pcOnline);
    } else {
        lastActivity = millis();
    }
    
    if (strcmp(mensagem, "ligar") == 0) {
        Serial.println("Comando MQTT: Ligar PC");
        if (!waitingForBoot) {
            sendWOL();
            pcOnline = false;
            atualizarTela(false);
        } else {
            Serial.println("Já aguardando boot do PC");
        }
    }
    else if (strcmp(mensagem, "status") == 0) {
        Serial.println("Comando MQTT: Status");
        verificarEAtualizarStatus();
    }
    else if (strcmp(mensagem, "reiniciar") == 0) {
        Serial.println("Comando MQTT: Reiniciar M5");
        // retain=true garante que o broker entregue mesmo se o device cair durante o restart
        mqttClient.publish(mqtt_topic_status, "REINICIANDO", true);
        delay(500); // Tempo seguro para o broker processar a mensagem
        ESP.restart();
    }
}

// ============================================
// SETUP
// ============================================
void setup() {
    Serial.begin(115200);
    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(COR_FUNDO);
    M5.Lcd.setTextColor(COR_TEXTO);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 40);
    M5.Lcd.println("Iniciando...");
    M5.Lcd.setCursor(10, 65);
    M5.Lcd.printf("v%s", FIRMWARE_VERSION);
    
    parseMacAddress(pc_mac);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int tentativas = 0;
    const int maxTentativas = 30;
    
    while (WiFi.status() != WL_CONNECTED && tentativas < maxTentativas) {
        delay(500);
        tentativas++;
        M5.Lcd.setCursor(10, 90);
        M5.Lcd.println("Conectando WiFi...");
        M5.Lcd.setCursor(10, 110);
        M5.Lcd.printf("Tentativa %d/%d", tentativas, maxTentativas);
        Serial.printf("WiFi tentativa %d/%d\n", tentativas, maxTentativas);
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        M5.Lcd.fillScreen(TFT_RED);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setCursor(10, 40);
        M5.Lcd.println("ERRO WiFi!");
        M5.Lcd.setCursor(10, 60);
        M5.Lcd.println("Aperte B p/ reiniciar");
        
        while(true) {
            delay(100);
            M5.update();
            if (M5.BtnB.wasPressed()) {
                ESP.restart();
            }
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi conectado!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        M5.Lcd.fillScreen(COR_FUNDO);
        M5.Lcd.setTextColor(TFT_GREEN);
        M5.Lcd.setCursor(10, 40);
        M5.Lcd.println("WiFi OK!");
        M5.Lcd.setTextColor(COR_TEXTO);
        M5.Lcd.setCursor(10, 65);
        M5.Lcd.print("IP: ");
        M5.Lcd.println(WiFi.localIP());
        delay(1000);
    }
    
    timeClient.begin();
    timeClient.update();
    lastNtpUpdate = millis();
    
    mqttClient.setServer(mqtt_broker, mqtt_port);
    mqttClient.setCallback(callbackMQTT);
    mqttClient.setBufferSize(512);
    
    pcOnline = isPCOnline();
    atualizarTela(pcOnline);
    
    Serial.println("Sistema pronto!");
}

// ============================================
// LOOP PRINCIPAL (100% NÃO-BLOQUEANTE)
// ============================================
void loop() {
    M5.update();
    
    // Atualiza NTP a cada 60s
    if (millis() - lastNtpUpdate > 60000) {
        if (WiFi.status() == WL_CONNECTED) {
            timeClient.update();
            lastNtpUpdate = millis();
        }
    }
    
    // Atualiza relógio na tela
    if (!modoCompleto && (millis() - lastClockUpdate > clockUpdateInterval)) {
        lastClockUpdate = millis();
        mostrarModoRelogio();
    }
    
    // Timeout da tela
    if (modoCompleto && (millis() - lastActivity > screenTimeout)) {
        entrarModoRelogio();
    }
    
    // Mantém MQTT vivo (não bloqueante)
    if (!mqttClient.connected()) {
        if (millis() - lastMqttAttempt > mqttRetryInterval) {
            lastMqttAttempt = millis();
            if (WiFi.status() == WL_CONNECTED) {
                conectarMQTT();
            }
        }
    } else {
        mqttClient.loop();
    }
    
    // Verificação NÃO-BLOQUEANTE de boot do PC
    if (waitingForBoot) {
        if (millis() - lastWolCheck > wolCheckInterval) {
            lastWolCheck = millis();
            
            if (isPCOnline()) {
                pcOnline = true;
                waitingForBoot = false;
                wolUdpInitialized = false;
                udp.stop();
                Serial.println("PC ligou com sucesso!");
                atualizarTela(true);
                publicarStatus();
            }
            else if (millis() - wolStartTime > wolTimeout) {
                waitingForBoot = false;
                wolUdpInitialized = false;
                udp.stop();
                Serial.println("Tempo esgotado. PC nao respondeu.");
                atualizarTela(false);
            }
        }
    }
    
    // Botão A: Status manual
    if (M5.BtnA.wasPressed()) {
        Serial.println("Botao A - Status");
        if (!modoCompleto) {
            modoCompleto = true;
            atualizarTela(pcOnline);
        } else {
            lastActivity = millis();
            atualizarTela(pcOnline);
        }
        verificarEAtualizarStatus();
    }
    
    // Botão B: Ligar PC (NÃO-BLOQUEANTE)
    if (M5.BtnB.wasPressed()) {
        Serial.println("Botao B - Ligar PC");
        if (!waitingForBoot) {
            if (!modoCompleto) {
                modoCompleto = true;
                atualizarTela(pcOnline);
            } else {
                lastActivity = millis();
            }
            sendWOL();
            pcOnline = false;
            atualizarTela(false);
        } else {
            Serial.println("Já aguardando boot do PC");
            M5.Lcd.setCursor(10, 150);
            M5.Lcd.setTextColor(TFT_YELLOW);
            M5.Lcd.println("Aguardando...");
        }
    }
    
    delay(5); // Delay mínimo apenas para estabilidade do WiFi stack
}
