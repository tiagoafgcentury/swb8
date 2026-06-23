# Template Basico Redmine - Bug Encontrado no Release

Use este modelo para abrir uma tarefa de bug no Redmine durante o teste de um novo release. Preencha com o maximo de informacoes que tiver no momento. Se algum campo nao se aplicar, escreva `nao se aplica` ou `nao informado`.

## Documentos relacionados

- [Fluxo de Desenvolvimento: Redmine, GitLab, MR e Release](fluxo-gitlab.md)
- [Template Redmine - Correcao do Desenvolvedor](TEMPLATE-REDMINE-CORRECAO-DESENVOLVEDOR.md)

## Quando usar

Use este template na criacao da tarefa de bug no Redmine. Ele e a entrada do fluxo e deve permitir que o desenvolvedor entenda o problema antes de iniciar a correcao.

## 1. Resumo do problema

```text
Descreva em poucas linhas o que aconteceu.
Exemplo: Ao entrar no Instala Facil e atualizar a lista, o receptor reinicia antes de concluir a busca.
```

## 2. Passos para reproduzir

1. `<passo 1>`
2. `<passo 2>`
3. `<passo 3>`
4. `<passo 4>`

## 3. Resultado atual

```text
O que aconteceu de errado?
Exemplo: O receptor reiniciou e voltou para a tela inicial.
```

## 4. Resultado esperado

```text
O que deveria acontecer?
Exemplo: A atualizacao da lista deveria finalizar sem reiniciar o receptor.
```

## 5. Frequência

Informe a frequência que ocorreu:

- [ ] Acontece sempre
- [ ] Acontece algumas vezes
- [ ] Aconteceu apenas uma vez
- [ ] Não consegui repetir

Se for intermitente, informe quantas vezes ocorreu:

```text
Exemplo: ocorreu 2 vezes em 5 tentativas.
```

## 6. Evidências obrigatórias quando possível

Anexar ou informar:

- **Foto ou video do problema:** `<anexo/link>`
- **Print da tela:** `<anexo/link>`

## 7. Comparação com release anterior

- [ ] Funcionava no release anterior
- [ ] Tambem falhava no release anterior
- [ ] Nao testado no release anterior
- [ ] Nao sei informar

Se souber, informe a versao comparada:

```text
Release anterior comparado: <versao>
Resultado no release anterior: <resultado>
```

## 8. Observações adicionais

```text
Informe qualquer detalhe que possa ajudar:
- canal usado
- satelite
- operadora
- sequencia exata no controle remoto
- se aconteceu apos reset de fabrica
- se aconteceu apos atualizar lista
- se aconteceu apos desligar/ligar da tomada
- se o problema depende de internet, cartao, CAS ou regionalizacao
```

## 9. Template para Redmine (wiki syntax)

Copiar e colar no campo **Descrição** da tarefa do Redmine. 

```text
*1. Resumo do problema*
_Descreva em poucas linhas o que aconteceu_

*2. Passos para reproduzir:*
# _a_
# _b_
# _c_
...

*3. Resultado atual*
_O que aconteceu de errado?_

*4. Resultado esperado*
_O que deveria acontecer?_

*5. Frequencia:*
_Informe a frequência que ocorreu_

*6. Evidencias obrigatorias quando possivel*
_Foto ou video do problema: <anexo/link>_
_Print da tela: <anexo/link>_

*7. Comparacao com release anterior*
_Release anterior comparado: <versao>_
_Resultado no release anterior: <resultado>_

*8. Observacoes adicionais*
_Informe qualquer detalhe que possa ajudar_
```
