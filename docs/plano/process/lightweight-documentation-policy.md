# Politica leve de documentacao

## Objetivo

Manter docs as code sem transformar documentacao em burocracia. A regra e simples: documentar quando a mudanca altera algo que outra pessoa precisara saber para desenvolver, operar, testar ou manter o projeto.

## Regra pratica

Atualize documentacao quando a mudanca afetar:

- Build, ambiente, toolchain, flags CMake ou scripts.
- Fluxo de release, assinatura, OTA, USB update ou debug em dispositivo.
- Interface compartilhada entre modulos.
- Comportamento visivel do usuario ou QA.
- Processo operacional recorrente.
- Decisao tecnica que muda direcao do projeto.

Nao exigir atualizacao documental para:

- Refactor interno sem mudanca de comportamento.
- Correcao pequena e localizada sem impacto em uso, build ou operacao.
- Ajuste de estilo, formatacao ou nomes internos triviais.
- Testes que apenas cobrem comportamento ja documentado.
- Mudancas temporarias ou experimentais ainda fora da branch principal.

## Niveis de documentacao

Use o menor nivel suficiente.

### Nivel 0: Nada a documentar

Quando a mudanca nao altera conhecimento reutilizavel.

Exemplo:

- Corrigir typo em log interno.
- Refatorar funcao privada sem mudar contrato.

No PR, basta marcar:

```md
- [x] Esta mudanca nao exige atualizacao de documentacao
```

### Nivel 1: Nota curta no PR

Quando a informacao e util para revisao, mas nao precisa virar documento permanente.

Exemplo:

- Workaround temporario.
- Observacao sobre teste manual.
- Pequena restricao tecnica que sera removida logo.

### Nivel 2: Atualizacao pequena em documento existente

Padrao mais comum.

Exemplo:

- Nova flag CMake.
- Mudanca em comando de build.
- Ajuste em um passo de release.
- Novo pre-requisito de ambiente.

### Nivel 3: Novo documento

Use quando o assunto e recorrente, critico ou grande demais para uma nota.

Exemplo:

- Novo runbook de update.
- Novo guia de build.
- Nova explicacao de arquitetura.
- Novo ADR.

## Checklist enxuto de PR

Use este checklist como padrao:

```md
## Documentacao

- [ ] Nao exige atualizacao de documentacao
- [ ] Atualizei documento existente
- [ ] Criei novo documento
- [ ] Atualizei comentarios Doxygen relevantes
- [ ] Conteudo apoiado por IA foi revisado por humano
```

Se a resposta for `Nao exige`, nao precisa justificar longamente. Uma frase basta quando houver duvida.

## Validacao leve

### Obrigatorio para mudancas em `docs/`

```bash
mkdocs build --strict
```

### Obrigatorio quando mudar comentarios Doxygen ou configuracao Doxygen

```bash
doxygen Doxyfile
```

### Recomendado, nao bloqueante no inicio

```bash
markdownlint docs/**/*.md
lychee docs/**/*.md
codespell docs
```

A validacao deve comecar simples. Primeiro garantir que o portal gera. Depois endurecer regras conforme a base amadurecer.

## Uso de IA sem burocracia

IA pode gerar rascunho, mas o responsavel precisa revisar. Nao e necessario criar registro pesado para toda interacao com IA.

Regra minima:

- Se IA ajudou em conteudo factual, o autor confirma os fatos antes do merge.
- Se houver incerteza, marcar como `Pendente` ou remover a afirmacao.
- Para release, seguranca, Nagra, OTA e hardware, exigir revisao do responsavel tecnico.

## Quando criar ADR

Crie ADR somente quando houver decisao com impacto duradouro.

Exemplos que merecem ADR:

- Escolha de MkDocs + Doxygen.
- Mudanca de arquitetura HAL.
- Mudanca de estrategia de release.
- Nova dependencia estrutural.

Nao criar ADR para:

- Ajustes pequenos de implementacao.
- Bugs locais.
- Decisoes reversiveis e pouco relevantes.

## Definicao simples de pronto

Uma mudanca de documentacao esta pronta quando:

- Esta no lugar certo.
- Foi revisada por alguem que entende o assunto.
- O MkDocs gera, se a mudanca afeta o portal.
- O Doxygen gera, se a mudanca afeta referencia de codigo.
- Nao contem suposicoes sem marcacao.

