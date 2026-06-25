# Documentacao assistida por IA

## Objetivo

Usar IA para acelerar levantamento, rascunho e manutencao de documentacao, mantendo verificacao humana como criterio obrigatorio de publicacao.

IA deve apoiar o processo, nao substituir responsabilidade tecnica.
## Engenharia de contexto

A ideia mais importante para o plano e tratar documentacao como contexto reutilizavel para pessoas e ferramentas de IA.

Na pratica, isso significa que o responsavel pela documentacao deve organizar informacoes de forma que um desenvolvedor, revisor, QA ou assistente de IA consiga entender rapidamente:

- Qual e o objetivo do modulo ou processo.
- Quais arquivos, scripts e comandos sao relevantes.
- Quais decisoes ja foram tomadas.
- Quais contratos nao podem ser quebrados.
- Quais informacoes estao confirmadas, inferidas ou pendentes.
- Quais exemplos reais devem ser usados como referencia.

Documentar bem passa a ser tambem preparar bom contexto. Isso reduz respostas genericas da IA, melhora revisoes tecnicas e diminui dependencia de explicacao oral.


## Fluxo agentico recomendado

Para atividades de documentacao e manutencao, evite pedir para a IA implementar ou escrever o documento final logo no primeiro passo. O fluxo recomendado e:

```text
Investigar contexto -> Planejar -> Implementar ou documentar -> Validar -> Registrar aprendizado
```

Aplicacao pratica:

1. Pedir primeiro a investigacao: arquivos envolvidos, fluxos, testes, scripts, dependencias e riscos.
2. Pedir um plano curto: o que sera alterado, quais documentos serao afetados e como validar.
3. Executar a mudanca ou redigir o documento.
4. Validar contra codigo, comandos, testes e responsaveis tecnicos.
5. Registrar o que se repete como contexto reutilizavel.

Exemplo de pedido inicial:

```text
Antes de documentar este processo, investigue onde ele aparece no repositorio, quais arquivos e scripts estao envolvidos, quais documentos ja existem, quais pontos precisam de validacao humana e quais riscos devem ser destacados.
```

## Contexto reutilizavel

Quando uma informacao aparece repetidamente no trabalho do time, ela deve deixar de depender apenas da memoria das pessoas.

Candidatos a contexto reutilizavel:

- Pergunta recorrente: transformar em prompt reutilizavel.
- Regra recorrente em revisao: transformar em instruction ou diretriz de projeto.
- Workflow com muitos passos: transformar em skill, runbook ou template.
- Acao sensivel que exige validacao: avaliar hook, checklist ou gate no fluxo.
- Decisao tecnica recorrente: registrar em ADR ou documento de arquitetura.

O objetivo nao e configurar tudo de uma vez. O objetivo e observar repeticoes e transformar os padroes mais uteis em contexto compartilhado para humanos e IA.

## Usos permitidos

IA pode ajudar em:

- Levantamento inicial de estrutura do repositorio.
- Identificacao de arquivos, modulos e dependencias aparentes.
- Geracao de rascunhos de documentacao.
- Sugestao de comentarios Doxygen.
- Conversao de conhecimento disperso em guias ou runbooks.
- Identificacao de lacunas e documentos candidatos.
- Padronizacao de linguagem e formato.

## Usos restritos

Exigem revisao humana reforcada:

- Release, assinatura, OTA e update.
- CAS/Nagra, criptografia, chaves e seguranca.
- Comportamento de hardware.
- Fluxos de negocio ou produto.
- Contratos de API.
- Troubleshooting operacional.
- Decisoes arquiteturais.

## Fluxo recomendado

```text
1. IA faz levantamento
2. IA gera rascunho
3. Responsavel tecnico revisa fatos
4. Comandos e procedimentos sao testados
5. Documento e ajustado
6. Documento entra em revisao por PR
7. Documento e publicado no MkDocs
```

## Criterios de aprovacao humana

Antes de aceitar um documento apoiado por IA, verificar:

- O documento cita arquivos reais do repositorio.
- O comportamento descrito foi confirmado no codigo ou por especialista.
- Comandos foram testados ou marcados como pendentes.
- Hipoteses foram removidas ou explicitadas.
- Nao ha informacao sensivel indevida.
- O texto diferencia fato, inferencia e pendencia.
- O dono do modulo revisou o conteudo.

## Marcadores de confiabilidade

Use estes marcadores em rascunhos quando necessario:

- `Confirmado`: validado no codigo, teste ou por responsavel tecnico.
- `Inferido`: deduzido a partir do codigo, ainda sem confirmacao completa.
- `Pendente`: precisa de validacao humana antes de publicacao final.

Exemplo:

```md
## Observacoes

- Confirmado: o build release usa `configure-release.sh`.
- Inferido: `MBGUI_USE_RLOTTIE` controla animacoes LVGL baseadas em rlottie.
- Pendente: validar com release owner se o fluxo Nagra atual ainda usa todos os passos descritos.
```

## Checklist para PR

```md
## Uso de IA

- [ ] Nao usei IA nesta documentacao
- [ ] Usei IA apenas para rascunho ou levantamento
- [ ] Revisei tecnicamente todos os fatos
- [ ] Testei os comandos documentados quando aplicavel
- [ ] Marquei pendencias e inferencias
- [ ] O responsavel pelo modulo revisou o conteudo
```

## Boas praticas de prompt

Ao usar IA, fornecer contexto objetivo:

- Caminho dos arquivos analisados.
- Objetivo do documento.
- Publico-alvo.
- Nivel de detalhe esperado.
- Restricoes conhecidas.
- Trechos de codigo relevantes.
- Pedido explicito para separar fatos de inferencias.

Exemplo:

```text
Analise os arquivos src/dvb/*.h e src/dvb/*.cpp e gere um rascunho de mapa do modulo DVB.
Separe responsabilidades confirmadas, inferencias e pendencias de validacao.
Nao invente comportamento que nao esteja evidente no codigo.
```

## Regra principal

Documento produzido com apoio de IA so deve ser tratado como documentacao oficial depois de revisao humana registrada no fluxo normal de mudanca.



