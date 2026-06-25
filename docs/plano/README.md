# Documentacao do projeto

Este diretorio concentra a documentacao oficial do projeto, sem alterar a estrutura do codigo-fonte.

O diretorio `doc/` existente deve ser considerado area de testes e nao faz parte desta base documental.

## Mapa inicial

- `documentation-strategy.md`: plano de estruturacao, governanca e melhoria continua.
- `management/documentation-management-plan.md`: plano de gestao do projeto de documentacao.
- `management/redmine-wbs-schedule.md`: orientacao para WBS, cronograma e controle no Redmine.
- `management/redmine-task-backlog.md`: lista inicial de tarefas para cadastrar no Redmine.
- `management/executive-dashboard.html`: painel executivo HTML para acompanhamento pela diretoria.
- `management/executive-presentation-5-slides.html`: apresentacao executiva HTML com 5 slides.
- `scope/process-functionality-inventory.md`: inventario inicial de processos e funcionalidades a documentar.
- `tooling/doxygen-mkdocs.md`: estrategia de uso combinado de Doxygen e MkDocs.
- `tooling/doxygen-commenting-guidelines.md`: diretrizes para comentarios Doxygen assistidos por IA.
- `tooling/markdown-mkdocs-style-guide.md`: guia de tags, recursos Markdown e boas praticas para gerar um portal MkDocs profissional.
- `process/docs-as-code.md`: processo de documentacao como codigo.
- `process/lightweight-documentation-policy.md`: politica leve para evitar burocracia.
- `process/redmine-gitlab-workflow.md`: boas praticas para fluxo Redmine e GitLab.
- `process/examples/redmine-8594-flow-example.md`: exemplo pratico do fluxo aplicado a issue Redmine #8594.
- `process/ai-assisted-documentation.md`: processo para uso de IA com verificacao humana.
- `process/software-documentation-architect-best-practices.md`: boas praticas e skills para o arquiteto ou analista responsavel pela documentacao de software.
- `document-templates/adr.md`: modelo para registrar decisoes arquiteturais.
- `document-templates/runbook.md`: modelo para procedimentos operacionais.
- `document-templates/technical-guide.md`: modelo para guias tecnicos de desenvolvimento.
- `document-templates/api-reference.md`: modelo para referencias de interfaces, endpoints e contratos.
- `document-templates/process/`: templates operacionais para Redmine, GitLab, release e impacto em documentacao.
- `docs/ppt-plano/`: projeto MkDocs para gerar um portal HTML com os documentos Markdown do plano.

## Taxonomia recomendada

A documentacao deve seguir o modelo Diataxis:

- Tutoriais: onboarding e primeiros passos.
- Guias: tarefas praticas e fluxos recorrentes.
- Referencia: comandos, variaveis, APIs, parametros e contratos.
- Explicacoes: arquitetura, decisoes, contexto e tradeoffs.

## Estrutura alvo

```text
docs/
  plano/
    README.md
    documentation-strategy.md
    getting-started/
    architecture/
    development/
    operations/
    release/
    api/
    decisions/
    tooling/
    process/
    management/
    scope/
    reference/
    document-templates/
```

Crie os subdiretorios conforme a documentacao for sendo produzida. Evite criar pastas vazias.





















