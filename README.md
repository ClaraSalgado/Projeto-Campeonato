# Sistema de Gerenciamento de Campeonatos de eSports

## Sobre o projeto

Este projeto é um sistema desenvolvido em linguagem C para cadastrar e
gerenciar campeonatos de eSports, com persistência dos dados em um banco
de dados MySQL.

O programa permite criar um campeonato, cadastrar suas equipes e
jogadores, registrar partidas e resultados, consultar campeonatos já
cadastrados e visualizar a classificação e a matriz de resultados.

## Funcionalidades

-   Criar um novo campeonato com nome, jogo, data de início, duração e
    quantidade de equipes.
-   Consultar campeonatos existentes e acessar um campeonato pelo seu
    identificador.
-   Cadastrar equipes vinculadas a um campeonato.
-   Cadastrar jogadores vinculados às equipes.
-   Verificar se o nome de uma equipe, o e-mail ou o telefone de um
    jogador já está cadastrado.
-   Cadastrar partidas entre duas equipes diferentes.
-   Registrar os pontos obtidos pelas equipes em cada partida.
-   Atualizar e consultar a classificação das equipes.
-   Ordenar a classificação pela pontuação, em ordem decrescente.
-   Exibir uma matriz com os resultados das equipes por partida.
-   Atualizar o nome de uma equipe e excluir um jogador por meio das
    rotinas disponíveis no banco de dados.

## Tecnologias e conceitos utilizados

### Linguagem e banco de dados

-   Linguagem C.
-   MySQL.
-   Biblioteca MySQL C API (`mysql.h`) para conexão e operações no banco
    de dados.

### Conceitos de programação

-   **Structs:** estruturas para representar campeonato, equipe,
    jogador, partida, resultado e classificação.
-   **Vetores:** armazenamento dos registros durante a execução do
    programa.
-   **Ponteiros:** passagem de endereços de estruturas e utilização da
    conexão com o MySQL.
-   **Busca linear:** localização de equipes e registros por meio de
    percorrimento sequencial.
-   **Busca binária:** localização de equipe pelo ID, considerando os
    IDs em ordem.
-   **Selection Sort:** ordenação da classificação pela maior pontuação.
-   **Matrizes:** armazenamento e exibição dos pontos de cada equipe em
    cada partida.
-   **Modularização:** separação das responsabilidades em funções e
    arquivos.
-   **Validação de dados:** verificações de limites, cadastros
    duplicados e valores informados.

## Estrutura dos arquivos

  -----------------------------------------------------------------------
  Arquivo                             Responsabilidade
  ----------------------------------- -----------------------------------
  `main.c`                            Ponto de entrada do programa, menu
                                      e fluxo principal de execução.

  `funcoes.c`                         Implementação dos cadastros,
                                      buscas, registro de resultados,
                                      ordenação e exibição da matriz.

  `funcoes.h`                         Protótipos das funções
                                      implementadas em `funcoes.c`.

  `models.h`                          Definição das estruturas
                                      (`structs`) utilizadas no sistema.

  `banco.c`                           Conexão com o MySQL e operações de
                                      inserção, consulta, atualização,
                                      exclusão e carregamento de dados.

  `banco.h`                           Declarações das funções de acesso
                                      ao banco de dados.

  `db_esports(2).sql`                 Script SQL de criação do banco de
                                      dados e de suas tabelas.

  `teste_conexao.c`                   Programa separado para testar a
                                      conexão com o MySQL.
  -----------------------------------------------------------------------

## Modelo do banco de dados

O banco de dados utilizado chama-se `esports` e possui seis tabelas:

  -----------------------------------------------------------------------
  Tabela                              Descrição
  ----------------------------------- -----------------------------------
  `campeonato`                        Armazena os dados dos campeonatos.

  `equipes`                           Armazena as equipes e sua
                                      associação com um campeonato.

  `jogadores`                         Armazena os jogadores e sua
                                      associação com uma equipe.

  `partidas`                          Armazena data, horário, duração e
                                      equipes participantes.

  `resultado_partida`                 Armazena os pontos registrados em
                                      cada partida.

  `classificacao`                     Armazena a pontuação e a posição
                                      das equipes.
  -----------------------------------------------------------------------

As relações entre as tabelas são estabelecidas por chaves primárias e
estrangeiras.

## Pré-requisitos

Antes de executar o projeto, é necessário ter:

-   Um compilador C, como GCC ou Clang.
-   MySQL Server instalado e em execução.
-   MySQL Connector/C ou os arquivos de desenvolvimento da biblioteca
    MySQL C API, incluindo `mysql.h`.
-   A biblioteca cliente do MySQL disponível para compilação e execução.

## Configuração do banco de dados

1.  Inicie o servidor MySQL.
2.  Abra o MySQL Workbench ou outro cliente MySQL.
3.  Revise o arquivo SQL do projeto antes de executá-lo.
4.  Execute o script para criar o banco `esports` e as tabelas.
5.  Confira as credenciais e os dados de conexão configurados em
    `banco.c`.

**Atenção:** o script SQL fornecido contém
`DROP DATABASE IF EXISTS esports;`. Essa instrução exclui o banco
`esports` existente e seus dados antes de recriá-lo. Faça backup e só
execute o script sabendo que os dados anteriores serão apagados.

### Configuração da conexão

No código atual, a conexão está configurada para:

-   Servidor: `localhost`
-   Usuário: `root`
-   Senha: vazia
-   Banco de dados: `esports`
-   Porta: `3306`

Se sua instalação do MySQL utilizar outras credenciais, altere os
parâmetros da função `mysql_real_connect()` em `banco.c` antes de
executar o programa. Não compartilhe senhas reais no repositório.

## Compilação e execução

A compilação depende do sistema operacional e do local de instalação dos
arquivos e bibliotecas do MySQL.

Em um ambiente no qual `mysql.h` e a biblioteca cliente estejam
configurados nos caminhos padrão, um exemplo de compilação é:

``` bash
gcc main.c funcoes.c banco.c -o esports $(mysql_config --cflags --libs)
```

Em seguida, execute:

``` bash
./esports
```

Se `mysql_config` não estiver disponível, consulte as instruções da
instalação do MySQL Connector/C para configurar os caminhos de inclusão
e de linkedição da biblioteca no compilador.

## Fluxo básico de utilização

Ao iniciar, o programa apresenta as opções:

1.  **Criar novo campeonato:** permite informar os dados do campeonato e
    cadastrar equipes, jogadores, partidas e resultados. Ao longo do
    processo, os dados são persistidos no MySQL.
2.  **Acessar campeonato existente:** lista os campeonatos disponíveis,
    solicita o ID desejado e carrega os dados associados ao campeonato.
3.  **Sair:** encerra o programa.

Após o registro dos resultados, o sistema apresenta a matriz de
resultados e a classificação ordenada das equipes.

## Observações

-   Os vetores utilizados no programa possuem limites definidos no
    código, como até 50 campeonatos, 100 equipes, 500 jogadores e 100
    partidas/resultados.
-   A busca binária por ID pressupõe que o vetor de equipes esteja
    ordenado pelos IDs.
-   A conexão com o banco deve estar disponível para utilizar as
    operações de persistência.
-   O arquivo `teste_conexao.c` pode ser utilizado separadamente para
    verificar a conexão com o MySQL, após configurar as credenciais
    necessárias.

## Autores

Projeto acadêmico desenvolvido como atividade de programação em
linguagem C, com integração ao banco de dados MySQL.

- Amanda Campos de Souza - RA 10782434
- Danilo Rasquinho Lima Santos - RA 10782439
- Maria Clara de Moraes Salgado - RA 10772005
- Natália Mota Sylvestrin - RA 10771890
- Thais Sgarbosa Specian - RA 10771886
