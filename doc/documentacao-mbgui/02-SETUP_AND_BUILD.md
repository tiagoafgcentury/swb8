# 02 - Setup, Build e Flags do Projeto

Este documento descreve o estado atual do build do MBGUI com base no codigo fonte disponivel neste workspace.

## Visao geral

O projeto usa CMake como sistema principal de build.

- Projeto: `mbgui`
- Versao declarada no CMake: `1.3.02.0`
- Versao minima do CMake: `3.22`
- Artefato principal: executavel `mbgui`
- Plataforma alvo: receptor embarcado Linux, com toolchains MIPS para ALi/Montage
- Interface: LVGL, com fontes, temas e recursos gerados/embarcados
- Integracoes principais: tuner/demux/player, banco SQLite, CAS/Nagra, EPG, middleware DVB e HALs

## Arquivos de build relevantes

- `CMakeLists.txt`: configuracao principal do projeto, opcoes globais e alvo `mbgui`.
- `src/CMakeLists.txt`: fontes do core, HALs, CAS, banco, DVB, player e modulos de aplicacao.
- `ui/lvgl/CMakeLists.txt`: integracao da interface LVGL.
- `toolchain_mips_ali.txt`: toolchain para builds ALi.
- `toolchain_mips_montage.txt`: toolchain para builds Montage.
- `configure.sh`, `configure-dl.sh`, `configure-nagra.sh`, `configure-release.sh`: scripts auxiliares de configuracao por perfil.
- `clean.sh`, `clean_release_files.sh`: scripts auxiliares de limpeza.
- `generate_release_seed.sh`: script usado no fluxo de release.

## Fluxo basico de build

O fluxo exato depende da maquina de build, toolchain instalada e perfil de release. Como referencia, o formato esperado e:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain_mips_montage.txt -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Para builds ALi, usar a toolchain correspondente:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain_mips_ali.txt -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Os scripts `configure*.sh` devem ser preferidos em ambientes oficiais de build, pois podem aplicar flags, paths e parametros especificos do release.

## Opcoes CMake principais

| Opcao | Padrao | Uso |
|---|---:|---|
| `MBGUI_NET_HLS` | `OFF` | Habilita suporte relacionado a HLS/rede. |
| `MBGUI_SAT_MONITOR` | `OFF` | Gera tambem o alvo opcional `sat_monitor`. |
| `MBGUI_MBTEST` | `OFF` | Habilita modo/testes MBTest. |
| `MBGUI_USE_RLOTTIE` | `ON` | Habilita suporte a animacoes Rive/Lottie. |
| `MBGUI_USE_PGO_GEN` | `OFF` | Build para geracao de perfil PGO. |
| `MBGUI_USE_PGO_USE` | `OFF` | Build usando perfil PGO. |
| `MBGUI_ANIMATION` | `ON` | Habilita animacoes na UI. |
| `MBGUI_USE_ASAN` | `OFF` | Habilita AddressSanitizer quando suportado pela toolchain. |
| `MBGUI_USE_UBSAN` | `OFF` | Habilita UndefinedBehaviorSanitizer quando suportado pela toolchain. |
| `MBGUI_USE_NAGRA_IRD_TESTS` | `OFF` | Inclui testes/rotinas especificas de IRD Nagra. |
| `MBGUI_USE_CHANNEL_RANKING` | `OFF` | Habilita ranking de canais. |
| `MBGUI_USE_EXTRA_DEBUGGING` | `OFF` | Habilita instrumentacao extra de debug. |
| `MBGUI_FORCED_UPDATE` | `OFF` | Habilita modo/fluxo de update forcado. |
| `MBGUI_USE_JPEG` | `OFF` | Habilita suporte JPEG quando dependencias estiverem disponiveis. |
| `SPLIT_DEBUG` | `ON` | Separa simbolos de debug do binario final. |
| `MBCAS_NAGRA_NAK` | `OFF` | Habilita comportamento NAK especifico do CAS Nagra. |
| `MBCAS_USE_DIR_EXT` | `OFF` | Habilita uso de diretorio externo no CAS quando aplicavel. |
| `MBGUI_PERIODIC_DUMP` | `OFF` | Habilita dumps periodicos para diagnostico. |

Tambem ha parametros de build usados para debug/log:

- `MBD_DEFAULT_LOG_LEVEL`: padrao atual `LOG_LEVEL_INFO`.
- `DEBUG_USER`: padrao atual `root`.

## Build de diagnostico

Para investigar travamentos, reinicios e corrupcao de memoria, os builds de diagnostico devem priorizar:

- `MBGUI_USE_ASAN=ON`, se a toolchain e runtime suportarem.
- `MBGUI_USE_UBSAN=ON`, se a toolchain e runtime suportarem.
- `MBGUI_USE_EXTRA_DEBUGGING=ON` para logs adicionais.
- `MBGUI_PERIODIC_DUMP=ON` para snapshots periodicos.
- `MBD_DEFAULT_LOG_LEVEL=LOG_LEVEL_DEBUG` quando o volume de log for aceitavel.
- `SPLIT_DEBUG=ON` para preservar simbolos sem aumentar o binario final de release.

Exemplo:

```bash
cmake -S . -B build-debug \
  -DCMAKE_TOOLCHAIN_FILE=toolchain_mips_montage.txt \
  -DCMAKE_BUILD_TYPE=Debug \
  -DMBGUI_USE_EXTRA_DEBUGGING=ON \
  -DMBGUI_PERIODIC_DUMP=ON \
  -DMBD_DEFAULT_LOG_LEVEL=LOG_LEVEL_DEBUG
cmake --build build-debug -j
```

## Observacoes sobre release

- O CMake consulta informacoes de Git para compor versao/identificacao; a maquina de build deve ter acesso ao repositorio Git.
- Com `SPLIT_DEBUG=ON`, os simbolos separados devem ser arquivados junto ao release para permitir analise posterior de core dump.
- Mudancas em instalacao facil, regionalizacao, bouquet, satelite, DiSEqC ou chaves devem gerar entrada em `doc/documentacao-mbgui/bugs` quando houver defeito conhecido ou risco de regressao.
- O build de release deve evitar flags de diagnostico que alterem timing, consumo de memoria ou comportamento de watchdog, exceto quando o release for explicitamente de investigacao.

## Cuidados ao reproduzir bugs

Para reproduzir bugs de receptor reiniciando ou travando:

- Registrar versao do binario, perfil de build e flags CMake usadas.
- Registrar operador, satelite, bouquet/regionalizacao e resultado de instalacao facil.
- Preservar banco antes/depois quando o defeito envolver reset de fabrica, varredura, canais ou favoritos.
- Coletar logs de `Task_Application`, `Task_Tuner`, `Task_Database`, `Task_Demux`, `Task_Player` e `Task_OSD`.
- Se possivel, testar a mesma sequencia com build contendo simbolos separados para facilitar stack trace.

