# GitBook Starter para o MBGUI

Este diretorio contem uma estrutura inicial pensada para publicar o projeto em GitBook desde o zero.

## Objetivo

Dar ao time um ponto de partida simples para:

- organizar conhecimento tecnico do projeto;
- acelerar onboarding;
- registrar decisoes e fluxos operacionais;
- criar uma base viva para QA, suporte e desenvolvimento.

## Como usar no GitBook

1. Criar um novo `space` no GitBook para o projeto.
2. Importar estes arquivos Markdown ou sincronizar este diretorio com GitHub/GitLab usando `Git Sync`.
3. Reproduzir a ordem das paginas conforme o arquivo [`SUMMARY.md`](./SUMMARY.md).
4. Ajustar os textos placeholder com a realidade do projeto.
5. Publicar primeiro como documentacao interna.

## Estrutura sugerida

- `00-HOME.md`: pagina inicial do projeto.
- `01-PROJECT-OVERVIEW.md`: visao geral e contexto.
- `02-ARCHITECTURE.md`: arquitetura e modulos.
- `03-SETUP-AND-BUILD.md`: ambiente e build.
- `04-RUN-AND-DEBUG.md`: execucao, logs e debug.
- `05-CODE-ORGANIZATION.md`: como o codigo esta dividido.
- `06-WORKFLOWS.md`: fluxo de desenvolvimento e entrega.
- `07-TESTING-AND-QA.md`: estrategia de testes e validacao.
- `08-TROUBLESHOOTING.md`: falhas comuns e abordagem de suporte.
- `09-DECISIONS-AND-STANDARDS.md`: regras, padroes e ADRs leves.
- `10-GLOSSARY-AND-FAQ.md`: glossario e perguntas frequentes.

## Observacoes

- Os textos foram escritos para funcionar mesmo quando o projeto ainda nao tem documentacao madura.
- Onde o conteudo ainda nao for conhecido, deixe explicitamente `A definir` em vez de adivinhar.
- Quando o time amadurecer a base, vale separar em multiplos `spaces` no GitBook, como `Produto`, `Engenharia`, `QA` e `Suporte`.
