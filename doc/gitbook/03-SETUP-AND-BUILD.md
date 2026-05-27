# Setup e Build

## Objetivo

Esta pagina deve permitir que uma pessoa monte o ambiente e gere um build sem depender de ajuda oral.

## Pre-requisitos

Preencher:

- Sistema operacional suportado:
- Toolchain necessaria:
- Dependencias externas:
- Credenciais ou acessos especiais:

## Passo a passo de setup

1. Clonar o repositorio
2. Inicializar submodulos, se houver
3. Configurar toolchain
4. Executar script de configuracao
5. Gerar build

## Comandos principais

Registrar aqui os comandos reais usados pelo time.

```bash
# Exemplo
./configure.sh
cmake -S . -B build
cmake --build build
```

## Variantes de build

Documente diferencas entre:

- debug
- release
- build por chipset
- build com ou sem componentes opcionais

## Saidas esperadas

- Onde o binario final e gerado
- Onde ficam artefatos intermediarios
- Como identificar que o build terminou corretamente

## Problemas comuns de setup

- Dependencia ausente
- Toolchain incorreta
- Erro de permissao
- Variavel de ambiente nao configurada
