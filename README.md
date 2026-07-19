# M5StickCPlus PC Remote

Controle seu PC remotamente com M5StickCPlus via MQTT e Wake-on-LAN

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-1.0.0-green.svg)](https://github.com/seuusuario/m5stick-pc-remote/releases)
[![Platform](https://img.shields.io/badge/platform-M5StickCPlus-orange.svg)](https://m5stack.com)

---

## Sobre

Este projeto transforma seu M5StickCPlus em um controle remoto para ligar e monitorar seu PC usando Wake-on-LAN e MQTT.

---

## Funcionalidades

- Ligar PC - Envia Magic Packet via Wake-on-LAN
- Verificar Status - Ping no PC
- Reiniciar Remoto - Reinicia o M5 via MQTT
- Modo Relogio - Vira relogio apos inatividade
- Indicador MQTT - Circulo verde/vermelho na tela

---

## Hardware

- M5StickCPlus (ESP32)
- PC com Wake-on-LAN ativado

---

## Instalacao

### Bibliotecas Necessarias

Instale via Arduino Library Manager:

- M5StickCPlus
- PubSubClient
- ESP32Ping
- NTPClient

### Configuracao

Edite no codigo:
```cpp
const char* ssid = "SeuWiFi";
const char* password = "SuaSenha";
const char* pc_mac = "AA:BB:CC:DD:EE:FF";
const char* pc_ip = "192.168.1.100";
````
### Upload

1. Selecione a placa: M5Stick-C-Plus
2. Clique em Upload

---

## Uso

### Botoes Fisicos

| Botao | Acao |
|-------|------|
| BtnA | Verificar status do PC |
| BtnB | Ligar o PC |

### Comandos MQTT
```BASH
mosquitto_pub -h broker.emqx.io -t pc/command -m "ligar"
mosquitto_pub -h broker.emqx.io -t pc/command -m "status"
mosquitto_pub -h broker.emqx.io -t pc/command -m "reiniciar"
```
### Topicos MQTT

| Topico | Descricao |
|--------|-----------|
| pc/command | Enviar comandos |
| pc/status | Receber status |

---

## Interface

### Tela Principal

| PC: LIGADO |
|------------|
IP: 192.168.1.101
MAC: AA:BB:CC:DD:EE:FF
WiFi: 192.168.1.100
MQTT: OK

### Modo Relogio

| 14:30 |
|-------|
19/07/2024
PC: LIGADO

### Indicadores

- Verde = MQTT Conectado
- Vermelho = MQTT Desconectado

---

## Configurando WoL no PC

### Windows

1. Gerenciador de Dispositivos
2. Placa de Rede -> Propriedades
3. Gerenciamento de Energia
4. Marque: Permitir que este dispositivo desperte o computador

### Linux

- sudo ethtool -s eth0 wol g

### Encontrar MAC do PC

- Windows: ipconfig /all
- Linux/Mac: ifconfig

---

## Resolucao de Problemas

### Tela nao liga

- Use M5Burner para restaurar firmware
- Reinstale a biblioteca M5StickCPlus

### WiFi nao conecta

- Verifique SSID e senha
- Reinicie o roteador

### MQTT nao conecta

- Verifique se o broker esta online
- Verifique firewall/porta 1883

### WoL nao funciona

- Confirme MAC correto
- Verifique WoL ativado no PC
- PC e M5 na mesma rede

---

## Contribuicao

1. Fork o projeto
2. Crie sua branch
3. Commit suas mudancas
4. Push para a branch
5. Abra um Pull Request

---

## Licenca

MIT License - veja o arquivo LICENSE para detalhes.

---

## Autor

<img src="https://img.shields.io/badge/GitHub-@didodr-181717?style=flat&logo=github" target="_blank"></a>

---

## Links Uteis

- https://docs.m5stack.com/en/core/stick_c_plus
- https://en.wikipedia.org/wiki/Wake-on-LAN
- https://www.hivemq.com/mqtt-essentials/
- https://github.com/arduino-libraries/NTPClient

---

## Apoie

Se este projeto te ajudou, de uma estrela no GitHub!

---

Feito com ❤️

---

**Divirta-se controlando seu PC!**
