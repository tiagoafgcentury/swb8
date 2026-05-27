# Organizacao do Codigo

## Objetivo

Explicar como o repositorio esta dividido e onde normalmente cada tipo de alteracao deve acontecer.

## Mapa inicial

Preencher ou ajustar conforme o codigo real:

- `src/`: codigo principal da aplicacao
- `src/common/`: tipos, utilitarios e configuracoes
- `src/tasks/`: tarefas e processamento de eventos
- `src/hal/`: abstracao de hardware
- `src/dvb/`: tabelas e logica DVB
- `src/cas/`: integracoes CAS
- `src/tpm/`: API local e HTTP
- `ui/`: interface e telas
- `res/`: recursos visuais
- `doc/`: documentacao

## Como decidir onde alterar

Use perguntas simples:

- E regra de negocio? Veja `common`, `tasks` ou modulo funcional.
- E hardware ou driver? Veja `hal`.
- E fluxo de interface? Veja `ui`.
- E parsing ou transporte DVB? Veja `dvb`.
- E persistencia ou banco? Procure modulos de estado e database.

## Boas praticas

- evitar mudar muitas camadas sem necessidade;
- documentar efeitos colaterais;
- registrar dependencias entre modulos;
- atualizar documentacao quando um fluxo estrutural mudar.

## Sinais de alerta

- uma correcao pequena exige tocar modulos demais;
- uma tela depende de detalhes internos de muitos subsistemas;
- nomes de arquivos nao deixam clara a responsabilidade.
