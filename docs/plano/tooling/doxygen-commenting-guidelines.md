# Diretrizes para comentarios Doxygen assistidos por IA

## Objetivo

Definir como revisar arquivos `.h` e `.cpp` com apoio de IA para inserir comentarios Doxygen padronizados, gerando uma referencia tecnica util e revisada.

IA deve ajudar no rascunho e levantamento. A aprovacao final deve ser humana.

## Estrategia

A documentacao Doxygen sera criada em ondas:

```text
1. Revisar e comentar arquivos `.h`
2. Revisar e comentar arquivos `.cpp` relevantes
3. Gerar Doxygen
4. Corrigir warnings e ruido
5. Publicar/linkar a referencia no MkDocs
```

## Prioridade

Priorizar comentarios em headers antes de implementacoes.

Ordem recomendada:

1. `src/common`
2. `src/hal`
3. `src/dvb`
4. `src/tasks`
5. `src/tpm`
6. `ui/lvgl`
7. `src/cas/nagra`, somente com revisao autorizada

## O que comentar

Comentar com Doxygen:

- Arquivos que definem responsabilidades claras de modulo.
- Classes, structs e enums compartilhados.
- Funcoes publicas ou usadas entre modulos.
- Interfaces HAL.
- Contratos entre tasks.
- Tipos e estruturas DVB.
- APIs internas de TPM, HTTP ou producao.
- Funcoes com efeito colateral relevante.
- Funcoes que dependem de ordem de chamada, estado global, hardware, timing ou persistencia.

## O que evitar

Evitar comentario Doxygen em:

- Getters/setters triviais.
- Funcoes locais obvias.
- Codigo morto ou temporario.
- Comentarios que apenas repetem o nome da funcao.
- Suposicoes nao confirmadas pela revisao humana.

## Tags padrao

### Arquivo

```cpp
/**
 * @file mb_task_demux.h
 * @brief Define a task responsavel por processar tabelas DVB e atualizar dados de canais.
 */
```

### Funcao

```cpp
/**
 * @brief Atualiza a lista de canais a partir dos dados processados.
 *
 * @param force_update Indica se a atualizacao deve ocorrer mesmo sem mudanca detectada.
 * @return true quando a lista foi atualizada com sucesso.
 * @return false quando os dados de entrada sao invalidos ou incompletos.
 *
 * @note Esta funcao nao deve salvar listas vazias.
 */
bool update_channel_list(bool force_update);
```

### Struct

```cpp
/**
 * @brief Representa informacoes basicas de um canal demultiplexado.
 */
struct mb_channel_info {
    /** @brief Identificador do servico na tabela DVB. */
    uint16_t service_id;
};
```

### Enum

```cpp
/**
 * @brief Estados possiveis da task de demux.
 */
enum class demux_state {
    /** @brief Task aguardando dados de entrada. */
    idle,
    /** @brief Task processando tabelas DVB. */
    processing,
};
```

## Fluxo com IA

```text
1. Selecionar modulo e lista de arquivos
2. IA faz leitura e sugere comentarios Doxygen
3. Desenvolvedor revisa tecnicamente cada comentario
4. Comentarios sao aplicados em branch propria
5. `doxygen Doxyfile` e executado
6. Warnings novos sao analisados
7. MR e aberto no GitLab
8. Revisor tecnico aprova ou solicita ajustes
9. Redmine e atualizado
```

## Prompt recomendado

```text
Analise este arquivo C/C++ e sugira comentarios Doxygen apenas para interfaces relevantes.
Nao documente funcoes triviais.
Separe fatos confirmados de inferencias.
Nao invente comportamento que nao esteja evidente no codigo.
Use tags @file, @brief, @param, @return, @note e @warning quando fizer sentido.
```

## Checklist de MR

```md
## Doxygen

- [ ] Comentarios adicionados primeiro em `.h`
- [ ] Comentarios em `.cpp` adicionados somente onde ha valor tecnico
- [ ] Comentarios revisados por responsavel tecnico
- [ ] Nao foram documentadas suposicoes nao confirmadas
- [ ] Tags Doxygen seguem o padrao do projeto
- [ ] `doxygen Doxyfile` executado
- [ ] Warnings novos analisados
- [ ] Redmine atualizado
```

## Criterio de aceite

Uma tarefa de comentarios Doxygen esta pronta quando:

- Arquivos previstos foram revisados.
- Comentarios agregam valor tecnico real.
- Revisao humana foi concluida.
- Doxygen gera sem erro critico.
- Warnings novos foram registrados ou corrigidos.
- MR foi aprovado.

## Observacoes

Nao tentar documentar todo o codigo de uma vez. O objetivo e construir uma referencia tecnica confiavel, nao aumentar volume de comentario sem valor.

