# Procedimento experimental — monitoramento de energia

Este procedimento usa os logs do próprio firmware e do backend. O arquivo Python auxiliar foi removido; a evidência principal é a mensagem registrada em cada etapa do fluxo real.

## 1. Abrir o monitor serial do ESP32

No terminal ESP-IDF, na raiz do firmware:

```powershell
idf.py -p COM5 monitor
```

Troque `COM5` pela porta da placa. Para cada publicação, procure:

```text
[EXPERIMENTO][MQTT_ENVIO] topico=energia mensagem_id=... payload={...}
```

O payload mostra os valores enviados pelo PZEM. Se aparecer `[EXPERIMENTO][MQTT_ENVIO_FALHA]`, a publicação não foi aceita localmente pelo cliente MQTT.

## 2. Conferir o que chegou ao broker

Em outro terminal com o cliente Mosquitto instalado:

```powershell
mosquitto_sub -h HOST_DO_BROKER -p 1883 -t energia -v
```

Substitua `HOST_DO_BROKER` pelo IP/host do broker configurado no dispositivo. O terminal deve exibir o tópico e o JSON recebido pelo broker. Essa etapa ajuda a distinguir a tentativa de envio do ESP32 da mensagem realmente observada no broker.

## 3. Conferir o recebimento e a persistência no backend

No console da aplicação Java do Caso de Uso 1, procure as linhas com a mesma identificação `id` do backend:

- `[EXPERIMENTO][MQTT_RECEBIDO]`: mensagem chegou ao listener do backend.
- `[EXPERIMENTO][INFLUX_INICIO]`: começou a gravação no InfluxDB.
- `[EXPERIMENTO][INFLUX_PERSISTIDO]`: a chamada síncrona de gravação retornou sem erro; o log inclui os campos e as associações gravadas.
- `[EXPERIMENTO][FALHA_PROCESSAMENTO]` ou `[EXPERIMENTO][INFLUX_FALHA]`: houve erro na etapa indicada.

O backend gera um identificador interno para cada mensagem recebida. Compare o payload emitido pelo dispositivo com o payload registrado pelo backend; o identificador interno do backend não é inserido no JSON MQTT original.

## 4. Confirmar no InfluxDB

No Data Explorer do InfluxDB, selecione o bucket `caso_uso_1`, o measurement `energia` e o mesmo intervalo de tempo do experimento. Compare os campos com a linha `INFLUX_PERSISTIDO`. A linha do log é uma evidência da chamada de gravação bem-sucedida; a consulta ao banco confirma que o registro pode ser recuperado.

## Repetições e registro dos resultados

Faça repetições em condições anotadas e registre hora inicial/final, mensagens enviadas pelo ESP32, mensagens vistas no broker, mensagens recebidas pelo backend, gravações bem-sucedidas e erros. Não preencha resultados antes de executar o teste. O log do ESP32 confirma a tentativa/resultado local do envio MQTT, mas não comprova sozinho a persistência final.


## Limite do ensaio e salvamento dos logs

O firmware PZEM limita o ensaio a 100 publicações de dados no tópico `energia`. Ao alcançar o limite, registra `[EXPERIMENTO][FIM]` e encerra a tarefa de publicação. O contador registra publicações aceitas localmente pela chamada MQTT; não prova que todas as mensagens chegaram ao broker ou foram persistidas.

Para gravar a saída serial em um arquivo no computador, abra o PowerShell na pasta do projeto e execute (ajuste a porta serial conforme necessário):

```powershell
idf.py -p COM5 flash monitor 2>&1 | Tee-Object -FilePath experimento-pzem.log
```

O arquivo `experimento-pzem.log` será criado na pasta atual. Se o firmware já estiver gravado, use `idf.py -p COM5 monitor 2>&1 | Tee-Object -FilePath experimento-pzem.log` para não regravar a placa. Guarde o arquivo de cada execução separadamente e não reutilize o mesmo nome se quiser preservar o ensaio anterior.

Para salvar também as mensagens observadas no broker, em outro PowerShell execute:

```powershell
mosquitto_sub -h HOST_DO_BROKER -p 1883 -t energia -v 2>&1 | Tee-Object -FilePath experimento-broker.log
```

Substitua `HOST_DO_BROKER` pelo host configurado no firmware. O arquivo do broker deve ser separado do log serial.
