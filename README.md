# CClustering

**CClustering** é uma implementação em C de algoritmos de clusterização de dados, incluindo K-médias e Agrupamento Hierárquico Aglomerativo (Single-Link e Complete-Link). O projeto também conta com um visualizador 2D simples utilizando X11 para exibir os resultados da clusterização e calcula o Índice Rand Ajustado (ARI) para avaliar a qualidade dos agrupamentos em relação a um gabarito.

## Funcionalidades

- **Algoritmos de Clusterização:**
  - K-médias (K-Means)
  - Agrupamento Hierárquico Aglomerativo (HAC) com:
    - Single-Link
    - Complete-Link
- **Manipulação de Dados:**
  - Carregamento de datasets a partir de arquivos de texto (`.txt`).
  - Salvamento dos resultados da clusterização em formato `.clu`.
  - Carregamento de resultados de clusterização para visualização.
- **Avaliação:**
  - Cálculo do **Índice Rand Ajustado (ARI)** para comparar os clusters gerados com um conjunto de referência.
- **Visualização:**
  - Plotagem 2D dos dados e seus respectivos clusters usando a biblioteca X11.
  - Interface de linha de comando interativa para seleção de algoritmos e parâmetros.

## Estrutura do Repositório

```
.
├── data/
│   ├── c2ds1-2sp.txt
│   ├── c2ds3-2g.txt
│   ├── monkey.txt
│   └── resultados/
│       ├── c2ds1-2sp.clu          # Gabarito para c2ds1-2sp.txt
│       ├── c2ds3-2g.clu           # Gabarito para c2ds3-2g.txt
│       ├── monkey.clu             # Gabarito para monkey.txt
│       └── G1_*.clu               # Arquivos de resultado gerados pelo programa
├── src/
│   ├── clustering.c
│   ├── clustering.h
│   ├── data_loader.c
│   ├── data_loader.h
│   ├── main.c
│   ├── x11_plotter.c
│   ├── x11_plotter.h
│   └── Makefile
└── README.md
```

- **`data/`**: Contém os datasets de entrada e os resultados da clusterização.
- **`data/resultados/`**: Subdiretório para os arquivos de gabarito e os resultados gerados.
- **`src/`**: Contém todo o código-fonte do projeto.

## Pré-requisitos

Para compilar e executar este projeto, você precisará de:
- Um compilador C (ex: `gcc`)
- Utilitário `make`
- Bibliotecas de desenvolvimento do X11.

## Compilação

O projeto utiliza um `Makefile` para simplificar a compilação.

1.  Navegue até o diretório `src`:
    ```bash
    cd src/
    ```
2.  Execute o comando `make`:
    ```bash
    make
    ```
3.  Um executável chamado `data_visualizer` será criado no mesmo diretório.

## Uso

O programa pode ser executado de duas formas: para realizar a clusterização de um dataset ou para visualizar um resultado de clusterização já existente.

### 1. Clusterizar um Dataset

Para executar um algoritmo de clusterização em um dataset, forneça o caminho para o arquivo `.txt`.

```bash
./data_visualizer ../data/nome_do_arquivo.txt
```

**Exemplo:**
```bash
./data_visualizer ../data/c2ds3-2g.txt
```

O programa solicitará interativamente que você escolha um algoritmo e seus parâmetros:

1.  **Escolha do Algoritmo**:
    ```
    Bem vindo(a) ao cclustering!
    Escolha o algoritmo desejado:
    1 - k-médias
    2 - single-link
    3 - complete-link
    ```

2.  **Entrada de Parâmetros**:
    - **Para K-médias (Opção 1):**
      - Número de clusters (k).
      - Número máximo de iterações.
    - **Para Single-Link/Complete-Link (Opções 2 e 3):**
      - Número mínimo de clusters (k) a ser gerado.
      - Número máximo de clusters (k) a ser gerado.

Após a execução, os resultados são salvos em `data/resultados/` com o nome `G1_<nome_do_arquivo>_<algoritmo>_<k>.clu`. O programa então calcula o ARI comparando o resultado com o arquivo de gabarito correspondente (se existir) e, por fim, abre uma janela X11 para exibir a visualização do último agrupamento gerado.

### 2. Visualizar um Resultado de Clusterização

Para visualizar um arquivo de clusterização (`.clu`) já existente, basta fornecer seu caminho como argumento. O programa carregará o dataset `.txt` correspondente automaticamente.

**Exemplo:**
```bash
./data_visualizer ../data/resultados/c2ds3-2g.clu
```

Uma janela X11 será aberta mostrando os pontos de dados coloridos de acordo com os clusters definidos no arquivo `.clu`.

### Controles da Janela de Visualização

- **`q` ou `Q`**: Pressione para fechar a janela e encerrar o programa.
- **Fechar a janela**: Clicar no botão de fechar da janela também encerrará o programa.

## Formato dos Arquivos

### Arquivo de Dados (`.txt`)
Os arquivos de dados de entrada devem ser formatados como valores separados por tabulação (`\t`) com um cabeçalho:

```
sample_label	d1	d2
c2g1s1	8.809783	7.611147
c2g1s2	4.110747	11.103186
...
```

### Arquivo de Cluster (`.clu`)
Os arquivos de cluster (tanto os gabaritos quanto os gerados) contêm o rótulo da amostra e o ID do cluster ao qual ela pertence, separados por tabulação.

```
sample_label	cluster_id
c2g1s1	0
c2g1s2	0
...
```
