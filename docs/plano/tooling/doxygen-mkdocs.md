# Doxygen e MkDocs

## Objetivo

Usar Doxygen e MkDocs de forma complementar:

- MkDocs organiza a documentacao para leitura humana, onboarding, arquitetura, processos e operacao.
- Doxygen gera referencia tecnica a partir do codigo C/C++.

## Papel do MkDocs

MkDocs deve ser a porta de entrada da documentacao.

Conteudos recomendados:

- Primeiros passos.
- Ambiente local.
- Build debug e release.
- Arquitetura por modulo.
- Fluxos principais.
- Guias de desenvolvimento.
- Runbooks operacionais.
- Processo de release.
- ADRs.
- Referencias manuais, como flags CMake, scripts e variaveis.

`docs/plano/README.md` pode continuar existindo para leitura direta no Git; o portal MkDocs usa `docs/ppt-plano/src/index.md` como pagina inicial gerada.

## Papel do Doxygen

Doxygen deve gerar referencia tecnica para:

- Classes, structs e enums.
- Funcoes publicas ou compartilhadas entre modulos.
- Interfaces de HAL.
- Contratos entre tasks.
- Tipos e estruturas DVB.
- Interfaces de CAS/Nagra expostas ao restante do sistema.
- APIs internas usadas por UI, TPM, demux, player e infraestrutura comum.

Evite usar Doxygen para documentar obviedades. O foco deve ser contrato e intencao.

Bom comentario:

```cpp
/**
 * Atualiza a tabela de canais a partir dos dados demultiplexados.
 *
 * A chamada assume que a tabela NIT ja foi processada. Retorna falso quando
 * os dados recebidos estao incompletos ou inconsistentes para persistencia.
 */
bool update_channel_list(...);
```

Comentario fraco:

```cpp
/** Atualiza a lista de canais. */
bool update_channel_list(...);
```

## Integracao entre MkDocs e Doxygen

Modelo recomendado:

1. Gerar Doxygen em uma pasta de build, fora de `docs/`.
2. Publicar a saida HTML do Doxygen como secao tecnica do portal.
3. Criar paginas MkDocs que apontem para areas relevantes do Doxygen.
4. Nao versionar HTML gerado, salvo se houver decisao explicita para publicacao estatica sem CI.

Exemplo de saida:

```text
build/
  doxygen/
    html/
site/
  reference/
    doxygen/
```

## Politica para comentarios Doxygen

Adicionar ou revisar comentarios Doxygen quando:

- Uma interface publica ou compartilhada e criada.
- Um contrato de parametro, retorno ou erro muda.
- Uma estrutura e usada por mais de um modulo.
- Uma funcao tem efeito colateral relevante.
- Uma funcao depende de ordem de chamada, estado global, hardware ou timing.
- O codigo envolve CAS, OTA, assinatura, DVB, HAL, persistencia ou update.

Nao exigir Doxygen detalhado para:

- Funcoes locais triviais.
- Getters simples.
- Codigo temporario.
- Implementacoes obvias com escopo muito pequeno.

## Validacoes recomendadas

No CI ou rotina local:

```bash
doxygen Doxyfile
mkdocs build --strict
```

O modo `--strict` do MkDocs ajuda a falhar em links quebrados e referencias invalidas.

Para Doxygen, considerar falhar o build quando houver warnings em modulos criticos. Se isso gerar ruido no inicio, aplicar primeiro como relatorio e depois endurecer a regra.

## Faseamento

1. Criar `mkdocs.yml` minimo com paginas existentes.
2. Criar `Doxyfile` inicial com foco em `src/` e `ui/lvgl/`.
3. Excluir ou reduzir ruido de terceiros em `extern/`.
4. Definir grupos Doxygen por modulo.
5. Integrar geracao em pipeline ou script local.
6. Publicar portal MkDocs com link para referencia Doxygen.

## Decisoes pendentes

- Onde publicar o portal.
- Se HTML gerado sera versionado ou apenas publicado por pipeline.
- Quais warnings Doxygen devem quebrar build.
- Qual cobertura minima de comentarios sera exigida por modulo.
- Como tratar codigo de terceiros em `extern/`.


