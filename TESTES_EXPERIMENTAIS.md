# Testes experimentais — firmware de energia

O script `tests/validar_experimento.py` analisa mensagens JSON capturadas no tópico MQTT `energia`. Ele verifica os campos `tensao`, `corrente`, `potencia`, `energiaKwh`, `frequencia`, `fatorPotencia` e `esp32Mac`.

## Como executar

No Windows, com Python 3 instalado:

```powershell
py tests\validar_experimento.py --autoteste
mosquitto_sub -h HOST_DO_BROKER -p 1883 -t energia > captura.jsonl
py tests\validar_experimento.py --arquivo captura.jsonl --saida resultado.json
```

Substitua `HOST_DO_BROKER` pelo host MQTT usado na execução. Pare a captura com Ctrl+C antes de analisar o arquivo. Registre o número da execução, o intervalo de coleta e as condições do teste.

O relatório separa mensagens válidas, JSON inválido, campos incorretos e mensagens de erro publicadas pelo firmware. O autoteste valida somente o script Python, não o ESP32 nem a precisão elétrica. Faça cinco repetições se isso for viável e registre os resultados observados, sem preencher valores estimados como se fossem medidos.
