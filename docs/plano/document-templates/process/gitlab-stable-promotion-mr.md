# Template GitLab: MR de promocao para stable

Use no MR de `main` para `stable`.

```md
## Release
Versao alvo: `<versao>`
Branch origem: `main`
Branch destino: `stable`
Tag planejada: `<tag>`

## Objetivo
Promover para `stable` o conjunto validado de mudancas do release.

## Issues Redmine incluidas
- #<id> - <titulo>
- #<id> - <titulo>

## Validacoes obrigatorias
- [ ] Lista de issues revisada
- [ ] MRs correspondentes integrados em `main`
- [ ] QA validou itens obrigatorios
- [ ] Nao ha bug bloqueante aberto para o release
- [ ] Documentacao de release/update revisada, se aplicavel

## Evidencias do release
- Build/release candidato:
- Resultado QA:
- Responsavel pela validacao:
- Data:

## Artefatos esperados
- [ ] Tag gerada na `stable`
- [ ] Pacote `.b8`, se aplicavel
- [ ] Pacote `.bin`, se aplicavel
- [ ] Assinatura Nagra, se aplicavel
- [ ] OTA `.ts`, se aplicavel

## Riscos
- 

## Acao apos merge
- [ ] Gerar tag na `stable`
- [ ] Gerar release a partir da `stable`
- [ ] Registrar evidencias no Redmine
- [ ] Atualizar/fechar issues entregues
```

