# Testes e QA

## Objetivo

Explicar como validar mudancas e reduzir regressao.

## Tipos de validacao

- build local
- smoke test
- validacao funcional
- regressao dirigida
- teste em hardware especifico

## O que toda entrega deveria responder

- O que mudou?
- Como reproduzir o problema original?
- Como provar que a correcao funcionou?
- O que pode ter sido afetado colateralmente?

## Roteiro minimo para bugfix

1. Reproduzir a falha
2. Registrar ambiente
3. Aplicar a correcao
4. Validar o caso original
5. Validar fluxos proximos

## Evidencias recomendadas

- log
- video
- print
- versao de build
- base ou lista usada no teste

## Gaps conhecidos

Use esta secao para listar ausencias do processo atual:

- falta de teste automatizado
- dependencia de bancada
- ausencia de massa de teste padronizada
