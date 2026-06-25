# Docs as Code

## Objetivo

Adotar documentacao como codigo para que a documentacao seja versionada, revisada, testada e publicada usando o mesmo fluxo disciplinado do desenvolvimento de software.

Neste projeto, docs as code significa:

- Documentacao mantida no repositorio Git.
- Alteracoes feitas por branch e pull request.
- Revisao tecnica junto com revisao de codigo.
- Validacoes automatizadas para Markdown, links, MkDocs e Doxygen.
- Publicacao automatica ou reproduzivel do portal.
- Responsabilidade clara por paginas, modulos e processos criticos.

## Principios

- A documentacao oficial vive no repositorio.
- Toda mudanca relevante no software deve considerar impacto documental.
- Documentacao deve ter dono, criterio de aceite e historico.
- O portal deve ser gerado automaticamente a partir dos arquivos versionados.
- Conteudo gerado por IA entra como rascunho ate passar por revisao humana.
- HTML gerado por MkDocs ou Doxygen nao deve ser editado manualmente.

## Fluxo de trabalho

```text
Issue ou lacuna
  -> branch de documentacao
  -> edicao em Markdown, Doxygen ou configuracao
  -> validacao local
  -> pull request
  -> revisao humana
  -> build automatizado
  -> merge
  -> publicacao
```

## Tipos de mudanca

### Mudanca de conteudo

Exemplos:

- Novo guia de build.
- Atualizacao de runbook.
- Explicacao de modulo.
- ADR.
- Ajuste de referencia operacional.

Validacoes esperadas:

- Conteudo revisado por responsavel tecnico.
- Links funcionando.
- Pagina aparece no menu MkDocs.
- Comandos testados ou marcados como pendentes.

### Mudanca de referencia de codigo

Exemplos:

- Comentarios Doxygen em headers.
- Documentacao de structs, enums e funcoes compartilhadas.
- Agrupamento Doxygen por modulo.

Validacoes esperadas:

- Doxygen gera sem erros criticos.
- Comentario explica contrato, nao obviedade.
- Dono do modulo revisou o conteudo.

### Mudanca de processo

Exemplos:

- Novo processo de release.
- Checklist de PR.
- Politica de IA.
- Politica de revisao trimestral.

Validacoes esperadas:

- Processo tem responsavel.
- Processo tem gatilho claro.
- Processo tem criterio de aceite.
- Processo tem forma de medicao ou auditoria.

## Branches e commits

Sugestao de convencao:

```text
docs/<assunto>
```

Exemplos:

```text
docs/build-release
docs/doxygen-dvb
docs/runbook-usb-update
docs/adr-hal-platforms
```

Sugestao de mensagens de commit:

```text
docs: add release build guide
docs: describe DVB module responsibilities
docs: document Nagra signing runbook
docs: add Doxygen comments for HAL interfaces
```

## Pull requests

Todo PR de documentacao deve responder:

- Qual lacuna este PR resolve?
- O conteudo foi validado por quem?
- Ha comandos ou procedimentos testados?
- Ha informacao inferida ou pendente?
- A pagina foi adicionada ao MkDocs?
- Doxygen/MkDocs geram sem erro?

Checklist sugerido:

```md
## Docs as Code

- [ ] Alteracao feita em branch propria
- [ ] Pagina adicionada ao menu do MkDocs quando aplicavel
- [ ] Links revisados
- [ ] Comandos testados ou marcados como pendentes
- [ ] Conteudo revisado por responsavel tecnico
- [ ] Conteudo gerado por IA foi validado por humano
- [ ] `mkdocs build --strict` executado ou validado no CI
- [ ] `doxygen Doxyfile` executado ou validado no CI quando aplicavel
```

## Validacao automatica recomendada

Fase inicial:

```bash
mkdocs build --strict
doxygen Doxyfile
```

Fase seguinte:

```bash
markdownlint docs/**/*.md
lychee docs/**/*.md
codespell docs
```

A regra pratica e comecar com validacoes que gerem pouco atrito e endurecer o CI conforme a base amadurecer.

## Publicacao

Modelo recomendado:

1. Merge na branch principal.
2. Pipeline executa MkDocs.
3. Pipeline executa Doxygen.
4. Saida Doxygen e anexada ou linkada no portal MkDocs.
5. Portal e publicado em ambiente interno.

Nao editar arquivos gerados manualmente.

## Papel da IA

IA pode criar rascunhos e levantar lacunas, mas a mudanca continua seguindo o fluxo de codigo:

- Rascunho gerado.
- Humano valida fatos.
- PR registra a revisao.
- Build automatizado valida a estrutura.
- Conteudo aprovado vira oficial.

## Definicao de pronto

Uma entrega de documentacao esta pronta quando:

- Resolve uma lacuna identificada.
- Esta versionada no Git.
- Foi revisada por responsavel humano.
- Passa nas validacoes acordadas.
- Esta navegavel pelo MkDocs, se for documento de usuario.
- Esta refletida no Doxygen, se for referencia de codigo.
- Nao contem pendencias ocultas.

