# Arquitetura

## Visao de alto nivel

Explique como o sistema e dividido em camadas. O ideal e que qualquer pessoa nova consiga entender onde procurar cada tipo de responsabilidade.

## Camadas sugeridas

### Interface

Responsavel pelas telas, navegacao, eventos de usuario e apresentacao visual.

### Aplicacao e orquestracao

Responsavel por fluxo principal, estado global, eventos e coordenacao entre modulos.

### Dominio e regras

Responsavel por configuracoes, modelos internos, regras de negocio e identidade dos dados.

### Plataforma e hardware

Responsavel por tuner, demux, display, audio, video, watchdog e integracoes com chipset.

### Persistencia e integracoes

Responsavel por banco local, arquivos de estado, APIs locais e servicos auxiliares.

## Fluxo principal do sistema

Preencher com um resumo do caminho feliz:

1. Sistema inicia
2. Configuracoes basicas sao carregadas
3. Modulos principais sobem
4. UI fica disponivel
5. Usuario interage com canais, player e configuracoes

## Riscos arquiteturais

Use esta secao para registrar pontos que exigem cuidado constante.

Exemplos:

- loops muito apertados;
- ponteiros ou referencias invalidas em fluxos assincronos;
- dependencias fortes entre UI e estado global;
- comportamento diferente por chipset.

## Perguntas que esta pagina deve responder

- Onde cada responsabilidade mora?
- Qual modulo costuma ser afetado por cada tipo de bug?
- Quais partes do sistema sao mais sensiveis?
