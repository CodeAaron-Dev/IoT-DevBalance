# 📊 DevBalance - Sistema de Rastreamento de Produtividade IoT

<div align="center">

![DevBalance Logo](./assets/logo.png)
<!-- Adicione sua logo aqui -->

**Monitoramento e controle de produtividade em tempo real com ESP32 e FIWARE**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![FIWARE](https://img.shields.io/badge/FIWARE-Enabled-blue)](https://www.fiware.org/)
[![ESP32](https://img.shields.io/badge/ESP32-Compatible-green)](https://www.espressif.com/)

[Demonstração](#-demonstração) • [Instalação](#-instalação) • [Como Usar](#-como-usar) • [Arquitetura](#-arquitetura) • [Documentação](#-documentação)

</div>

---

## 👥 Equipe DevBalance

| Nome | RM | Papel |
|------|----|----|
| **Cesar Aaron Herrera** | 565398 | Desenvolvedor IoT & Integração FIWARE |
| **Rafael Seiji Aoke Arakaki** | 561993 | Arquitetura de Sistemas & Backend |
| **Rafael Yuji Nakaya** | 563624 | Hardware & Protocolos de Comunicação |

---

## 📋 Índice

- [Sobre o Projeto](#-sobre-o-projeto)
- [Características](#-características)
- [Demonstração](#-demonstração)
- [Arquitetura](#-arquitetura)
- [Tecnologias](#-tecnologias)
- [Pré-requisitos](#-pré-requisitos)
- [Instalação](#-instalação)
- [Como Usar](#-como-usar)
- [API Reference](#-api-reference)
- [Fluxo de Dados](#-fluxo-de-dados)
- [Troubleshooting](#-troubleshooting)
- [Roadmap](#-roadmap)
- [Contribuindo](#-contribuindo)
- [Licença](#-licença)

---

## 🎯 Sobre o Projeto

**DevBalance** é um sistema IoT de rastreamento de produtividade que utiliza um dispositivo ESP32 conectado à plataforma **FIWARE** para monitorar sessões de trabalho em tempo real.

O sistema permite:
- ✅ Controle local via botões físicos
- ✅ Controle remoto via API REST
- ✅ Persistência de dados no Orion Context Broker
- ✅ Visualização em tempo real no display OLED
- ✅ Integração com dashboards externos

### 🎥 Demonstração

![Demonstração do Sistema](./assets/demo.gif)
<!-- Adicione um GIF ou vídeo mostrando o sistema funcionando -->

---

## ✨ Características

### Hardware
- 🖥️ **ESP32** com WiFi integrado
- 📟 **Display OLED SSD1306** (128x64 I2C)
- 🟢 **Botão Verde** - Start/Resume trabalho
- 🔴 **Botão Vermelho** - Pause/Stop trabalho

### Software
- 🌐 **MQTT** para comunicação IoT
- 🔄 **IoT Agent UltraLight** para tradução de protocolos
- 💾 **Orion Context Broker** para persistência de contexto
- 📡 **API REST** para consultas e comandos

### Funcionalidades
- ⏱️ Cronometragem de sessões de trabalho
- ⏸️ Pausar e retomar trabalho
- 📊 Contagem de ciclos completos
- 🔄 Reset de dados
- 📈 Histórico temporal (via TimeInstant)

---

## 🏗️ Arquitetura

### Diagrama de Componentes

```
┌─────────────────────────────────────────────────────────────┐
│                    CAMADA DE APLICAÇÃO                       │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Postman    │  │  Dashboard   │  │   Mobile     │      │
│  │   (REST)     │  │    (Web)     │  │    App       │      │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘      │
└─────────┼──────────────────┼──────────────────┼─────────────┘
          │                  │                  │
          └──────────────────┼──────────────────┘
                             │ HTTP/REST
          ┌──────────────────▼──────────────────┐
          │    Orion Context Broker (1026)      │
          │  ┌────────────────────────────────┐ │
          │  │  urn:ngsi-ld:Productivity:001  │ │
          │  │  - state: WORK                 │ │
          │  │  - workSeconds: 125            │ │
          │  │  - cyclesCompleted: 3          │ │
          │  └────────────────────────────────┘ │
          └──────────────────┬──────────────────┘
                             │ NGSI v2
          ┌──────────────────▼──────────────────┐
          │    IoT Agent UltraLight (4041)      │
          │  ┌────────────────────────────────┐ │
          │  │  Device: productivity001       │ │
          │  │  Attributes Mapping           │ │
          │  │  Commands Translation         │ │
          │  └────────────────────────────────┘ │
          └──────────────────┬──────────────────┘
                             │ MQTT
          ┌──────────────────▼──────────────────┐
          │    Mosquitto MQTT Broker (1883)     │
          │  Topics:                             │
          │  - /ul/{apikey}/{device}/attrs       │
          │  - /{apikey}/{device}/cmd            │
          │  - /{apikey}/{device}/cmdexe         │
          └──────────────────┬──────────────────┘
                             │ MQTT
          ┌──────────────────▼──────────────────┐
          │         ESP32 + Display OLED         │
          │  ┌────────────────────────────────┐ │
          │  │  State: WORK                   │ │
          │  │  Work: 125s                    │ │
          │  │  Cycles: 3                     │ │
          │  └────────────────────────────────┘ │
          │  [🟢 Green]  [🔴 Red]               │
          └─────────────────────────────────────┘
```

### Fluxo de Dados

![Arquitetura FIWARE](./assets/architecture.png)
<!-- Adicione um diagrama da arquitetura FIWARE -->

#### 1️⃣ Publicação de Dados (Device → Cloud)

```
ESP32 publica via MQTT
  ↓ Tópico: /ul/4jggo.../productivity001/attrs
  ↓ Formato: state|WORK|work_seconds|125|cycles_completed|3
  ↓
IoT Agent UltraLight recebe
  ↓ Traduz UltraLight → NGSI v2
  ↓ Mapeia atributos conforme provisionamento
  ↓
Orion Context Broker atualiza entidade
  ↓ Entity: urn:ngsi-ld:Productivity:001
  ↓ Adiciona TimeInstant
  ↓
Dados disponíveis via API REST
```

#### 2️⃣ Envio de Comandos (Cloud → Device)

```
Cliente (Postman) envia PATCH
  ↓ URL: /v2/entities/.../attrs
  ↓ Body: {"start": {"value": ""}}
  ↓
Orion Context Broker atualiza atributo
  ↓ Notifica IoT Agent (via subscrição interna)
  ↓
IoT Agent traduz comando
  ↓ Publica no MQTT
  ↓ Tópico: /4jggo.../productivity001/cmd
  ↓ Formato: productivity001@start|
  ↓
ESP32 recebe e executa comando
  ↓ Muda estado interno
  ↓ Envia ACK via /{apikey}/{device}/cmdexe
  ↓
IoT Agent confirma execução no Orion
```

---

## 🛠️ Tecnologias

### Hardware
| Componente | Especificação |
|------------|--------------|
| **Microcontrolador** | ESP32 DevKit v1 |
| **Display** | OLED SSD1306 128x64 I2C |
| **Botões** | 2x Push Button (pull-up) |
| **Conectividade** | WiFi 802.11 b/g/n |

### Software - Firmware
| Tecnologia | Versão | Uso |
|------------|--------|-----|
| **Arduino Core** | ESP32 | Framework de desenvolvimento |
| **PubSubClient** | 2.8+ | Cliente MQTT |
| **Adafruit GFX** | 1.11.0+ | Biblioteca gráfica |
| **Adafruit SSD1306** | 2.5.0+ | Driver OLED |

### Software - Backend (FIWARE)
| Componente | Versão | Porta | Descrição |
|------------|--------|-------|-----------|
| **Orion Context Broker** | 3.10+ | 1026 | Gerenciamento de contexto |
| **IoT Agent UltraLight** | 2.4+ | 4041 | Tradução de protocolos |
| **Mosquitto** | 2.0+ | 1883 | Broker MQTT |
| **MongoDB** | 4.4+ | 27017 | Banco de dados |

---

## 📦 Pré-requisitos

### Para o ESP32
- [Arduino IDE](https://www.arduino.cc/en/software) 1.8.19+ ou [PlatformIO](https://platformio.org/)
- Bibliotecas Arduino instaladas (ver [Instalação](#-instalação))

### Para o FIWARE
- Servidor Linux (Ubuntu 20.04+ recomendado)
- Docker 20.10+
- Docker Compose 1.29+
- 2GB RAM mínimo
- Portas liberadas: 1026, 1883, 4041

### Para Testes
- [Postman](https://www.postman.com/downloads/) ou equivalente
- Acesso à rede WiFi
- IP público ou VPN para acesso remoto

---

## 🚀 Instalação

### 1. Configurar FIWARE (Servidor)

#### 1.1 Clonar FIWARE Descomplicado

```bash
git clone https://github.com/fabiocabrini/fiware
cd fiware
```

#### 1.2 Iniciar Serviços

```bash
sudo docker-compose up -d
```

#### 1.3 Verificar Status

```bash
sudo docker ps

# Deve mostrar:
# - fiware_orion
# - fiware_iot-agent
# - fiware_mosquitto
# - fiware_mongo
```

#### 1.4 Liberar Portas no Firewall

```bash
sudo ufw allow 1883/tcp  # MQTT
sudo ufw allow 1026/tcp  # Orion
sudo ufw allow 4041/tcp  # IoT Agent
sudo ufw reload
```

---

### 2. Provisionar Dispositivo (Postman)

#### 2.1 Criar Service Group

**Requisição:**
```http
POST http://SEU_IP:4041/iot/services
Content-Type: application/json
fiware-service: smart
fiware-servicepath: /
```

**Body:**
```json
{
  "services": [
    {
      "apikey": "4jggokgpepnvsb2uv4s40d59ov",
      "cbroker": "http://orion:1026",
      "entity_type": "Productivity",
      "resource": "/iot/d"
    }
  ]
}
```

**Resposta esperada:** `201 Created`

---

#### 2.2 Provisionar Dispositivo ESP32

**Requisição:**
```http
POST http://SEU_IP:4041/iot/devices
Content-Type: application/json
fiware-service: smart
fiware-servicepath: /
```

**Body:**
```json
{
  "devices": [
    {
      "device_id": "productivity001",
      "entity_name": "urn:ngsi-ld:Productivity:001",
      "entity_type": "Productivity",
      "protocol": "PDI-IoTA-UltraLight",
      "transport": "MQTT",
      "timezone": "America/Sao_Paulo",
      "attributes": [
        {
          "object_id": "state",
          "name": "state",
          "type": "Text"
        },
        {
          "object_id": "work_seconds",
          "name": "workSeconds",
          "type": "Integer"
        },
        {
          "object_id": "cycles_completed",
          "name": "cyclesCompleted",
          "type": "Integer"
        },
        {
          "object_id": "uptime_seconds",
          "name": "uptimeSeconds",
          "type": "Integer"
        }
      ],
      "commands": [
        {"name": "start", "type": "command"},
        {"name": "pause", "type": "command"},
        {"name": "resume", "type": "command"},
        {"name": "stop", "type": "command"},
        {"name": "reset", "type": "command"}
      ]
    }
  ]
}
```

**Resposta esperada:** `201 Created`

---

### 3. Configurar ESP32

#### 3.1 Instalar Bibliotecas Arduino

No Arduino IDE:
1. **Sketch → Include Library → Manage Libraries**
2. Instale:
   - `PubSubClient` by Nick O'Leary
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`

#### 3.2 Configurar Código

Edite o arquivo `DevBalance.ino`:

```cpp
// CONFIGURAÇÕES WI-FI
const char* WIFI_SSID     = "SUA_REDE_WIFI";  // ← Altere aqui
const char* WIFI_PASSWORD = "SUA_SENHA";       // ← Altere aqui

// CONFIGURAÇÕES MQTT / FIWARE
const char* MQTT_SERVER = "SEU_IP_PUBLICO";   // ← Altere aqui
const int   MQTT_PORT   = 1883;

const char* API_KEY   = "4jggokgpepnvsb2uv4s40d59ov";
const char* DEVICE_ID = "productivity001";
```

#### 3.3 Conectar Hardware

| ESP32 Pin | Componente | Descrição |
|-----------|------------|-----------|
| GPIO 21 | OLED SDA | Dados I2C |
| GPIO 22 | OLED SCL | Clock I2C |
| GPIO 5 | Botão Verde | Start/Resume |
| GPIO 4 | Botão Vermelho | Pause/Stop |
| 3.3V | OLED VCC | Alimentação |
| GND | OLED GND + Botões | Terra |

![Esquema de Ligação](./assets/wiring.png)
<!-- Adicione diagrama de ligação -->

#### 3.4 Upload do Código

1. Conecte ESP32 via USB
2. Selecione: **Tools → Board → ESP32 Dev Module**
3. Selecione a porta COM correta
4. Clique em **Upload**
5. Abra **Serial Monitor** (115200 baud)

---

## 📖 Como Usar

### Controle Local (Botões)

#### Máquina de Estados

```
┌─────────┐
│  IDLE   │ ← Estado inicial
└────┬────┘
     │ 🟢 Botão Verde: START
     ▼
┌─────────┐
│  WORK   │ ← Cronômetro incrementando
└────┬────┘
     │ 🔴 Botão Vermelho: PAUSE
     ▼
┌─────────┐
│ PAUSED  │ ← Tempo pausado
└────┬────┘
     │ 🟢 Botão Verde: RESUME
     ▼
┌─────────┐
│  WORK   │
└────┬────┘
     │ 🔴 Botão Vermelho: STOP
     ▼
┌─────────┐
│  IDLE   │ ← Ciclo completado (+1)
└─────────┘
```

### Controle Remoto (API REST)

#### Consultar Dados Atuais

**Requisição:**
```http
GET http://SEU_IP:1026/v2/entities/Productivity:productivity001
fiware-service: smart
fiware-servicepath: /
```

**Resposta:**
```json
{
  "id": "Productivity:productivity001",
  "type": "Productivity",
  "state": {
    "type": "Text",
    "value": "WORK",
    "metadata": {}
  },
  "workSeconds": {
    "type": "Text",
    "value": "125",
    "metadata": {}
  },
  "cyclesCompleted": {
    "type": "Text",
    "value": "3",
    "metadata": {}
  },
  "uptimeSeconds": {
    "type": "Text",
    "value": "456",
    "metadata": {}
  },
  "TimeInstant": {
    "type": "DateTime",
    "value": "2025-11-22T15:30:45.123Z",
    "metadata": {}
  }
}
```

---

#### Enviar Comandos

**Template:**
```http
PATCH http://SEU_IP:1026/v2/entities/urn:ngsi-ld:Productivity:001/attrs
Content-Type: application/json
fiware-service: smart
fiware-servicepath: /
```

| Comando | Body | Efeito |
|---------|------|--------|
| **START** | `{"start": {"value": ""}}` | IDLE → WORK |
| **PAUSE** | `{"pause": {"value": ""}}` | WORK → PAUSED |
| **RESUME** | `{"resume": {"value": ""}}` | PAUSED → WORK |
| **STOP** | `{"stop": {"value": ""}}` | WORK/PAUSED → IDLE (+1 ciclo) |
| **RESET** | `{"reset": {"value": ""}}` | Zera workSeconds e cycles |

**Resposta esperada:** `204 No Content`

---

## 📊 Dados Rastreados

### Atributos da Entidade

| Atributo | Tipo | Descrição | Atualização |
|----------|------|-----------|-------------|
| `state` | Text | Estado atual: IDLE, WORK, PAUSED | Mudança de estado |
| `workSeconds` | Integer | Tempo total trabalhando (segundos) | A cada 1 segundo (em WORK) |
| `cyclesCompleted` | Integer | Quantidade de ciclos completos | Transição para IDLE |
| `uptimeSeconds` | Integer | Tempo desde que ESP32 ligou | A cada 5 segundos |
| `TimeInstant` | DateTime | Timestamp da última atualização | Toda publicação |

### Exemplo de Evolução Temporal

```
t=0s   : state=IDLE, workSeconds=0, cycles=0
t=1s   : [🟢 START pressionado]
t=1s   : state=WORK, workSeconds=0
t=2s   : state=WORK, workSeconds=1
t=10s  : state=WORK, workSeconds=9
t=11s  : [🔴 PAUSE pressionado]
t=11s  : state=PAUSED, workSeconds=10
t=20s  : state=PAUSED, workSeconds=10 (não incrementa)
t=21s  : [🟢 RESUME pressionado]
t=21s  : state=WORK, workSeconds=10
t=30s  : state=WORK, workSeconds=19
t=31s  : [🔴 STOP pressionado]
t=31s  : state=IDLE, workSeconds=20, cycles=1
```

---

## 🔌 API Reference

### Base URL

```
http://SEU_IP:1026/v2
```

### Headers Obrigatórios

```
fiware-service: smart
fiware-servicepath: /
```

---

### Endpoints

#### 1. Listar Todas as Entidades

```http
GET /entities
```

**Resposta:** Array de entidades

---

#### 2. Consultar Entidade Específica

```http
GET /entities/Productivity:productivity001
```

**Query Parameters:**
- `type=Productivity` (opcional)
- `attrs=state,workSeconds` (filtrar atributos)

---

#### 3. Consultar Apenas um Atributo

```http
GET /entities/Productivity:productivity001/attrs/state
```

**Resposta:**
```json
{
  "type": "Text",
  "value": "WORK",
  "metadata": {}
}
```

---

#### 4. Atualizar Atributos (Enviar Comando)

```http
PATCH /entities/urn:ngsi-ld:Productivity:001/attrs
Content-Type: application/json
```

**Body:**
```json
{
  "start": {
    "value": ""
  }
}
```

---

#### 5. Histórico Temporal (STH-Comet)

```http
GET http://SEU_IP:8666/STH/v1/contextEntities/type/Productivity/id/Productivity:productivity001/attributes/workSeconds?lastN=100
fiware-service: smart
fiware-servicepath: /
```

**Resposta:** Array com histórico dos últimos 100 valores

---

## 🐛 Troubleshooting

### Problema: ESP32 não conecta ao WiFi

**Sintomas:**
- Serial Monitor mostra "Conectando ao WiFi..." infinitamente
- Display mostra "Inicializando..." sem atualizar

**Soluções:**
1. Verifique SSID e senha no código
2. Verifique se WiFi está em 2.4GHz (ESP32 não suporta 5GHz)
3. Aproxime ESP32 do roteador
4. Teste com hotspot do celular

---

### Problema: MQTT não conecta

**Sintomas:**
- Serial Monitor: "Falha MQTT, rc=-2"
- Display não atualiza após conectar WiFi

**Códigos de Erro:**
| Código | Significado | Solução |
|--------|-------------|---------|
| `-4` | Connection timeout | Firewall bloqueando porta 1883 |
| `-2` | Connect failed | IP ou porta incorretos |
| `3` | Server unavailable | FIWARE não está rodando |
| `5` | Not authorized | Credenciais inválidas |

**Soluções:**
1. Verificar se FIWARE está rodando: `sudo docker ps`
2. Testar conexão: `telnet SEU_IP 1883`
3. Ver logs Mosquitto: `sudo docker logs fiware_mosquitto -f`
4. Verificar firewall: `sudo ufw status`

---

### Problema: Dados não aparecem no Orion

**Sintomas:**
- ESP32 conectado ao MQTT
- Serial mostra "Publish attrs [...] OK"
- GET retorna 404 Not Found

**Soluções:**
1. Verificar provisionamento:
   ```http
   GET http://SEU_IP:4041/iot/devices/productivity001
   fiware-service: smart
   fiware-servicepath: /
   ```

2. Ver logs IoT Agent:
   ```bash
   sudo docker logs fiware_iot-agent -f
   ```

3. Verificar tópico MQTT:
   ```bash
   mosquitto_sub -h SEU_IP -t "/ul/#" -v
   ```

4. Reprovisionar dispositivo (DELETE + POST novamente)

---

### Problema: Comandos não chegam no ESP32

**Sintomas:**
- PATCH retorna 204 No Content
- ESP32 não reage ao comando

**Soluções:**
1. Verificar subscrição MQTT no Serial Monitor:
   ```
   Assinado em: /4jggokgpepnvsb2uv4s40d59ov/productivity001/cmd
   ```

2. Monitorar tópico de comandos:
   ```bash
   mosquitto_sub -h localhost -t "/+/+/cmd" -v
   ```

3. Verificar formato do comando no Orion:
   ```http
   GET /entities/urn:ngsi-ld:Productivity:001
   ```
   Deve ter atributos `start_status`, `pause_status`, etc.

---

### Problema: Display OLED não funciona

**Sintomas:**
- Serial Monitor funciona
- Display fica preto

**Soluções:**
1. Verificar endereço I2C:
   ```cpp
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C)  // Tente 0x3D se não funcionar
   ```

2. Verificar ligações:
   - SDA → GPIO 21
   - SCL → GPIO 22
   - VCC → 3.3V (não 5V!)
   - GND → GND

3. Testar com código exemplo da Adafruit

---

## 🗺️ Roadmap

### Versão 1.0 (Atual) ✅
- [x] Controle local via botões
- [x] Integração FIWARE completa
- [x] Display OLED com status
- [x] Comandos remotos via API
- [x] Persistência de dados

### Versão 1.1 (Próxima)
- [ ] Dashboard web em tempo real
- [ ] Notificações via subscrições FIWARE
- [ ] Suporte a múltiplos dispositivos
- [ ] Exportar dados históricos (CSV)

### Versão 2.0 (Futuro)
- [ ] Integração com Grafana
- [ ] Machine Learning para previsão de produtividade
- [ ] App mobile (React Native)
- [ ] Modo Pomodoro (25min trabalho / 5min pausa)
- [ ] Relatórios semanais automáticos

---

## 🤝 Contribuindo

Contribuições são bem-vindas! Para contribuir:

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

### Diretrizes

- Siga o padrão de código existente
- Adicione testes para novas funcionalidades
- Atualize a documentação
- Descreva mudanças no Pull Request

---

## 📚 Referências e Links Úteis

### FIWARE
- [FIWARE Documentation](https://fiware-tutorials.readthedocs.io/)
- [Orion Context Broker](https://fiware-orion.readthedocs.io/)
- [IoT Agent UltraLight](https://fiware-iotagent-ul.readthedocs.io/)
- [FIWARE Descomplicado (Portuguese)](https://github.com/fabiocabrini/fiware)

### ESP32
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [PubSubClient Library](https://pubsubclient.knolleary.net/)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)

### MQTT
- [MQTT.org](https://mqtt.org/)
- [Eclipse Mosquitto](https://mosquitto.org/)
- [HiveMQ MQTT Essentials](https://www.hivemq.com/mqtt-essentials/)

### Tutoriais
- [FIWARE Step-by-Step](https://fiware-tutorials.readthedocs.io/en/latest/index.html)
- [ESP32 com MQTT](https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/)
- [Wokwi Simulator](https://wokwi.com/)

---

## 📄 Licença

Este projeto está licenciado sob a Licença MIT - veja o arquivo [LICENSE](LICENSE) para detalhes.

```
MIT License

Copyright (c) 2025 DevBalance Team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 📞 Contato

**DevBalance Team**

- 📧 Email: devbalance@fiap.com.br
- 💼 LinkedIn: [DevBalance](https://linkedin.com/company/devbalance)
- 🐙 GitHub: [@devbalance](https://github.com/devbalance)

---

<div align="center">

**Desenvolvido com ❤️ pela equipe DevBalance**

[⬆ Voltar ao topo](#-devbalance---sistema-de-rastreamento-de-produtividade-iot)

</div>
