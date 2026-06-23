# Estrutura da Documentação

Este documento explica como organizar, criar e atualizar arquivos Markdown no portal de documentação do MBGUI, que agora utiliza o **MkDocs Material** como gerador de site estático.

## Visão Geral

A documentação do MBGUI está organizada assim:

```
doc/documentacao-mbgui/
├── mkdocs.yml          # Configuração do portal (tema, navegação, extensões)
├── requirements.txt    # Dependência Python para instalar o MkDocs
├── docs/               # Todo o conteúdo Markdown fica aqui dentro
│   ├── index.md        # Página inicial do portal
│   ├── 00-COMO-USAR-ESTRUTURA-DA-DOCUMENTACAO.md
│   ├── 01-ARCHITECTURE.md
│   ├── 02-SETUP_AND_BUILD.md
│   ├── ...
│   ├── bugs/           # Bugs documentados
│   ├── guias/          # Guias práticos
│   ├── processos/      # Processos internos
│   └── releases/       # Notas de release
└── site/               # HTML gerado (não editar manualmente)
```

## Onde Colocar Cada Tipo de Documento

| Pasta | Quando usar |
|---|---|
| `docs/` | Documentação técnica central do produto (arquitetura, build, banco, HAL, UI, fluxos). |
| `docs/bugs/` | Bugs documentados, análises técnicas, PRDs de correção e histórico de problemas. |
| `docs/guias/` | Guias práticos, onboarding técnico, passo a passo de uso e checklists operacionais. |
| `docs/processos/` | Processos internos do time, rotinas operacionais e padrões de execução. |
| `docs/releases/` | Notas de release, histórico de versões e consolidação de mudanças publicadas. |

As pastas que possuem `README.md` funcionam como página ?ndice da seção no portal.

## Como Criar um Novo Documento

### Passo 1: Criar o arquivo `.md`

Crie o arquivo na pasta adequada dentro de `docs/`. Exemplo:

```powershell
# Documento técnico na raiz
docs/09-NOVO-TEMA.md

# Guia pratico
docs/guias/guia_configuracao_rede.md

# Processo
docs/processos/03-ATUALIZACAO-OTA.md
```

### Passo 2: Registrar no `mkdocs.yml`

!!! warning "Importante"
    Diferente do sistema anterior, agora ? necessário adicionar o novo arquivo na seção `nav` do arquivo `mkdocs.yml` para que ele apareca no menu lateral do portal.

Abra o arquivo `mkdocs.yml` e adicione uma entrada na seção `nav`. Exemplo:

```yaml
nav:
  - Início: index.md
  - Visão Geral:
      - Como Usar a Documentação: 00-COMO-USAR-ESTRUTURA-DA-DOCUMENTACAO.md
      - Arquitetura: 01-ARCHITECTURE.md
      - Setup e Build: 02-SETUP_AND_BUILD.md
      - Novo Tema: 09-NOVO-TEMA.md           # <-- nova página aqui
  - Processos:
      - Standby do Receptor: processos/01-STANDBY-RECEPTOR.md
      - Instala Fácil: processos/02-INSTALA-FACIL-RECEPTOR.md
      - Atualizacao OTA: processos/03-ATUALIZACAO-OTA.md  # <-- novo processo
```

O caminho do arquivo ? relativo a pasta `docs/`.

### Passo 3: Gerar o portal (Build)

Execute o comando abaixo a partir da pasta `doc/documentacao-mbgui/`:

```powershell
py -3.12 -m mkdocs build --clean
```

O resultado sera gerado na pasta `site/`.

### Passo 4: Acessar o portal

Abra o arquivo `site/index.html` pelo Windows Explorer (duplo clique). Não ? necessário nenhum servidor web rodando.

## Regra Prática

Antes de criar um novo documento, responda:

1. Este documento explica o produto em si? → `docs/`
2. Este documento explica um bug? → `docs/bugs/`
3. Este documento ensina um fluxo prático? → `docs/guias/`
4. Este documento descreve um processo do time? → `docs/processos/`
5. Este documento registra mudanças de release? → `docs/releases/`

## Boas Práticas para Novos Arquivos

- Comece com um título claro no topo do arquivo usando `#`.
- Use seções com `##` e `###`.
- Prefira nomes descritivos e padronizados.
- Quando houver fluxo visual, use Mermaid (suportado nativamente pelo MkDocs Material).
- Quando houver documento de bug, registre ambiente, reprodução, impacto e critério de aceite.
- Quando houver processo, escreva de forma que outra pessoa consiga executar sem depender de contexto informal.

### Recursos extras do MkDocs Material

O MkDocs Material suporta caixas de destaque (admonitions). Use assim:

```markdown
!!! note "Titulo da nota"
    Conteúdo da nota aqui.

!!! warning "Atencao"
    Conteúdo do aviso aqui.

!!! tip "Dica"
    Conteúdo da dica aqui.
```

Blocos de código com syntax highlighting e botão de copiar:

````markdown
```c
int main() {
    printf("Hello MBGUI\n");
    return 0;
}
```
````

## Convenção de Nomes

| Tipo | Padrão | Exemplo |
|---|---|---|
| Técnico central | `NN-NOME-DO-TEMA.md` | `09-NETWORKING.md` |
| Guia | `guia_<tema>.md` | `guia_configuracao_rede.md` |
| Processo | `NN-NOME-DO-PROCESSO.md` | `03-ATUALIZACAO-OTA.md` |
| Release | `release-<versao>.md` | `release-2.5.0.md` |
| Bug/PRD | `PRD-BUG-<id>-<tema>.md` | `PRD-BUG-042-audio-sync.md` |

## Quando Criar Nova Pasta

Crie uma nova pasta dentro de `docs/` somente quando houver um novo tipo de documentação recorrente. Exemplos:

- `docs/integracoes/`
- `docs/diagnosticos/`
- `docs/laboratorio/`

Ao criar uma nova pasta, adicione uma nova seção correspondente no `nav` do `mkdocs.yml`.

## Pré-requisitos para Gerar o Portal

Na máquina que for executar o build (não ? necessário para quem apenas lê a documentação):

1. **Python 3** instalado (versão 3.8 ou superior).
2. Instalar as dependências:

```powershell
py -3.12 -m pip install -r requirements.txt
```

## Resumo do Fluxo

1. Escolher a pasta certa dentro de `docs/`.
2. Criar ou atualizar o arquivo `.md`.
3. Registrar a nova página no `nav` do `mkdocs.yml`.
4. Rodar `py -3.12 -m mkdocs build --clean` para regenerar o portal.
5. Abrir `site/index.html` para validar.

## Padrão Recomendado de Navegação

Para manter o portal escalavel:

- use a raiz de `docs/` para documentação técnica central
- use o `README.md` de cada pasta como página ?ndice da seção
- adicione no menu lateral apenas páginas de entrada ou documentos realmente recorrentes
- quando houver muitos arquivos de uma mesma categoria, consolide por tema ou release antes de expandir o `nav`
