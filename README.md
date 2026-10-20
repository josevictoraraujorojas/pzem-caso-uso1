# Monitoramento de Energia — Caso de Uso 1

Firmware desenvolvido com **ESP-IDF** para aquisição de dados elétricos de um módulo **PZEM-004T v3.0** e envio das medições por MQTT. O projeto faz parte do TCC sobre integração de dispositivos IoT com bancos de dados de séries temporais e bancos orientados a grafos no IF Goiano – Campus Urutaí.

## Objetivo

Monitorar grandezas elétricas de um equipamento do LabMaker, permitindo coletar leituras periodicamente e encaminhá-las à aplicação responsável pelo recebimento e armazenamento dos dados.

## Como funciona

1. O ESP32 inicializa a comunicação serial com o PZEM-004T.
2. Conecta-se à rede Wi-Fi configurada no firmware.
3. Após obter conexão de rede, inicia o cliente MQTT.
4. A cada **5 segundos**, consulta o módulo e publica um objeto JSON no tópico `energia`.
5. O identificador MAC do ESP32 acompanha as medições para ajudar a reconhecer o dispositivo de origem.

## Dados publicados

O payload pode conter os seguintes campos, conforme a leitura do módulo:

| Campo | Descrição |
|---|---|
| `tensao` | Tensão elétrica |
| `corrente` | Corrente elétrica |
| `potencia` | Potência ativa |
| `energiaKwh` | Energia acumulada em kWh |
| `frequencia` | Frequência da rede |
| `fatorPotencia` | Fator de potência |
| `esp32Mac` | Endereço MAC do ESP32 que realizou a leitura |

Exemplo ilustrativo de mensagem:

```json
{
  "tensao": 127.0,
  "corrente": 3.5,
  "potencia": 420.0,
  "energiaKwh": 1.25,
  "frequencia": 60.0,
  "fatorPotencia": 0.95,
  "esp32Mac": "AA:BB:CC:DD:EE:FF"
}
```

Os valores acima são apenas ilustrativos, não representam uma medição real.

## Hardware e comunicação

- **Microcontrolador:** ESP32
- **Medidor:** PZEM-004T v3.0, com transformador de corrente compatível
- **Interface com o medidor:** UART1
- **GPIO do ESP32:** TX = GPIO 17; RX = GPIO 16
- **Velocidade serial:** 9600 baud
- **Endereço Modbus usado pelo módulo:** `0xF8`
- **Rede e mensageria:** Wi-Fi e MQTT

Confira a pinagem e a tensão elétrica dos módulos antes de conectar o circuito. A medição de rede elétrica deve ser montada com os cuidados e isolamentos apropriados.

## Requisitos

- Placa ESP32 compatível com o firmware
- ESP-IDF instalado e configurado
- Módulo PZEM-004T v3.0 e ligações adequadas
- Rede Wi-Fi disponível
- Broker MQTT acessível pelo ESP32

## Configuração

Antes de compilar, confira no código as configurações de Wi-Fi e o endereço/porta do broker MQTT. Ajuste-as para o ambiente em que o dispositivo será executado. Não publique senhas ou credenciais reais no repositório.

O firmware usa o tópico MQTT `energia` para publicar as medições e realiza o registro do dispositivo no tópico `monitorEnergia`, conforme a implementação atual.

## Compilação e gravação

No terminal configurado com o ESP-IDF, na raiz do projeto:

```bash
idf.py set-target esp32
idf.py build
idf.py -p PORTA flash monitor
```

Substitua `PORTA` pela porta serial da placa, por exemplo `COM5` no Windows ou `/dev/ttyUSB0` no Linux. Se o alvo já estiver configurado, não é necessário executar novamente `set-target`.

## Papel no TCC

Este firmware representa a etapa de aquisição do caso de uso de monitoramento de energia. Ele envia as leituras para a infraestrutura MQTT, que pode encaminhá-las à API e aos componentes de armazenamento do sistema.

## Observações

- A publicação depende da conexão Wi-Fi e da disponibilidade do broker MQTT.
- O formato do JSON deve permanecer compatível com o consumidor MQTT/API.
- Os valores do exemplo são fictícios e servem apenas para demonstrar a estrutura da mensagem.
