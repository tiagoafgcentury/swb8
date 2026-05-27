# Troubleshooting

## Objetivo

Centralizar problemas recorrentes, hipoteses provaveis e a primeira resposta esperada do time.

## Modelo de registro

Para cada problema, documente:

- Sintoma
- Contexto
- Causa provavel
- Como investigar
- Workaround, se houver
- Status

## Casos iniciais para preencher

### Sistema nao sobe

- verificar build;
- verificar ambiente;
- verificar logs iniciais;
- verificar watchdog ou reinicio automatico.

### Build falha

- validar toolchain;
- validar dependencia externa;
- validar scripts de configuracao.

### Funcao funciona em um chipset e falha em outro

- mapear diferencas de HAL;
- comparar flags de build;
- revisar dependencias especificas da plataforma.

### Bug nao reproduzivel

- revisar frequencia;
- revisar passos;
- pedir evidencias melhores;
- tentar reduzir variaveis do ambiente.

## Regra pratica

Se uma pessoa repetir a mesma explicacao duas vezes, isso ja merece entrar nesta pagina.
