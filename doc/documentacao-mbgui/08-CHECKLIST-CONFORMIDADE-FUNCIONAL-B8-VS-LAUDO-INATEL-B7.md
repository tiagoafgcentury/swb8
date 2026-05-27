# 08 - Checklist de Conformidade Funcional B8 vs. Laudo INATEL do B7

## Objetivo

Este documento compara o software atual do modelo B8 com as funcoes verificadas no laudo INATEL do modelo B7 (`AV024-123`).

O foco aqui e exclusivamente funcional. Nao e uma afirmacao de certificacao, equivalencia de hardware ou reaproveitamento automatico de aprovacao regulatoria.

## Escopo

- Base de referencia funcional: laudo INATEL `AV024-123` do B7.
- Base de analise tecnica: codigo-fonte atual do B8 neste workspace.
- Criterio de status:
  - `Atende por evidencia de codigo`: existe implementacao clara no codigo.
  - `Atende com ressalva`: ha implementacao e sinais positivos, mas existe risco relevante ou evidencias parciais.
  - `Nao comprovado por codigo`: nao foi possivel provar apenas pela leitura do codigo.
  - `Nao aplicavel a analise estatica`: depende de bancada, medicao eletrica ou validacao fisica.

## Referencias

- Laudo extraido para texto: [inatel_report.txt](/abs/path/H:/Tiago/projetos_ai/mbgui-main/inatel_report.txt:816)
- Arquitetura atual: [01-ARCHITECTURE.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/01-ARCHITECTURE.md:11)
- Build atual: [02-SETUP_AND_BUILD.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/02-SETUP_AND_BUILD.md:7)
- Fluxos principais: [07-CORE_FUNCTIONS_AND_FLOWS.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/07-CORE_FUNCTIONS_AND_FLOWS.md:3)

## Resumo executivo

Conclusao atual: o B8 apresenta cobertura funcional forte para a maior parte dos itens de software do laudo do B7, especialmente OTA, EPG, regionalizacao, lineup, legendas, audio/video e DiSEqC.

Mesmo assim, a avaliacao nao fecha `100%` por leitura estatica porque:

- parte dos itens do laudo depende de bancada e medicao;
- alguns descritores aparecem no codigo com tratamento generico ou nao tratado;
- a documentacao interna do projeto aponta riscos atuais em regionalizacao e consistencia de estado.

## Checklist

| Grupo | Item do laudo B7 | Status B8 | Evidencia principal | Observacao |
|---|---|---|---|---|
| Especificacao de hardware | Botoes Power on/off | Atende por evidencia de codigo | [mb_remote_control_keys.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/mb_remote_control_keys.h:41) | Ha mapeamento explicito de `KEY_POWER`. |
| Especificacao de hardware | Canal +ch/-ch | Atende por evidencia de codigo | [mb_remote_control_keys.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/mb_remote_control_keys.h:22), [mb_remote_control_keys.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/mb_remote_control_keys.h:66) | Ha mapeamento explicito de `KEY_CHUP` e `KEY_CHDOWN`. |
| Especificacao de hardware | Porta HDMI / HDMI 1.4a tipo A | Atende com ressalva | [01-ARCHITECTURE.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/01-ARCHITECTURE.md:15), [mb_hdmi.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/mb_hdmi.h:159) | O codigo tem camada HDMI, mas conector fisico e versao exata dependem do hardware do B8. |
| Especificacao de hardware | HDMI / HDCP 1.4 | Nao comprovado por codigo | [mb_hdmi.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/mb_hdmi.h:159) | Existe suporte HDMI, mas a conformidade HDCP 1.4 nao ficou claramente demonstrada na leitura atual. |
| Especificacao de hardware | Porta RCA video composto | Nao comprovado por codigo | [01-ARCHITECTURE.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/01-ARCHITECTURE.md:15) | Pode existir por hardware/SDK, mas nao foi localizada evidencia clara no software para fechar o item. |
| Especificacao de hardware | Entrada RF / conector F | Nao aplicavel a analise estatica | [01-ARCHITECTURE.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/01-ARCHITECTURE.md:15) | A existencia do conector e caracteristica de hardware. |
| Especificacao de hardware | Porta USB 2.0 tipo A | Atende com ressalva | [mb_osd_usb_disk.cpp](/abs/path/H:/Tiago/projetos_ai/mbgui-main/ui/lvgl/mb_osd_usb_disk.cpp:44) | O software trata eventos de USB inserido/removido; a confirmacao de porta fisica e eletrica depende do produto. |
| Especificacao de hardware | Entrada AC 100-240V 50-60Hz | Nao aplicavel a analise estatica | [inatel_report.txt](/abs/path/H:/Tiago/projetos_ai/mbgui-main/inatel_report.txt:826) | Item fisico/eletrico do equipamento. |
| Especificacao de hardware | Saida DC 12V 1.0A | Nao aplicavel a analise estatica | [inatel_report.txt](/abs/path/H:/Tiago/projetos_ai/mbgui-main/inatel_report.txt:827) | Item fisico/eletrico do equipamento. |
| Especificacao de hardware | Manual do usuario em portugues | Nao comprovado por codigo | [TERMOS DE USO E CONDIÇÕES rev01 - 20240529.docx](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/TERMOS%20DE%20USO%20E%20CONDI%C3%87%C3%95ES%20rev01%20-%2020240529.docx:1) | Ha documentacao em portugues no repo, mas isso nao prova que o manual de produto entregue ao usuario final esteja presente e correto. |
| Especificacao de hardware | Cabo AC padrao ABNT | Nao aplicavel a analise estatica | [inatel_report.txt](/abs/path/H:/Tiago/projetos_ai/mbgui-main/inatel_report.txt:829) | Item fisico de embalagem/acessorio. |
| Especificacao de hardware | Cabo HDMI 1.5m | Nao aplicavel a analise estatica | [inatel_report.txt](/abs/path/H:/Tiago/projetos_ai/mbgui-main/inatel_report.txt:830) | Item fisico de embalagem/acessorio. |
| Especificacao de hardware | Controle remoto com pilhas | Atende com ressalva | [mb_remote_control_keys.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/mb_remote_control_keys.h:7) | O software suporta controle remoto, mas o fornecimento do acessorio e fisico. |
| Funcionalidade | Atualizacao OTA | Atende por evidencia de codigo | [mb_dvb_century.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/dvb/mb_dvb_century.h:32), [mb_dvb_ota.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/dvb/mb_dvb_ota.h:11), [01-ARCHITECTURE.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/01-ARCHITECTURE.md:14) | Existe parser e infraestrutura de OTA no stack DVB. |
| Funcionalidade | EPG | Atende por evidencia de codigo | [mb_dvb_eit.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/dvb/mb_dvb_eit.h:14), [01-ARCHITECTURE.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/01-ARCHITECTURE.md:19) | O projeto declara suporte a EPG e tabela EIT. |
| Funcionalidade | Identificacao de audio | Atende com ressalva | [mb_player.cpp](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/Montage/mb_player.cpp:679), [mb_dvb_globals.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/dvb/mb_dvb_globals.h:14) | Ha suporte a AAC e selecao de fluxo, mas a regra exata de priorizar audio em portugues com menor PID nao foi confirmada na leitura atual. |
| Funcionalidade | Closed Caption | Atende com ressalva | [mb_player.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/mb_player.h:88), [mb_player_cc.cpp](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/mb_player_cc.cpp:558) | Ha suporte forte a EIA-608, mas o descritor de caption aparece como nao tratado em uma camada de parsing. |
| Funcionalidade | Descricao de audio | Nao comprovado por codigo | [inatel_report.txt](/abs/path/H:/Tiago/projetos_ai/mbgui-main/inatel_report.txt:836) | O laudo exige e aprovou no B7, mas nao encontrei evidencia suficiente do fluxo especifico no B8 para fechar esse item. |
| Funcionalidade | Interface de nivel de sinal RF | Atende por evidencia de codigo | [01-ARCHITECTURE.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/01-ARCHITECTURE.md:19), [mb_osd_waiting_signal.cpp](/abs/path/H:/Tiago/projetos_ai/mbgui-main/ui/lvgl/mb_osd_waiting_signal.cpp:27) | A arquitetura e a UI indicam recursos de diagnostico/sinal para o usuario. |
| Regionalizacao | Varredura de canais | Atende por evidencia de codigo | [07-CORE_FUNCTIONS_AND_FLOWS.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/07-CORE_FUNCTIONS_AND_FLOWS.md:53), [01-ARCHITECTURE.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/01-ARCHITECTURE.md:16) | Existe fluxo de build de lineup por demux e busca. |
| Regionalizacao | Banner de ativacao | Nao comprovado por codigo | [inatel_report.txt](/abs/path/H:/Tiago/projetos_ai/mbgui-main/inatel_report.txt:840) | Nao localizei evidencia objetiva do banner de ativacao SATHDR na leitura atual. |
| Regionalizacao | Atualizacao automatica de regiao | Atende com ressalva | [07-CORE_FUNCTIONS_AND_FLOWS.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/07-CORE_FUNCTIONS_AND_FLOWS.md:103), [mb_ird_command.cpp](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/cas/nagra/mb_ird_command.cpp:163) | Existe fluxo de `zone_id` e persistencia, mas a propria documentacao interna aponta riscos de consistencia. |
| Regionalizacao | Zone_ID + lista de canais NIT/BAT | Atende com ressalva | [mb_dvb_bat.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/dvb/mb_dvb_bat.h:12), [mb_dvb_nit.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/dvb/mb_dvb_nit.h:14), [mb_dvb_idescriptor_interface.cpp](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/dvb/mb_dvb_idescriptor_interface.cpp:203), [07-CORE_FUNCTIONS_AND_FLOWS.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/07-CORE_FUNCTIONS_AND_FLOWS.md:114) | Implementacao existe, mas ha risco declarado de divergencia entre `zone_id`, `bouquet_id`, `network_id` e estado persistido. |
| Regionalizacao | Atualizacao automatica de canais | Atende com ressalva | [07-CORE_FUNCTIONS_AND_FLOWS.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/07-CORE_FUNCTIONS_AND_FLOWS.md:57), [01-ARCHITECTURE.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/01-ARCHITECTURE.md:16) | O fluxo de lineup suporta rebuild/update, mas o comportamento automatico completo deve ser confirmado em bancada. |
| Audio e video | Decodificacao H.264 | Atende por evidencia de codigo | [mb_dvb_globals.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/dvb/mb_dvb_globals.h:10), [mb_player.cpp](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/Montage/mb_player.cpp:711) | Ha suporte direto ao codec. |
| Audio e video | Decodificacao H.265 | Atende com ressalva | [inatel_report.txt](/abs/path/H:/Tiago/projetos_ai/mbgui-main/inatel_report.txt:847), [mb_dvb_globals.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/dvb/mb_dvb_globals.h:13) | O repo identifica HEVC no parsing; a amarracao completa de playback precisa de confirmacao adicional no player/SDK alvo. |
| Audio e video | Decodificacao AAC | Atende com ressalva | [mb_dvb_globals.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/dvb/mb_dvb_globals.h:14), [mb_player.cpp](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/Montage/mb_player.cpp:679), [mb_dvb_idescriptor_interface.cpp](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/dvb/mb_dvb_idescriptor_interface.cpp:827) | Ha suporte no player, mas o descritor AAC aparece como nao tratado em parte do parsing. |
| Teste de desempenho de RF | DVB-S2 | Nao aplicavel a analise estatica | [inatel_report.txt](/abs/path/H:/Tiago/projetos_ai/mbgui-main/inatel_report.txt:848) | Exige medicao e ensaio de recepcao. |
| Teste de desempenho de RF | Nivel do sinal de entrada IF banda L | Nao aplicavel a analise estatica | [inatel_report.txt](/abs/path/H:/Tiago/projetos_ai/mbgui-main/inatel_report.txt:849) | Exige bancada e instrumentos. |
| Teste de desempenho de RF | Maximo C/N para recepcao | Nao aplicavel a analise estatica | [inatel_report.txt](/abs/path/H:/Tiago/projetos_ai/mbgui-main/inatel_report.txt:850) | Exige bancada e instrumentos. |
| Teste de hardware | Sinal de controle para LNBF | Atende com ressalva | [mb_lnb_config.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/mb_lnb_config.h:31), [01-ARCHITECTURE.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/01-ARCHITECTURE.md:15) | O software possui configuracao de LNBF, mas a conformidade eletrica depende de hardware e medicao. |
| Teste de hardware | Sinal de controle DiSEqC | Atende com ressalva | [mb_diseqc.h](/abs/path/H:/Tiago/projetos_ai/mbgui-main/src/hal/mb_diseqc.h:8), [04-HAL_AND_TUNING.md](/abs/path/H:/Tiago/projetos_ai/mbgui-main/doc/documentacao-mbgui/04-HAL_AND_TUNING.md:25) | O software implementa DiSEqC, mas o comportamento eletrico e de interoperabilidade precisa de bancada. |

## Itens que hoje parecem mais fortes

- Atualizacao OTA
- EPG
- Varredura e build de lineup
- Parsing de NIT/BAT
- Fluxo de `zone_id`
- Suporte a DiSEqC/LNBF
- Closed Caption em EIA-608
- H.264 e AAC

## Itens que pedem validacao adicional

- Banner de ativacao da regionalizacao
- Descricao de audio
- Regra exata de identificacao/priorizacao de audio
- H.265 no caminho completo de reproducao do produto alvo
- Coerencia automatica de regionalizacao apos mudanca de operadora/regiao
- Itens eletricos e de RF

## Pendencias de bancada para fechar homologacao funcional

- Validar OTA real por transponder/fluxo homologado
- Validar EPG em canais com EIT presente e schedule
- Validar Closed Caption e descricao de audio em stream real
- Validar ativacao, banner e troca de regiao
- Validar inclusao/remocao automatica de canais
- Validar H.264 e H.265 com os perfis usados em operacao
- Validar tensao LNBF, comando DiSEqC e comportamento em chaveamento real
- Validar nivel de sinal, C/N e sensibilidade em bancada

## Parecer final

Parecer atual do B8 frente ao laudo funcional do B7:

- `Atende por evidencia de codigo`: varios itens centrais de software.
- `Atende com ressalva`: regionalizacao, AAC, Closed Caption, H.265, LNBF e DiSEqC.
- `Nao comprovado por codigo`: banner de ativacao, descricao de audio e alguns itens de hardware/acessorios.
- `Nao aplicavel a analise estatica`: desempenho de RF e requisitos eletricos.

Portanto, o B8 esta funcionalmente bem alinhado ao conjunto de funcoes do laudo do B7, mas ainda nao deve ser descrito como `100% comprovado` sem a rodada final de bancada.
