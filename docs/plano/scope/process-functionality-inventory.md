# Inventario de processos e funcionalidades

## Origem

Este inventario foi iniciado a partir do arquivo `doc/Processos Software B8.pdf`, indicado como fonte de processos e funcionalidades ja mapeados.

Observacao: o diretorio `doc/` continua fora da base oficial de documentacao. Este PDF e uma excecao usada como insumo de levantamento. O conteudo oficial deve ser migrado, revisado e mantido em `docs/`.

## Objetivo

Mapear os processos e funcionalidades que precisam ser documentados no projeto B8, servindo como base para:

- Backlog de documentacao.
- Priorizacao por criticidade.
- WBS do projeto de documentacao.
- Cronograma no Redmine.
- Definicao de responsaveis e revisores.
- Identificacao de lacunas para levantamento com IA e verificacao humana.

## Como usar este inventario

Cada item deve evoluir para um ou mais artefatos Diataxis:

- Tutorial: quando o objetivo for ensinar alguem novo a executar um fluxo completo.
- Guia: quando o objetivo for realizar uma tarefa especifica.
- Referencia: quando o objetivo for consultar regras, parametros, tabelas, formatos ou comandos.
- Explicacao: quando o objetivo for entender comportamento, arquitetura, regras de negocio ou tradeoffs.

Nem todo item precisa virar documento novo. Sempre aplicar a politica leve de documentacao.

## Categorias iniciais

### Ciclo de energia e estado

| Item | Subitens | Tipo provavel | Prioridade sugerida | Status |
| --- | --- | --- | --- | --- |
| Stand-by | Claro, Sky, entrada em stand-by, comportamento em stand-by, retorno de stand-by | Explicacao, guia de teste | Alta | A validar |
| Funcionamento em stand-by para agendamentos | Gravacao, assistir | Explicacao, guia de QA | Alta | A validar |

### Canais, listas e favoritos

| Item | Subitens | Tipo provavel | Prioridade sugerida | Status |
| --- | --- | --- | --- | --- |
| Lista de canais TV/Radio | Opcoes de edicao, regras de favoritos | Guia, referencia | Alta | A validar |
| Regras para mostrar canais Claro | SATHDR, BAT, TVRO, bloqueio de TV paga | Explicacao, referencia | Alta | A validar |
| Atualizacao de lista Claro | BAT, NIT, SDT, canal de servico | Explicacao, referencia | Alta | A validar |
| Regras Sky para canais livres | Nova Parabolica | Explicacao, referencia | Alta | A validar |
| Atualizacao de lista Sky | Regras de atualizacao | Explicacao, referencia | Alta | A validar |
| Busca de canais | Automatica, manual | Guia, explicacao | Alta | A validar |

### Multimidia e gravacao

| Item | Subitens | Tipo provavel | Prioridade sugerida | Status |
| --- | --- | --- | --- | --- |
| Fotos | Configuracoes, codecs suportados | Referencia, guia | Media | A validar |
| Video | Configuracoes, codecs suportados | Referencia, guia | Media | A validar |
| Musicas | Configuracoes, codecs suportados | Referencia, guia | Media | A validar |
| Gravador | Teste de pendrive, opcoes de gravacao | Guia, runbook | Alta | A validar |
| Agendamentos | Gravacao, assistir, comportamento em stand-by | Guia, explicacao | Alta | A validar |

### Configuracao, instalacao e recepcao

| Item | Subitens | Tipo provavel | Prioridade sugerida | Status |
| --- | --- | --- | --- | --- |
| Padrao de fabrica | Procedimento Claro, procedimento Sky | Guia, explicacao | Alta | A validar |
| LNBF | Tipos e configuracoes | Referencia, guia | Alta | A validar |
| Chave DiSEqC | Configuracoes, busca automatica | Guia, referencia | Alta | A validar |
| Instala Facil | Sequencia de funcionamento | Tutorial, explicacao | Alta | A validar |
| Configuracoes do receptor | Idiomas, resolucao, padrao de cor, aspecto | Referencia, guia | Media | A validar |
| Relogio | Fonte de data/hora, tabela usada, fuso horario | Explicacao, referencia | Alta | A validar |
| Regras de transicao | Sky para Claro, Claro para Sky | Explicacao, guia de teste | Alta | A validar |

### Acessibilidade, audio e legendas

| Item | Subitens | Tipo provavel | Prioridade sugerida | Status |
| --- | --- | --- | --- | --- |
| Closed Caption | CEA 608, CEA 708 compatible, ARIB B-37, DVB Subtitle | Referencia, explicacao | Alta | A validar |
| Regras de canais de audio | Portugues, Ingles, audiodescricao | Referencia, explicacao | Alta | A validar |
| Audiodescricao | Stream individualizado, PES private data, identificacao AU1/AU2/AUD | Referencia, explicacao | Alta | Pendente de revisao normativa |

### Operadoras e servicos satelitais

| Item | Subitens | Tipo provavel | Prioridade sugerida | Status |
| --- | --- | --- | --- | --- |
| Claro SATHDR | BAT, canais, atualizacao de lista, canal de servico 12120 MHz | Explicacao, referencia | Alta | A validar |
| Canal de servico Claro | OTA, DVB-SI, NIT, SDT, BAT, TVRO | Explicacao, referencia | Alta | A validar |
| Sky Nova Parabolica | Canais livres, recarga, atualizacao de lista | Explicacao, referencia | Alta | A validar |

### Atualizacao, release e assinatura

| Item | Subitens | Tipo provavel | Prioridade sugerida | Status |
| --- | --- | --- | --- | --- |
| Atualizacao de software | Compulsoria, USB, satelite | Runbook, guia | Alta | A validar |
| Geracao de releases | `.b8`, `.bin`, assinatura Nagra, scripts | Runbook, referencia | Alta | A validar |
| Assinatura Nagra | Fluxo e artefatos | Runbook | Alta | Requer revisao responsavel |

### Interface, navegacao e informacoes

| Item | Subitens | Tipo provavel | Prioridade sugerida | Status |
| --- | --- | --- | --- | --- |
| Tela de informacoes do canal | Info 1x, Info 2x | Guia de UX/QA, referencia | Media | A validar |
| Teclas | LAST, Mais, Sleep, TV/Radio | Referencia, guia de QA | Media | A validar |
| Tela de diagnostico | Campos e interpretacao | Referencia, guia | Alta | A validar |
| Mensagens gerais | Tipos e formatos, Sky, Nagra, Century | Referencia | Alta | A validar |
| Informacoes do receptor | Campos exibidos | Referencia | Media | A validar |
| Informacoes de acesso condicional | Campos e comportamento | Referencia | Alta | A validar |
| Suporte | Fluxos e informacoes de suporte | Guia, referencia | Media | A validar |

### Estruturas internas e producao

| Item | Subitens | Tipo provavel | Prioridade sugerida | Status |
| --- | --- | --- | --- | --- |
| Estruturas gerais | BD canais, BD configuracao STB, arquivos ZoneID, PersonalBit, Nagra | Referencia, explicacao | Alta | A validar |
| TPM | Funcionamento para producao, protocolo HTTP/TPM | Runbook, referencia API | Alta | A validar |

### Onboarding

| Item | Subitens | Tipo provavel | Prioridade sugerida | Status |
| --- | --- | --- | --- | --- |
| Ambiente de desenvolvimento e build | Preparacao, toolchains, comandos | Tutorial, guia | Alta | A validar |

## Campos recomendados para evoluir cada item

Ao transformar um item em tarefa no Redmine ou documento oficial, preencher:

- ID do item.
- Nome do processo ou funcionalidade.
- Categoria.
- Tipo Diataxis esperado.
- Documento alvo em `docs/`.
- Modulos de codigo relacionados.
- Fonte atual de informacao.
- Responsavel tecnico.
- Revisor humano.
- Prioridade.
- Criticidade operacional.
- Status de validacao: `A validar`, `Inferido`, `Confirmado`, `Publicado`.
- Criterio de aceite.
- Dependencias.
- Estimativa.
- Link da issue no Redmine.

## Proximos passos

1. Validar este inventario com responsaveis de produto, desenvolvimento, QA e release.
2. Marcar itens criticos para documentacao inicial.
3. Converter itens prioritarios em epicos/tarefas no Redmine.
4. Criar WBS com base nas categorias deste documento.
5. Definir marcos do cronograma: fundacao, onboarding/build, arquitetura, operacao/release, funcionalidades.
6. Usar IA para rascunhos por categoria, sempre com revisao humana.

