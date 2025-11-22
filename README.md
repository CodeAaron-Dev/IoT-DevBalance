# DevBalance - Sistema de Rastreamento de Produtividade IoT

Sistema de monitoramento de produtividade em tempo real utilizando **ESP32**, **MQTT** e **FIWARE** para coleta, armazenamento e visualização de dados de sessões de trabalho.

---

## 👥 Equipe

| Nome | RM |
|------|-----|
| Cesar Aaron Herrera | 565398 |
| Rafael Seiji Aoke Arakaki | 561993 |
| Rafael Yuji Nakaya | 563624 |

**Disciplina**: Edge Computing and Computer Systems - FIAP  
**Professor**: Fábio Cabrini

---

## 📋 Índice

- [Sobre o Projeto](#-sobre-o-projeto)
- [Arquitetura](#-arquitetura)
- [Hardware](#-hardware)
- [Dashboard em Tempo Real](#-dashboard-em-tempo-real)
- [FIWARE - O que Usamos](#-fiware---o-que-usamos)
- [Instalação](#-instalação)
- [Como Usar](#-como-usar)
- [API e Dados](#-api-e-dados)
- [Aplicações Futuras](#-aplicações-futuras)

---

## 📖 Sobre o Projeto

**DevBalance** é um sistema IoT que rastreia tempo de trabalho e produtividade utilizando um dispositivo ESP32 conectado à plataforma FIWARE.

### Funcionalidades Implementadas

- ⏱️ **Cronômetro de trabalho** com precisão de 1 segundo
- 🎮 **Controle por botões físicos** (Start/Stop, Pause/Resume)
- 📟 **Display OLED** mostrando status em tempo real
- 📡 **Envio de dados via MQTT** a cada 5 segundos
- 💾 **Armazenamento no FIWARE** (Orion Context Broker)
- 🔄 **Estados**: IDLE (parado), WORK (trabalhando), PAUSED (pausado)
- 📊 **Contagem de ciclos** completos de trabalho
- 🌐 **Controle remoto** via API REST

### O que o Sistema Faz

1. **ESP32 coleta dados localmente**: estado atual, tempo de trabalho, ciclos completados
2. **Publica via MQTT** no formato UltraLight 2.0
3. **IoT Agent traduz** MQTT → NGSI (padrão FIWARE)
4. **Orion armazena** os dados com timestamp
5. **API REST disponibiliza** para consultas e comandos
6. **Dashboard consome** os dados para visualização

---

## 🏗️ Arquitetura

### Diagrama Completo da Solução

<img width="992" height="961" alt="FiwareDeploy_new_v5" src="https://github.com/user-attachments/assets/ab4fdb72-aac2-4185-9090-606a0d16c9dd" />


A arquitetura segue o modelo **FIWARE Descomplicado** proposto pelo Professor Fábio Cabrini:

```
┌─────────────┐
│   ESP32     │  Publica dados via MQTT (porta 1883)
│   + OLED    │  Formato: state|WORK|work_seconds|120|...
│   + Botões  │
└──────┬──────┘
       │ MQTT
       ▼
┌─────────────┐
│  Mosquitto  │  Broker MQTT
│    (1883)   │  Roteia mensagens
└──────┬──────┘
       │
       ▼
┌─────────────┐
│ IoT Agent   │  Traduz UltraLight → NGSI v2
│    (4041)   │  Mapeia atributos do dispositivo
└──────┬──────┘
       │ HTTP/NGSI
       ▼
┌─────────────┐
│   Orion     │  Armazena entidades (contexto)
│   (1026)    │  API REST para consultas
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  Postman    │  Consulta dados via GET
│  Dashboard  │  Envia comandos via PATCH
└─────────────┘
```

### Portas Utilizadas

| Serviço | Porta | Protocolo | Função |
|---------|-------|-----------|--------|
| Mosquitto | 1883 | MQTT | Recebe dados do ESP32 |
| IoT Agent | 4041 | HTTP | Provisiona dispositivos |
| Orion | 1026 | HTTP | API REST (NGSI v2) |
| STH-Comet | 8666 | HTTP | Histórico (futuro) |

---

## 🔌 Hardware

### Componentes

| Item | Especificação | Quantidade |
|------|--------------|------------|
| ESP32 DevKit v1 | Microcontrolador WiFi | 1 |
| Display OLED | SSD1306 128x64 I2C | 1 |
| Push Button | Tact switch 6x6mm | 2 |
| Protoboard | 830 pontos | 1 |
| Jumpers | Macho-Macho | 10 |

### Conexões

| Componente | Pino | ESP32 GPIO |
|------------|------|-----------|
| OLED VCC | VCC | 3.3V |
| OLED GND | GND | GND |
| OLED SDA | SDA | GPIO 21 |
| OLED SCL | SCL | GPIO 22 |
| Botão Verde | Terminal 1 | GPIO 5 |
| Botão Verde | Terminal 2 | GND |
| Botão Vermelho | Terminal 1 | GPIO 4 |
| Botão Vermelho | Terminal 2 | GND |

> **Nota**: Botões usam pull-up interno do ESP32 (configurado por software)

### Foto do Projeto

<img width="514" height="384" alt="Captura de tela 2025-11-22 133525" src="https://github.com/user-attachments/assets/871f82c1-5fe9-4d53-80dc-cb14146d6e13" />



**Simulação Online**: [Testar no Wokwi](https://wokwi.com/projects/440265854761066497)

---

## 🌐 FIWARE - O que Usamos

O **FIWARE** é uma plataforma open-source para desenvolvimento de soluções IoT e cidades inteligentes. Utilizamos os seguintes componentes:

### 1. Orion Context Broker (Porta 1026)

**O que é**: Banco de dados de contexto em tempo real baseado no padrão NGSI v2.

**O que usamos**:
- Armazenamento de entidades (nosso dispositivo `productivity001`)
- API REST para consultas (`GET /v2/entities`)
- API REST para comandos (`PATCH /v2/entities/.../attrs`)
- Timestamp automático (`TimeInstant`)

- **Protocolo UltraLight 2.0**:
Formato simplificado chave|valor separados por |

Exemplo:
state|WORK|work_seconds|120|cycles_completed|3|pause_count|1 É traduzido para NGSI:
{
  "state": {"value": "WORK"},
  "work_seconds": {"value": "120"},
  "cycles_completed": {"value": "3"},
  "pause_count": {"value": "1"} }

**Como funciona no projeto**:
```
Orion recebe do IoT Agent:
{
  "state": {"type": "Text", "value": "WORK"},
  "workSeconds": {"type": "Text", "value": "120"}
}

Armazena como entidade:
{
  "id": "Productivity:productivity001",
  "type": "Productivity",
  "state": {...},
  "workSeconds": {...},
  "TimeInstant": "2025-11-22T15:30:45.123Z"
  "pause_count": {...}
}
```

### 2. IoT Agent MQTT - UltraLight (Porta 4041)

**O que é**: Tradutor entre protocolo MQTT (usado por dispositivos IoT) e NGSI v2 (usado pelo Orion).

**O que usamos**:
- Provisionamento de dispositivos (registrar o ESP32)
- Tradução de payloads UltraLight → NGSI
- Gerenciamento de comandos NGSI → MQTT

**Protocolo UltraLight 2.0**:
```
Formato simplificado chave|valor separados por |

Exemplo:
state|WORK|work_seconds|120|cycles_completed|3

É traduzido para NGSI:
{
  "state": {"value": "WORK"},
  "work_seconds": {"value": "120"},
  "cycles_completed": {"value": "3"}
}
```

**Mapeamento de Atributos**:
| Dado do ESP32 | Nome no Orion | Tipo |
|---------------|---------------|------|
| `state` | `state` | Text |
| `work_seconds` | `workSeconds` | Text |
| `cycles_completed` | `cyclesCompleted` | Text |
| `uptime_seconds` | `uptimeSeconds` | Text |

### 3. Eclipse Mosquitto (Porta 1883)

**O que é**: Broker MQTT leve e eficiente.

**O que usamos**:
- Receber publicações do ESP32
- Rotear para o IoT Agent
- Enviar comandos do IoT Agent para o ESP32

**Tópicos MQTT**:
```
Publicação (ESP32 → Cloud):
/ul/4jggokgpepnvsb2uv4s40d59ov/productivity001/attrs

Comandos (Cloud → ESP32):
/4jggokgpepnvsb2uv4s40d59ov/productivity001/cmd

Confirmação (ESP32 → Cloud):
/4jggokgpepnvsb2uv4s40d59ov/productivity001/cmdexe
```

### 4. MongoDB

**O que é**: Banco de dados NoSQL usado pelo Orion.

**O que usamos**:
- Armazenamento persistente das entidades
- Histórico de atualizações (via `TimeInstant`)

> **Nota**: Não acessamos o MongoDB diretamente, apenas via API do Orion.

### Fluxo Completo de Dados

```
1. ESP32 lê estado: WORK, 120 segundos trabalhados
   ↓
2. Monta payload UltraLight:
   state|WORK|work_seconds|120|cycles_completed|2
   ↓
3. Publica via MQTT no tópico /ul/.../attrs
   ↓
4. Mosquitto roteia para IoT Agent
   ↓
5. IoT Agent traduz UltraLight → NGSI v2:
   {"state": {"type": "Text", "value": "WORK"}, ...}
   ↓
6. IoT Agent envia HTTP POST/PATCH para Orion
   ↓
7. Orion atualiza entidade no MongoDB
   ↓
8. Postman faz GET e recebe:
   {
     "id": "Productivity:productivity001",
     "state": {"value": "WORK"},
     "workSeconds": {"value": "120"},
     "TimeInstant": "2025-11-22T15:30:45.123Z"
   }
```

---

## 🚀 Instalação

### Pré-requisitos

**Servidor (VM Azure)**:
- Ubuntu Server 20.04+
- 2GB RAM
- Docker e Docker Compose instalados
- Portas 1883, 1026, 4041 liberadas

**ESP32**:
- Arduino IDE 1.8.19+
- Bibliotecas: `PubSubClient`, `Adafruit_GFX`, `Adafruit_SSD1306`

---

### Passo 1: Configurar FIWARE na VM

#### 1.1 Clonar Repositório

```bash
git clone https://github.com/fabiocabrini/fiware
cd fiware
```

#### 1.2 Iniciar Containers

```bash
sudo docker-compose up -d
```

#### 1.3 Verificar Status

```bash
sudo docker ps
```

Deve mostrar 5 containers rodando:
- `fiware_orion`
- `fiware_iot-agent`
- `fiware_mosquitto`
- `fiware_mongo`
- `fiware_sth-comet`

---

### Passo 2: Provisionar Dispositivo (Postman)

#### 2.1 Criar Service Group

```http
POST http://SEU_IP:4041/iot/services
Content-Type: application/json
fiware-service: smart
fiware-servicepath: /
```

**Body**:
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

#### 2.2 Registrar ESP32

```http
POST http://SEU_IP:4041/iot/devices
Content-Type: application/json
fiware-service: smart
fiware-servicepath: /
```

**Body**:
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
        {"object_id": "state", "name": "state", "type": "Text"},
        {"object_id": "work_seconds", "name": "workSeconds", "type": "Text"},
        {"object_id": "cycles_completed", "name": "cyclesCompleted", "type": "Text"},
        {"object_id": "uptime_seconds", "name": "uptimeSeconds", "type": "Text"}
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

---

### Passo 3: Configurar ESP32

#### 3.1 Editar Código

Abra `DevBalance.ino` e configure:

```cpp
// WiFi
const char* WIFI_SSID     = "SUA_REDE";
const char* WIFI_PASSWORD = "SUA_SENHA";

// MQTT
const char* MQTT_SERVER = "IP_DA_VM_AZURE";
const int   MQTT_PORT   = 1883;
```

#### 3.2 Upload

1. Conecte ESP32 via USB
2. Selecione placa: **ESP32 Dev Module**
3. Clique em **Upload**
4. Abra **Serial Monitor** (115200 baud)

---

## 📖 Como Usar

### Controle Local (Botões)

**Botão Verde (GPIO 5)**:
- `IDLE` → Clique → `WORK` (inicia trabalho)
- `PAUSED` → Clique → `WORK` (retoma trabalho)

**Botão Vermelho (GPIO 4)**:
- `WORK` → Clique → `PAUSED` (pausa trabalho)
- `PAUSED` ou `WORK` → Clique → `IDLE` (para e conta ciclo)

### Estados

```
IDLE (Parado)
  └─ Botão Verde → WORK

WORK (Trabalhando - incrementa segundos)
  ├─ Botão Vermelho → PAUSED
  └─ Botão Vermelho (2x) → IDLE (ciclo +1)

PAUSED (Pausado - mantém tempo)
  ├─ Botão Verde → WORK
  └─ Botão Vermelho → IDLE (ciclo +1)
```

### Controle Remoto (API)

#### Consultar Dados

```http
GET http://SEU_IP:1026/v2/entities/Productivity:productivity001
fiware-service: smart
fiware-servicepath: /
```

**Resposta**:
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
  "TimeInstant": {
    "type": "DateTime",
    "value": "2025-11-22T15:30:45.123Z",
    "metadata": {}
  }
}
```

#### Enviar Comandos

**Template**:
```http
PATCH http://SEU_IP:1026/v2/entities/urn:ngsi-ld:Productivity:001/attrs
Content-Type: application/json
fiware-service: smart
fiware-servicepath: /
```

**Iniciar trabalho**:
```json
{"start": {"value": ""}}
```

**Pausar**:
```json
{"pause": {"value": ""}}
```

**Retomar**:
```json
{"resume": {"value": ""}}
```

**Parar (ciclo +1)**:
```json
{"stop": {"value": ""}}
```

**Reset**:
```json
{"reset": {"value": ""}}
```

---

## 📊 API e Dados

### Endpoints Principais

| Método | Endpoint | Descrição |
|--------|----------|-----------|
| `GET` | `/v2/entities` | Listar todas entidades |
| `GET` | `/v2/entities/{id}` | Consultar entidade específica |
| `GET` | `/v2/entities/{id}/attrs/{attr}` | Consultar atributo |
| `PATCH` | `/v2/entities/{id}/attrs` | Atualizar atributos (comandos) |

### Dados Coletados

| Atributo | Tipo | Descrição | Atualização |
|----------|------|-----------|-------------|
| `state` | Text | IDLE, WORK ou PAUSED | Mudança de estado |
| `workSeconds` | Text | Tempo trabalhando (segundos) | A cada 1s em WORK |
| `cyclesCompleted` | Text | Quantidade de ciclos | Transição para IDLE |
| `uptimeSeconds` | Text | Tempo desde boot (segundos) | A cada 3s |
| `TimeInstant` | DateTime | Timestamp da atualização | Automático |
| `PauseCount` | Text | Quantidade de pausas | a cada clique de saida após o botão vermelho ser pressionado |

### Exemplo de Evolução

```
t=0s  : state=IDLE, workSeconds=0, cycles=0
        [Botão Verde pressionado]

t=1s  : state=WORK, workSeconds=0
t=2s  : state=WORK, workSeconds=1
t=10s : state=WORK, workSeconds=9
        [Botão Vermelho pressionado]

t=11s : state=PAUSED, workSeconds=10
t=20s : state=PAUSED, workSeconds=10 (mantém)
        [Botão Verde pressionado]

t=21s : state=WORK, workSeconds=10
t=30s : state=WORK, workSeconds=19
        [Botão Vermelho 2x pressionado]

t=31s : state=IDLE, workSeconds=20, cycles=1
```

---

## 🔮 Aplicações Futuras

### Curto Prazo

- [ ] **Dashboard Web em Tempo Real**
  - Gráficos de produtividade (Chart.js)
  - Atualização via polling ou WebSocket
  - Exportar relatórios em PDF

  integrado com sucesso!

  - Dashboard com atualização real:
  

<img width="1608" height="994" alt="Captura de tela 2025-11-22 161925" src="https://github.com/user-attachments/assets/3ff9289d-74cd-4eb4-bbf3-f43342c8be3b" />


- [ ] **Integração STH-Comet**
  - Consultar histórico: `GET http://IP:8666/STH/v1/...`
  - Gerar gráficos de tendência
  - Análise de produtividade por período

- [ ] **Notificações Push**
  - Subscrições FIWARE para alertas
  - Integração com Telegram/Discord
  - Avisos de pausas longas

### Médio Prazo

- [ ] **App Mobile**
  - React Native ou Flutter
  - Controle remoto via smartphone
  - Sincronização com calendário

- [ ] **Modo Pomodoro**
  - Timer de 25min trabalho / 5min pausa
  - Notificações automáticas
  - Estatísticas de foco

- [ ] **Múltiplos Dispositivos**
  - Suporte a vários ESP32
  - Dashboard consolidado
  - Comparação de produtividade

### Longo Prazo

- [ ] **Machine Learning**
  - Previsão de produtividade
  - Detecção de padrões
  - Recomendações personalizadas

- [ ] **Integração com Ferramentas**
  - Google Calendar
  - Trello/Jira
  - Slack/Teams

- [ ] **Relatórios Avançados**
  - Análise semanal/mensal
  - Comparação com metas
  - Insights de produtividade

---

## 🛠️ Troubleshooting

### ESP32 não conecta ao WiFi

**Sintoma**: "Conectando ao WiFi..." infinito

**Solução**:
- Verifique SSID e senha
- Use WiFi 2.4GHz (ESP32 não suporta 5GHz)
- Teste com hotspot do celular

### MQTT não conecta

**Sintoma**: "Falha MQTT, rc=-2"

**Solução**:
```bash
# Na VM, verificar Mosquitto
sudo docker logs fiware_mosquitto

# Testar porta
telnet SEU_IP 1883
```

### Dados não aparecem no Orion

**Sintoma**: GET retorna 404

**Solução**:
1. Verificar provisionamento:
```http
GET http://SEU_IP:4041/iot/devices/productivity001
```

2. Ver logs IoT Agent:
```bash
sudo docker logs fiware_iot-agent -f
```

3. Monitorar MQTT:
```bash
mosquitto_sub -h SEU_IP -t "/ul/#" -v
```

---

## 📚 Referências

- [FIWARE Documentation](https://fiware-tutorials.readthedocs.io/)
- [FIWARE Descomplicado - Prof. Fábio Cabrini](https://github.com/fabiocabrini/fiware)
- [Orion Context Broker](https://fiware-orion.readthedocs.io/)
- [IoT Agent UltraLight](https://fiware-iotagent-ul.readthedocs.io/)
- [NGSI v2 Specification](https://fiware.github.io/specifications/ngsiv2/stable/)

---

## 🙏 Agradecimentos

- **Professor Fábio Cabrini** pelo repositório FIWARE Descomplicado e orientações
- **FIAP** pela infraestrutura e suporte
- **Comunidade FIWARE** pela documentação e exemplos

---

<div align="center">

**Desenvolvido por DevBalance Team - FIAP 2025**

[⬆ Voltar ao topo](#devbalance---sistema-de-rastreamento-de-produtividade-iot)

</div>
