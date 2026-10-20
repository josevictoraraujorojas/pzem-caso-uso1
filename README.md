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

## Testes experimentais do TCC

Esta seção descreve como executar o teste do firmware e registrar evidências. O procedimento deve ser repetido em condições semelhantes e cada execução deve usar um arquivo de log próprio.

### Objetivo e fluxo avaliado

Verificar a aquisição das leituras do PZEM-004T e o envio das mensagens pelo tópico MQTT `energia`, observando os registros do ESP32, do broker e do backend do Caso de Uso 1, além de conferir os dados persistidos no InfluxDB.

### Preparação

1. Confira as credenciais Wi-Fi e o endereço/porta do broker MQTT no firmware.
2. Conecte o PZEM e o ESP32 conforme a seção de hardware. Identifique a porta serial da placa.
3. Inicie o broker MQTT, o InfluxDB, o Neo4j e o backend do [Caso de Uso 1](https://github.com/josevictoraraujorojas/caso-de-uso-1), com as configurações exigidas por cada serviço.
4. Abra um terminal ESP-IDF na raiz deste repositório.

### Execução e coleta dos logs

Compile e grave o firmware (substitua `COM5` pela porta correta):

```powershell
idf.py build
idf.py -p COM5 flash
Start-Transcript -Path "$HOME\TCC-Experimentos\01-pzem-serial.log"
idf.py -p COM5 monitor
```

Crie antes a pasta de resultados, se necessário: `New-Item -ItemType Directory -Force -Path "$HOME\TCC-Experimentos"`. Encerre o monitor com `Ctrl+]` e depois execute `Stop-Transcript` no PowerShell, caso o terminal retorne ao prompt. Para observar o tópico no broker em outro terminal, use:

```powershell
mosquitto_sub -h HOST_DO_BROKER -p 1883 -t energia -v |
  Tee-Object -FilePath "$HOME\TCC-Experimentos\05-mqtt-energia.log"
```

Substitua `HOST_DO_BROKER` pelo endereço real. Inicie o assinante antes do firmware; ele não recupera mensagens anteriores por padrão. Aguarde o marcador `[EXPERIMENTO][FIM]` do firmware.

### Como o teste foi instrumentado

O firmware registra marcadores com o prefixo `[EXPERIMENTO]`, incluindo tentativas/contagem de publicações e o término do envio. O limite experimental configurado é de até 100 publicações locais no tópico `energia`. Uma confirmação do cliente MQTT não comprova, sozinha, que o broker recebeu a mensagem; compare os logs do dispositivo, do assinante e do backend.

### Verificação e interpretação

- Conte os marcadores de envio no log serial e as mensagens observadas pelo assinante MQTT.
- No log do backend, procure `[EXPERIMENTO][MQTT_RECEBIDO]`, `[EXPERIMENTO][INFLUX_INICIO]`, `[EXPERIMENTO][INFLUX_PERSISTIDO]` e eventuais marcadores de falha.
- No InfluxDB, consulte o bucket `caso_uso_1` e a medição `energia`, ajustando o intervalo de tempo para a execução.
- Registre horário de início/fim, quantidade de mensagens em cada etapa, registros encontrados no banco e erros.

O resultado deve ser obtido por consulta ao banco; o marcador de persistência indica que a chamada de escrita não retornou erro, mas não substitui a conferência dos registros. Use um arquivo de log diferente para cada repetição.
