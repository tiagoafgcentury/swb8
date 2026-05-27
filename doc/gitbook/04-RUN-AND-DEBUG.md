# Execucao e Debug

## Como executar

Descreva como rodar o sistema no ambiente disponivel:

- bancada
- emulacao, se existir
- hardware real
- ambiente de laboratorio

## Logs e sinais de vida

Registre:

- onde ficam logs;
- quais mensagens indicam inicializacao correta;
- como detectar travamento, reboot ou falha de modulo.

## Debug basico

Checklist inicial para qualquer incidente:

1. Confirmar build usada
2. Confirmar hardware e ambiente
3. Coletar log
4. Verificar passo a passo de reproducao
5. Identificar modulo suspeito

## Ferramentas uteis

Preencher:

- `gdb`
- scripts internos
- ferramentas de serial/telnet
- coletores de log

## Quando escalar

Escalone rapidamente quando houver:

- reboot recorrente
- corrupcao de dados
- travamento em watchdog
- falha intermitente sem log suficiente
