# Redmine, WBS e cronograma

## Objetivo

Usar o Redmine para controlar o projeto de documentacao com tarefas, entregas, responsaveis, prazos e acompanhamento de progresso.

A documentacao continua seguindo docs as code no Git. O Redmine controla gestao do trabalho; o Git controla conteudo, revisao e historico.

## Separacao de responsabilidades

| Area | Ferramenta | Papel |
| --- | --- | --- |
| Conteudo oficial | Git + Markdown | Fonte da verdade da documentacao |
| Portal | MkDocs | Publicacao navegavel |
| Referencia de codigo | Doxygen | Documentacao tecnica gerada do codigo |
| Gestao do projeto | Redmine | Backlog, WBS, cronograma, responsaveis e status |
| Revisao tecnica | Pull Request | Validacao humana e historico da mudanca |

## Estrutura recomendada no Redmine

### Projeto

Criar ou usar um projeto Redmine chamado, por exemplo:

```text
Documentacao Software B8
```

### Trackers sugeridos

- Epic: grande frente de documentacao.
- Task: entrega documental objetiva.
- Review: validacao humana de documento.
- Bug: erro ou desatualizacao na documentacao.
- Improvement: melhoria incremental.

Se o Redmine atual tiver trackers padronizados, adaptar os nomes sem criar complexidade desnecessaria.

### Campos customizados recomendados

- Tipo Diataxis: Tutorial, Guia, Referencia, Explicacao.
- Fonte: Codigo, PDF, entrevista, teste, IA, documento legado.
- Status de validacao: A validar, Inferido, Confirmado, Publicado.
- Documento alvo: caminho em `docs/`.
- Modulo relacionado: HAL, DVB, UI, CAS, TPM, Release, Build, etc.
- Revisor tecnico.
- Link do PR.
- Link do portal publicado.

## WBS inicial sugerida

```text
1. Fundacao docs as code
   1.1 Configurar MkDocs
   1.2 Configurar Doxygen
   1.3 Definir checklist de PR
   1.4 Definir publicacao local/interna

2. Inventario e planejamento
   2.1 Validar inventario de processos e funcionalidades
   2.2 Priorizar itens por criticidade
   2.3 Criar backlog no Redmine
   2.4 Fechar cronograma inicial

3. Onboarding e build
   3.1 Ambiente de desenvolvimento
   3.2 Build debug
   3.3 Build release
   3.4 SDK Montage
   3.5 Toolchains ALi/Montage

4. Arquitetura tecnica
   4.1 Visao geral
   4.2 Mapa de modulos
   4.3 HAL e plataformas
   4.4 DVB e listas de canais
   4.5 UI LVGL
   4.6 CAS/Nagra
   4.7 TPM

5. Funcionalidades do receptor
   5.1 Stand-by
   5.2 Lista de canais e favoritos
   5.3 Busca de canais
   5.4 Instalacao facil
   5.5 Configuracoes do receptor
   5.6 Multimidia e gravacao
   5.7 Closed Caption e audio
   5.8 Telas de informacao, diagnostico e suporte

6. Operadoras e regras satelitais
   6.1 Claro SATHDR
   6.2 Canal de servico Claro
   6.3 Sky Nova Parabolica
   6.4 Regras de transicao Claro/Sky

7. Operacao e release
   7.1 Atualizacao USB
   7.2 Atualizacao via satelite
   7.3 Atualizacao compulsoria
   7.4 Geracao `.b8`
   7.5 Geracao `.bin`
   7.6 Assinatura Nagra
   7.7 Scripts de release

8. Referencia Doxygen assistida por IA
   8.1 Criar Doxyfile inicial
   8.2 Gerar baseline Doxygen sem comentarios novos
   8.3 IA sugere comentarios para headers `.h` de modulo piloto
   8.4 Revisao humana e aplicacao dos comentarios em headers `.h` de HAL/DVB/tasks/TPM
   8.5 IA sugere comentarios para `.cpp` relevantes, com revisao humana antes do MR
   8.6 Corrigir warnings e ruido Doxygen
   8.7 Linkar referencia Doxygen no MkDocs

9. Qualidade e melhoria continua
   9.1 Validacao MkDocs
   9.2 Validacao Doxygen
   9.3 Revisao mensal de lacunas
   9.4 Revisao trimestral de documentos criticos
```

## Modelo de tarefa Redmine

Titulo:

```text
[Docs] Documentar build release
```

Descricao:

```md
## Objetivo
Documentar o processo de build release do B8.

## Tipo Diataxis
Guia

## Documento alvo
`docs/plano/getting-started/build-release.md`

## Fontes
- `configure-release.sh`
- `toolchain_mips_ali.txt`
- Validacao com responsavel de build/release

## Criterios de aceite
- Comandos documentados.
- Pre-requisitos listados.
- Saidas esperadas descritas.
- Pendencias marcadas, se houver.
- Revisao humana concluida.
- PR linkado.
- MkDocs build validado.
```

## Estados sugeridos

Fluxo simples:

```text
Novo -> Em levantamento -> Em redacao -> Em revisao -> Publicado -> Fechado
```

Para evitar burocracia, nao crie estados demais. Use comentarios e campos para detalhes.

## Marcos sugeridos

### Marco 1: Fundacao

- MkDocs minimo.
- Doxygen minimo.
- Processo docs as code leve.
- Inventario inicial migrado.

### Marco 2: Onboarding e build

- Ambiente.
- Build debug.
- Build release.
- Mapa do repositorio.

### Marco 3: Operacao critica

- Release.
- Atualizacao.
- Assinatura Nagra.
- TPM/producao.

### Marco 4: Funcionalidades principais

- Canais.
- Busca.
- Stand-by.
- Instalacao facil.
- Claro/Sky.

### Marco 5: Referencia tecnica

- Doxygen em modulos criticos.
- Links entre MkDocs e Doxygen.
- Revisao de qualidade.

## Como ligar Git e Redmine

Recomendacao simples:

- Toda tarefa Redmine relevante deve ter branch ou PR associado.
- Todo PR de documentacao deve citar o ID Redmine.
- O Redmine acompanha progresso; o PR guarda revisao tecnica.

Exemplo de branch:

```text
docs/redmine-123-build-release
```

Exemplo de commit:

```text
docs: document release build refs #123
```

## Proximos passos

1. Validar a WBS inicial.
2. Definir trackers e campos customizados disponiveis no Redmine atual.
3. Criar epicos das frentes principais.
4. Criar tarefas para os itens de prioridade alta do inventario.
5. Estimar esforco com base em complexidade e necessidade de validacao humana.
6. Montar cronograma por marcos.



