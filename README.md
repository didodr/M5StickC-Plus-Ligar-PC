🖥️ M5StickCPlus PC Remote
Controle seu PC remotamente com M5StickCPlus via MQTT e Wake-on-LAN

https://img.shields.io/badge/version-1.0.0-green.svg
https://img.shields.io/badge/platform-M5StickCPlus-orange.svg
https://img.shields.io/badge/license-MIT-blue.svg

📋 Sobre
Este projeto transforma seu M5StickCPlus em um controle remoto para ligar e monitorar seu PC usando Wake-on-LAN e MQTT.

✨ Funcionalidades
🔌 Ligar PC - Envia Magic Packet via WoL

📊 Verificar Status - Ping no PC

🔄 Reiniciar Remoto - Reinicia o M5 via MQTT

🕐 Modo Relógio - Vira relógio após inatividade

📶 Indicador MQTT - Círculo verde/vermelho na tela

🛠️ Hardware
M5StickCPlus (ESP32)

PC com Wake-on-LAN ativado

📦 Instalação
1. Bibliotecas Necessárias
Instale via Arduino Library Manager:

M5StickCPlus

PubSubClient

ESP32Ping

NTPClient

2. Configuração -->
Edite no código:

ccp
const char* ssid = "SeuWiFi";

const char* password = "SuaSenha";

const char* pc_mac = "AA:BB:CC:DD:EE:FF";  // MAC do PC

const char* pc_ip = "192.168.1.100";       // IP do PC

3. Upload

Selecione a placa: M5Stick-C-Plus

Clique em Upload

🚀 Uso
Botões Físicos
Botão	Ação
BtnA	Verificar status do PC
BtnB	Ligar o PC
Comandos MQTT
bash
# Ligar PC
mosquitto_pub -h broker.emqx.io -t pc/command -m "ligar"

# Verificar Status
mosquitto_pub -h broker.emqx.io -t pc/command -m "status"

# Reiniciar M5
mosquitto_pub -h broker.emqx.io -t pc/command -m "reiniciar"
Tópicos MQTT
Tópico	Descrição
pc/command	Enviar comandos
pc/status	Receber status
🖥️ Interface
Tela Principal
text
┌─────────────────────────────┐
│ PC do DidoDR              │
│ ─────────────────────────── │
│ PC: LIGADO                  │
│ IP: 192.168.1.200          │
│ MAC: AA:BB:CC:DD:EE:FF    │
│ WiFi: 192.168.1.100        │
│ MQTT: OK                   │
└─────────────────────────────┘
Modo Relógio
text
┌─────────────────────────────┐
│ ●   14:30                   │
│      19/07/2024             │
│      PC: LIGADO             │            │
└─────────────────────────────┘
Indicadores
🟢 Verde = MQTT Conectado

🔴 Vermelho = MQTT Desconectado

🔧 Configurando WoL no PC
Windows
Gerenciador de Dispositivos

Placa de Rede → Propriedades

Gerenciamento de Energia

Marque: "Permitir que este dispositivo desperte o computador"

Linux
bash
sudo ethtool -s eth0 wol g
Encontrar MAC do PC
bash
# Windows
ipconfig /all

# Linux/Mac
ifconfig
⚠️ Resolução de Problemas

WiFi não conecta
Verifique SSID e senha
Reinicie o roteador

MQTT não conecta
Verifique se o broker está online

Verifique firewall/porta 1883

WoL não funciona
Confirme MAC correto

Verifique WoL ativado no PC

PC e M5 na mesma rede

📁 Estrutura do Código
text
├── m5stick_pc_remote.ino    # Código principal
├── README.md                 # Este arquivo
└── LICENSE                   # Licença MIT
🤝 Contribuição
Fork o projeto

Crie sua branch (git checkout -b feature/nova)

Commit (git commit -m 'Adiciona nova feature')

Push (git push origin feature/nova)

Abra um Pull Request

📜 Licença
MIT License - veja o arquivo LICENSE para detalhes.

👨‍💻 Autor
didodr

https://img.shields.io/badge/GitHub-@seuusuario-181717?style=flat&logo=github

⭐ Apoie
Se este projeto te ajudou, dê uma ⭐ no GitHub!

Divirta-se! 🚀

Links Úteis
M5StickCPlus Docs

Wake-on-LAN Guide

MQTT Essentials

v1.0.0 | 📅 Julho 2024

