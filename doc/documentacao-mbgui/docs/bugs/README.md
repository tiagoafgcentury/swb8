# Bugs do MBGUI

Esta pasta concentra a documentação de bugs do MBGUI.

## Quando usar esta pasta

Use esta pasta para registrar:

- bugs encontrados em bancada ou campo
- análises técnicas de causa provavel
- PRDs de correção
- critérios de aceite
- riscos de regressão
- evidências por release

## Analises registradas

- [Varredura MBGUI 2026-05-28](varredura_mbgui_2026-05-28.md)
- [PRD - Bouquet segment SKY apos transicao Claro para SKY](PRD-BUG-sky-bouquet-segment-transicao-claro-sky.md)

## Quando não usar esta pasta

Não use esta pasta para:

- documentação base de arquitetura
- guias operacionais
- processos internos do time
- notas gerais de release

Esses materiais devem ficar nas pastas apropriadas da documentação.

## Estrutura recomendada de um bug

Cada documento de bug deve, quando fizer sentido, conter:

- resumo do problema
- ambiente de reprodução
- passos para reproduzir
- resultado esperado
- resultado atual
- impacto
- hipóteses de causa
- arquivos e funções suspeitas
- logs necessários
- proposta inicial de correção
- critérios de aceite
- plano de regressão

## Convenção de nomes

Padrões sugeridos:

- `PRD-BUG-<id>-<tema>.md`
- `analise_<tema>.md`
- `release-<versao>-bugs.md`

Exemplos:

- `PRD-BUG-8492-editar-chave-satelite.md`
- `analise_reboot_primeira_ativacao.md`
- `release-1.3.2-bugs.md`

## Fluxo de uso

1. Criar o arquivo `.md` nesta pasta.
2. Documentar o bug com o maximo de contexto util.
3. Registrar o novo arquivo no `nav` do `mkdocs.yml` caso ele deva aparecer no menu lateral.
4. Regenerar o portal HTML.

```powershell
py -3.12 -m mkdocs build --clean
```

## Observacao

O portal não descobre novas páginas automaticamente pelo menu. Para aparecer na navegação lateral, cada novo arquivo relevante precisa ser adicionado ao `nav` do `mkdocs.yml`.

Se a pasta passar a ter muitos bugs ativos, prefira criar uma subsecao consolidada por release ou tema antes de inflar o menu lateral com muitos arquivos individuais.
