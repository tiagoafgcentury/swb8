# Setup, Build e Flags do Projeto

Este documento descreve o estado atual do build do MBGUI com base no código fonte disponível neste workspace.

## Visão geral

O projeto usa CMake como sistema principal de build.

- Projeto: `mbgui`
- Versão declarada no CMake: `1.3.5.2`
- Versão mínima do CMake: `3.22`
- Artefato principal: executável `mbgui`
- Plataforma alvo: receptor embarcado Linux, com toolchains MIPS para ALi/Montage
- Interface: LVGL, com fontes, temas e recursos gerados/embarcados
- Integrações principais: tuner/demux/player, banco SQLite, CAS/Nagra, EPG, middleware DVB e HALs

## Arquivos de build relevantes

- `CMakeLists.txt`: configuração principal do projeto, opções globais e alvo `mbgui`.
- `src/CMakeLists.txt`: fontes do core, HALs, CAS, banco, DVB, player e módulos de aplicação.
- `ui/lvgl/CMakeLists.txt`: integração da interface LVGL.
- `toolchain_mips_ali.txt`: toolchain para builds ALi.
- `toolchain_mips_montage.txt`: toolchain para builds Montage.
- `configure.sh`, `configure-dl.sh`, `configure-nagra.sh`, `configure-release.sh`: scripts auxiliares de configuração por perfil.
- `clean.sh`, `clean_tpm.sh`: scripts auxiliares de limpeza.
- `generate_release_seed.sh`: script usado no fluxo de release.
- `montage_sdk/*.sh`: scripts auxiliares para build/configuração em ambiente Montage/Docker.

## Fluxo básico de build

O fluxo exato depende da máquina de build, toolchain instalada e perfil de release. Como referência, o formato esperado ?:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain_mips_montage.txt -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Para builds ALi, usar a toolchain correspondente:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain_mips_ali.txt -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Os scripts `configure*.sh` devem ser preferidos em ambientes oficiais de build, pois podem aplicar flags, paths e parâmetros específicos do release.

## Opções CMake principais

| Opção | Padrão | Uso |
|---|---:|---|
| `MBGUI_NET_HLS` | `OFF` | Habilita suporte relacionado a HLS/rede. |
| `MBGUI_SAT_MONITOR` | `OFF` | Gera também o alvo opcional `sat_monitor`. |
| `MBGUI_MBTEST` | `OFF` | Habilita modo/testes MBTest. |
| `MBGUI_USE_RLOTTIE` | `ON` | Habilita suporte a animacoes Rive/Lottie. |
| `MBGUI_USE_PGO_GEN` | `OFF` | Build para geração de perfil PGO. |
| `MBGUI_USE_PGO_USE` | `OFF` | Build usando perfil PGO. |
| `MBGUI_ANIMATION` | `ON` | Habilita animacoes na UI. |
| `MBGUI_USE_ASAN` | `OFF` | Habilita AddressSanitizer quando suportado pela toolchain. |
| `MBGUI_USE_UBSAN` | `OFF` | Habilita UndefinedBehaviorSanitizer quando suportado pela toolchain. |
| `MBGUI_USE_NAGRA_IRD_TESTS` | `OFF` | Inclui testes/rotinas especificas de IRD Nagra. |
| `MBGUI_USE_CHANNEL_RANKING` | `OFF` | Habilita ranking de canais. |
| `MBGUI_USE_EXTRA_DEBUGGING` | `OFF` | Habilita instrumentacao extra de debug. |
| `MBGUI_FORCED_UPDATE` | `OFF` | Habilita modo/fluxo de update forcado. |
| `MBGUI_USE_JPEG` | `OFF` | Habilita suporte JPEG quando dependências estiverem disponiveis. |
| `SPLIT_DEBUG` | `ON` | Separa símbolos de debug do binário final. |
| `MBCAS_NAGRA_NAK` | `OFF` | Habilita comportamento NAK específico do CAS Nagra. |
| `MBCAS_USE_DIR_EXT` | `OFF` | Habilita uso de diretório externo no CAS quando aplicável. |
| `MBGUI_PERIODIC_DUMP` | `OFF` | Habilita dumps periodicos para diagnóstico. |

Também há parâmetros de build usados para debug/log:

- `MBD_DEFAULT_LOG_LEVEL`: padrão atual `LOG_LEVEL_INFO`.
- `DEBUG_USER`: padrão atual `root`.

## Build de diagnóstico

Para investigar travamentos, reinicios e corrupção de memória, os builds de diagnóstico devem priorizar:

- `MBGUI_USE_ASAN=ON`, se a toolchain e runtime suportarem.
- `MBGUI_USE_UBSAN=ON`, se a toolchain e runtime suportarem.
- `MBGUI_USE_EXTRA_DEBUGGING=ON` para logs adicionais.
- `MBGUI_PERIODIC_DUMP=ON` para snapshots periodicos.
- `MBD_DEFAULT_LOG_LEVEL=LOG_LEVEL_DEBUG` quando o volume de log for aceitavel.
- `SPLIT_DEBUG=ON` para preservar símbolos sem aumentar o binário final de release.

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

- O CMake consulta informações de Git para compor versão/identificação; a máquina de build deve ter acesso ao repositório Git.
- A geração de `git_version.cpp` usa pipeline com `git show`, `head`, `cut` e `sed`; o ambiente oficial precisa disponibilizar esses utilitários.
- Com `SPLIT_DEBUG=ON`, os símbolos separados devem ser arquivados junto ao release para permitir análise posterior de core dump.
- Mudanças em instalação fácil, regionalização, bouquet, satélite, DiSEqC ou chaves devem gerar entrada em `doc/documentacao-mbgui/bugs` quando houver defeito conhecido ou risco de regressão.
- O build de release deve evitar flags de diagnóstico que alterem timing, consumo de memória ou comportamento de watchdog, exceto quando o release for explicitamente de investigação.

## Cuidados ao reproduzir bugs

Para reproduzir bugs de receptor reiniciando ou travando:

- Registrar versão do binário, perfil de build e flags CMake usadas.
- Registrar operador, satélite, bouquet/regionalização e resultado de instalação fácil.
- Preservar banco antes/depois quando o defeito envolver reset de fabrica, varredura, canais ou favoritos.
- Coletar logs de `Task_Application`, `Task_Tuner`, `Task_Database`, `Task_Demux`, `Task_Player` e `Task_OSD`.
- Se possível, testar a mesma sequência com build contendo símbolos separados para facilitar stack trace.
