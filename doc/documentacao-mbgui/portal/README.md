# Portal HTML da documentacao MBGUI

Este diretorio contem a estrutura para gerar um portal HTML simples a partir dos arquivos Markdown em `doc/documentacao-mbgui`.

## Objetivo

Manter o fluxo de documentacao incremental:

- Os arquivos `.md` continuam sendo a fonte oficial.
- O portal em HTML e regenerado localmente quando houver mudancas.
- A saida pode ser aberta diretamente no navegador pelo arquivo `site/index.html`.

## Como gerar

No diretorio raiz do repositorio:

```powershell
node .\doc\documentacao-mbgui\portal\build-docs.js
```

## Estrutura

- `build-docs.js`: gerador do portal.
- `site/`: HTMLs gerados.

## Fluxo recomendado

1. Criar ou atualizar um `.md` em `doc/documentacao-mbgui`.
2. Se o documento for novo, incluir a entrada na lista `sections` dentro de `build-docs.js`.
3. Regenerar o portal com o comando acima.
4. Abrir `doc\documentacao-mbgui\portal\site\index.html` no navegador.

## Quando migrar para uma ferramenta maior

Se a documentacao crescer muito e passar a exigir busca completa, versionamento de docs, diagramas mais avancados ou publicacao automatica, ai vale migrar para uma stack como MkDocs Material ou VitePress. No estado atual, o gerador local atende melhor por ser simples, offline e sem dependencia externa.
