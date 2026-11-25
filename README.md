# Sistema de Gerenciamento de Campeonatos de eSports

## Sobre o projeto

Este projeto consiste em um sistema desenvolvido em linguagem C para gerenciar campeonatos de eSports, utilizando o MySQL para armazenar e recuperar os dados.

O sistema permite cadastrar campeonatos, equipes, jogadores, partidas e resultados. Também realiza consultas, apresenta uma matriz de resultados e organiza a classificação das equipes de acordo com a pontuação registrada.

## Funcionalidades

- Criar um novo campeonato.
- Consultar campeonatos cadastrados e acessar um campeonato pelo ID.
- Cadastrar equipes vinculadas a um campeonato.
- Cadastrar jogadores vinculados às equipes.
- Verificar a existência de nomes de equipes, e-mails e telefones já cadastrados.
- Buscar equipes pelo nome ou pelo ID.
- Cadastrar partidas entre duas equipes.
- Registrar os pontos das equipes em cada partida.
- Atualizar e consultar a classificação no banco de dados.
- Ordenar a classificação por pontuação, em ordem decrescente.
- Exibir uma matriz com os pontos de cada equipe em cada partida.
- Alterar o nome de uma equipe.
- Excluir um jogador.

## Tecnologias utilizadas

- **Linguagem:** C.
- **Banco de dados:** MySQL.
- **Integração com o banco:** MySQL C API, por meio da biblioteca `mysql.h`.
- **Ferramentas de desenvolvimento:** compilador C e MySQL Workbench ou outro cliente MySQL.

## Conceitos de programação aplicados

- **Structs:** organização dos dados de campeonatos, equipes, jogadores, partidas, resultados e classificação.
- **Vetores:** armazenamento dos registros durante a execução.
- **Ponteiros:** passagem de estruturas por endereço e manipulação da conexão com o banco.
- **Busca linear:** procura sequencial de registros, como a busca de equipe pelo nome.
- **Busca binária:** procura de equipe pelo ID em um vetor ordenado por identificador.
- **Selection Sort:** ordenação da classificação pela pontuação, da maior para a menor.
- **Matrizes:** armazenamento dos pontos de cada equipe em cada partida.
- **Modularização:** divisão do programa em arquivos e funções com responsabilidades específicas.
- **Validação de dados:** verificação de limites, valores inválidos e cadastros duplicados.

## Organização dos arquivos

- **`main.c`** — contém a função principal, o menu e o fluxo de execução do sistema.
- **`funcoes.c`** — implementa os cadastros, as buscas, o registro de resultados, a ordenação da classificação e a exibição da matriz.
- **`funcoes.h`** — reúne os protótipos das funções implementadas em `funcoes.c`.
- **`models.h`** — define as estruturas (`structs`) utilizadas pelo sistema.
- **`banco.c`** — implementa a conexão com o MySQL e as operações de inserção, consulta, atualização, exclusão e carregamento de dados.
- **`banco.h`** — declara as funções responsáveis pelo acesso ao banco de dados.
- **`db_esports(2).sql`** — contém o script SQL para criação do banco de dados e das tabelas. Se o arquivo tiver outro nome no repositório, utilize o nome que aparece nele.
- **`teste_conexao.c`** — arquivo separado para testar a conexão com o MySQL.

## Estrutura do banco de dados

O banco de dados utilizado pelo projeto chama-se **`esports`** e possui seis tabelas:

1. **`campeonato`** — armazena o nome do campeonato, o jogo, a data de início, a duração e a quantidade de equipes.
2. **`equipes`** — armazena o nome das equipes, a quantidade de membros e o ID do campeonato ao qual pertencem.
3. **`jogadores`** — armazena nome, idade, e-mail, telefone e o ID da equipe de cada jogador.
4. **`partidas`** — armazena data, horário, duração e os IDs das duas equipes participantes.
5. **`resultado_partida`** — armazena os pontos das duas equipes e o ID da partida correspondente.
6. **`classificacao`** — armazena a pontuação, a posição e o ID da equipe.

As tabelas utilizam chaves primárias e estrangeiras para identificar os registros e estabelecer os relacionamentos entre campeonatos, equipes, jogadores, partidas, resultados e classificação.

## Pré-requisitos

Para compilar e executar o projeto, é necessário:

- Ter um compilador C instalado, como GCC ou Clang.
- Ter o MySQL Server instalado e em execução.
- Ter a MySQL C API (MySQL Connector/C ou biblioteca de desenvolvimento equivalente) instalada.
- Ter os arquivos de cabeçalho e bibliotecas do MySQL disponíveis para o compilador.

## Configuração do banco de dados

1. Inicie o servidor MySQL.
2. Abra o MySQL Workbench ou outro cliente MySQL.
3. Abra o arquivo SQL do projeto.
4. Revise o script e execute-o para criar o banco `esports` e suas tabelas.
5. Confira as configurações de conexão utilizadas em `banco.c`.

### Configurações de conexão atuais

A função `Conectar_Banco()` está configurada com os seguintes parâmetros:

- **Servidor:** `localhost`
- **Usuário:** `root`
- **Senha:** vazia
- **Banco de dados:** `esports`
- **Porta:** `3306`

Caso sua instalação utilize outro usuário, senha, servidor ou porta, ajuste os parâmetros de `mysql_real_connect()` em `banco.c`.

**Atenção:** o script SQL enviado contém o comando `DROP DATABASE IF EXISTS esports;`. Ele apaga o banco `esports` existente, incluindo seus dados, antes de criá-lo novamente. Faça backup e execute o script somente se estiver ciente dessa consequência.

## Compilação e execução

A compilação depende da instalação do MySQL Connector/C e da configuração dos caminhos de inclusão e das bibliotecas no sistema.

Em ambientes nos quais `mysql_config` está disponível, um exemplo de compilação é:

```bash
gcc main.c funcoes.c banco.c -o esports $(mysql_config --cflags --libs)
```

Para executar o programa:

```bash
./esports
```

Se `mysql_config` não estiver disponível, consulte a documentação da biblioteca MySQL instalada para configurar manualmente os caminhos de compilação e linkedição.

O arquivo `teste_conexao.c` é um programa independente. Ele pode ser compilado separadamente para verificar se a conexão com o MySQL está funcionando.

## Como utilizar

Ao iniciar, o programa apresenta um menu com as opções:

1. **Criar novo campeonato:** informa os dados do campeonato e permite cadastrar equipes, jogadores, partidas e resultados. Os registros são enviados ao banco de dados.
2. **Acessar campeonato existente:** apresenta os campeonatos cadastrados, solicita o ID do campeonato e carrega os dados relacionados.
0. **Sair:** encerra o programa.

No fluxo de criação, após o cadastro das partidas, são registrados os resultados. O sistema atualiza a classificação, exibe a matriz de resultados e apresenta a classificação ordenada.

## Limites definidos no código

O programa utiliza vetores com limites máximos definidos:

- Até 50 campeonatos.
- Até 100 equipes.
- Até 500 jogadores.
- Até 100 partidas.
- Até 100 resultados.

## Observações

- O servidor MySQL precisa estar ativo para que as operações de banco de dados funcionem.
- A busca binária por ID depende de o vetor de equipes estar ordenado pelos IDs.
- O script SQL deve ser executado antes de utilizar o sistema pela primeira vez.
- Não publique senhas reais ou outras credenciais no repositório.

## Autores

Projeto acadêmico desenvolvido como atividade de programação em linguagem C, com integração ao banco de dados MySQL.

⁠- Amanda Campos de Souza - RA 10782434
- Danilo Rasquinho Lima Santos - RA 10782439
- Maria Clara de Moraes Salgado - RA 10772005
- Natália Mota Sylvestrin - RA 10771890
- Thais Sgarbosa Specian - RA 107718860 
