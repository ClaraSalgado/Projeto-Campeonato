#include <stdio.h>
#include "funcoes.h"
#include "banco.h"

int main(void)
{

    MYSQL *conexao = Conectar_Banco();

    if (conexao == NULL)
    {
        printf("Nao foi possivel conectar ao banco.\n");
        return 1;
    }

    int opcao;

    printf("\n========== SISTEMA DE CAMPEONATOS ==========\n");
    printf("1 - Criar novo campeonato\n");
    printf("2 - Acessar campeonato existente\n");
    printf("0 - Sair\n");
    printf("Escolha uma opção: ");
    scanf("%d", &opcao);

    if (opcao == 0)
    {
        printf("Programa encerrado.\n");
        Desconectar_Banco(conexao);
        return 0;
    }

    if (opcao == 2)
    {

        Campeonato campeonatos[50];
        Equipe equipes[100];
        Jogador jogadores[500];
        Partida partidas[100];
        ResultadoPartida resultados[100];
        Classificacao classificacao[100];

        int id_campeonato;
        int qntd_equipes;
        int qntd_jogadores;
        int qntd_partidas;
        int qntd_resultados;
        int qntd_classificacao;

        printf("\n========== CAMPEONATOS DISPONIVEIS ==========\n");

        Consultar_Campeonatos_Banco(conexao);

        printf("\nDigite o ID do campeonato que deseja acessar: ");
        scanf("%d", &id_campeonato);

        char sql[150];

        snprintf(sql, sizeof(sql),
                 "SELECT ID_campeonato FROM campeonato "
                 "WHERE ID_campeonato = %d",
                 id_campeonato);

        if (mysql_query(conexao, sql))
        {
            printf("Erro ao consultar campeonato.\n");
            Desconectar_Banco(conexao);
            return 1;
        }

        MYSQL_RES *consulta = mysql_store_result(conexao);
        MYSQL_ROW linha = mysql_fetch_row(consulta);

        if (linha == NULL)
        {
            printf("Campeonato nao encontrado.\n");
            mysql_free_result(consulta);
            Desconectar_Banco(conexao);
            return 0;
        }

        mysql_free_result(consulta);

        printf("\nCampeonato encontrado! Recuperando dados...\n");

        qntd_equipes = Carregar_Equipes_Banco(
            conexao, equipes, id_campeonato);

        qntd_jogadores = Carregar_Jogadores_Banco(
            conexao, jogadores, id_campeonato);

        qntd_partidas = Carregar_Partidas_Banco(
            conexao, partidas, id_campeonato);

        qntd_resultados = Carregar_Resultados_Banco(
            conexao, resultados, id_campeonato);

        qntd_classificacao = Carregar_Classificacao_Banco(
            conexao, classificacao, id_campeonato);

        if (qntd_equipes < 0 ||
            qntd_jogadores < 0 ||
            qntd_partidas < 0 ||
            qntd_resultados < 0 ||
            qntd_classificacao < 0)
        {

            printf("Erro ao recuperar os dados do campeonato.\n");
            Desconectar_Banco(conexao);
            return 1;
        }

        printf("\n========== EQUIPES DO CAMPEONATO ==========\n");

        for (int i = 0; i < qntd_equipes; i++)
        {
            printf("\nID da equipe: %d\n", equipes[i].id_equipe);
            printf("Nome: %s\n", equipes[i].nome_equipe);
            printf("Quantidade de membros: %d\n", equipes[i].qntd_membros);
        }

        printf("\n========== JOGADORES DO CAMPEONATO ==========\n");

        for (int i = 0; i < qntd_jogadores; i++)
        {
            printf("\nID do jogador: %d\n", jogadores[i].id_jogador);
            printf("Nome: %s\n", jogadores[i].nome_jogador);
            printf("Idade: %d\n", jogadores[i].idade);
            printf("Email: %s\n", jogadores[i].email);
            printf("Telefone: %s\n", jogadores[i].telefone);
            printf("ID da equipe: %d\n", jogadores[i].id_equipe);
        }

        printf("\nDados recuperados com sucesso!\n");

        Desconectar_Banco(conexao);
        return 0;
    }

    if (opcao != 1)
    {
        printf("Opcao invalida.\n");
        Desconectar_Banco(conexao);
        return 1;
    }

    if (opcao == 1)
    {
        Campeonato campeonatos[50];
        Equipe equipes[100];
        Jogador jogadores[500];
        Partida partidas[100];
        ResultadoPartida resultados[100];
        Classificacao classificacao[100];
        int matriz_resultados[100][100] = {0};

        int qntd_campeonatos = 0;
        int qntd_equipes = 0;
        int qntd_jogadores = 0;
        int qntd_partidas = 0;
        int qntd_resultados = 0;
        int qntd_classificacao = 0;

        Campeonato novo_campeonato;
        Equipe nova_equipe;

        int id_campeonato;
        int quantidade_partidas;
        int posicao;
        int id_busca;
        char nome_busca[51];

        int retorno, c, existe;

        // CADASTRO DO CAMPEONATO

        printf("========== CAMPEONATO ==========\n");

        do
        {
            printf("Nome: ");
            retorno = scanf("%100[^\n]", novo_campeonato.nome);
            while ((c = getchar()) != '\n' && c != EOF)
            {
                // descarta o resto da linha (inclusive o '\n')
            }
            if (retorno == EOF)
                return 1;
            if (retorno != 1)
                printf("Nome do campeonato não pode ser vazio.\n");
        } while (retorno != 1);

        do
        {
            printf("Jogo: ");
            retorno = scanf("%50[^\n]", novo_campeonato.jogo);
            while ((c = getchar()) != '\n' && c != EOF)
            {
                // descarta o resto da linha (inclusive o '\n')
            }
            if (retorno == EOF)
                return 1;
            if (retorno != 1)
                printf("Jogo do campeonato não pode ser vazio.\n");
        } while (retorno != 1);

        printf("Data de inicio (AAAA-MM-DD): ");
        scanf("%10s", novo_campeonato.data_inicio);

        do
        {
            printf("Data de inicio (AAAA-MM-DD): ");
            retorno = scanf("%10s", novo_campeonato.data_inicio);
            while ((c = getchar()) != '\n' && c != EOF)
            {
                // descarta o resto da linha (inclusive o '\n')
            }
            if (retorno == EOF)
                return 1;
            if (retorno != 1)
                printf("Data de inicio do campeonato não pode ser vazia.\n");
        } while (retorno != 1);

        do
        {
            printf("Tempo de duracao: ");
            retorno = scanf("%20[^\n]", novo_campeonato.tempo_duracao);
            while ((c = getchar()) != '\n' && c != EOF)
            {
                // descarta o resto da linha (inclusive o '\n')
            }
            if (retorno == EOF)
                return 1;
            if (retorno != 1)
                printf("Tempo de duracao do campeonato não pode ser vazio.\n");
        } while (retorno != 1);

        do
        {
            printf("Quantidade de equipes: ");
            scanf("%d", &novo_campeonato.num_equipes);
            if (novo_campeonato.num_equipes <= 1)
            {
                printf("A quantidade de equipes deve ser pelo menos 2.\n");
            }
        } while (novo_campeonato.num_equipes <= 1);

        if (!Cadastrar_Campeonato(
                campeonatos,
                qntd_campeonatos,
                &novo_campeonato))
        {
            printf("Erro ao cadastrar campeonato.\n");
            return 1;
        }

        if (!Salvar_Campeonato_Banco(conexao, &novo_campeonato))
        {
            printf("Erro ao salvar campeonato no banco.\n");
            Desconectar_Banco(conexao);
            return 1;
        }

        campeonatos[qntd_campeonatos] = novo_campeonato;

        id_campeonato = novo_campeonato.id_campeonato;
        qntd_campeonatos++;

        printf("\nCampeonato cadastrado com sucesso!\n");

        // CADASTRO DAS EQUIPES

        for (int i = 0; i < novo_campeonato.num_equipes; i++)
        {

            printf("\n========== EQUIPE %d ==========\n", i + 1);

            do
            {
                existe = 0;
                printf("Nome da equipe: ");
                retorno = scanf("%50[^\n]", nova_equipe.nome_equipe);
                while ((c = getchar()) != '\n' && c != EOF)
                {
                    // descarta o resto da linha (inclusive o '\n')
                }
                if (retorno == EOF)
                    return 1;
                if (retorno != 1)
                    printf("Nome da equipe nao pode ser vazio.\n");

                existe = nome_equipe_existe(conexao, nova_equipe.nome_equipe);
                if (existe == -1)
                {
                    printf("Erro ao consultar o banco.\n");
                    Desconectar_Banco(conexao);
                    return 1;
                }
                if (existe == 1)
                {
                    printf("Já existe uma equipe com esse nome. Digite outro nome.\n");
                }
            } while (retorno != 1 || existe);

            do{
                printf("Quantidade de membros: ");
                scanf("%d", &nova_equipe.qntd_membros);
                if (nova_equipe.qntd_membros <= 0){
                    printf("A quantidade de membros deve ser maior que 0.\n");
                }
            } while (nova_equipe.qntd_membros <= 0);


            if (Cadastrar_Equipe(
                    equipes,
                    qntd_equipes,
                    nova_equipe,
                    id_campeonato))
            {

                if (!Salvar_Equipe_Banco(conexao, &equipes[qntd_equipes]))
                {
                    printf("Erro ao salvar equipe no banco.\n");
                    Desconectar_Banco(conexao);
                    return 1;
                }

                qntd_equipes++;

                printf("Equipe cadastrada e salva!\n");
            }
            else
            {
                printf("Erro ao cadastrar equipe.\n");
                Desconectar_Banco(conexao);
                return 1;
            }
        }

        // CADASTRO DOS JOGADORES

        printf("\n========== JOGADORES ==========\n");

        qntd_jogadores = Cadastrar_Jogadores(
            jogadores,
            equipes,
            qntd_equipes);

        for (int i = 0; i < qntd_jogadores; i++)
        {

            if (!Salvar_Jogador_Banco(conexao, &jogadores[i]))
            {

                printf("Erro ao salvar jogador no banco.\n");

                Desconectar_Banco(conexao);
                return 1;
            }
        }

        printf("\n%d jogadores cadastrados!\n", qntd_jogadores);

        // BUSCA LINEAR POR NOME

        printf("\n========== BUSCA EQUIPE ==========\n");

        printf("Digite o nome da equipe: ");
        scanf(" %50[^\n]", nome_busca);

        posicao = Buscar_Equipe_Pelo_Nome(
            equipes,
            qntd_equipes,
            nome_busca);

        if (posicao != -1)
        {
            printf("Equipe encontrada: %s\n",
                   equipes[posicao].nome_equipe);
            printf("ID: %d\n", equipes[posicao].id_equipe);
        }
        else
        {
            printf("Equipe nao encontrada.\n");
        }

        // BUSCA BINARIA POR ID

        printf("\n========== BUSCA ID ==========\n");

        printf("Digite o ID da equipe: ");
        scanf("%d", &id_busca);

        posicao = Buscar_Equipe_Pelo_ID(
            equipes,
            qntd_equipes,
            id_busca);

        if (posicao != -1)
        {
            printf("Equipe encontrada: %s\n",
                   equipes[posicao].nome_equipe);
            printf("ID: %d\n", equipes[posicao].id_equipe);
        }
        else
        {
            printf("Equipe nao encontrada.\n");
        }

        // CADASTRO DAS PARTIDAS

        printf("\n========== PARTIDAS ==========\n");

        do{
                printf("Quantidade de partidas: ");
                scanf("%d", &quantidade_partidas);
                if (quantidade_partidas < 1 || quantidade_partidas > 100){
                    printf("Quantidade de partidas invalida.\n");
                }
            } while (quantidade_partidas < 1 || quantidade_partidas > 100);

        for (int i = 0; i < quantidade_partidas; i++)
        {

            if (Cadastrar_Partida(
                    partidas,
                    qntd_partidas,
                    equipes,
                    qntd_equipes))
            {

                if (!Salvar_Partida_Banco(
                        conexao,
                        &partidas[qntd_partidas]))
                {
                    printf("Erro ao salvar partida no banco.\n");
                    Desconectar_Banco(conexao);
                    return 1;
                }

                qntd_partidas++;

                printf("Partida cadastrada e salva!\n");
            }
            else
            {
                printf("Erro ao cadastrar partida.\n");
                Desconectar_Banco(conexao);
                return 1;
            }
        }

        // INICIALIZACAO DA CLASSIFICACAO

        for (int i = 0; i < qntd_equipes; i++)
        {

            classificacao[i].id_classificacao = i + 1;
            classificacao[i].id_equipe = equipes[i].id_equipe;
            classificacao[i].pontuacao = 0;
            classificacao[i].posicao = 0;

            qntd_classificacao++;
        }

        Consultar_Classificacao_Banco(conexao, id_campeonato);

        // CADASTRO DOS RESULTADOS

        printf("\n========== RESULTADOS ==========\n");

        for (int i = 0; i < qntd_partidas; i++)
        {

            if (Cadastrar_Resultado(
                    resultados,
                    qntd_resultados,
                    partidas,
                    qntd_partidas,
                    equipes,
                    qntd_equipes,
                    classificacao,
                    qntd_classificacao,
                    matriz_resultados))
            {

                if (!Salvar_Resultado_Banco(
                        conexao,
                        &resultados[qntd_resultados]))
                {
                    printf("Erro ao salvar resultado no banco.\n");
                    Desconectar_Banco(conexao);
                    return 1;
                }

                if (!Atualizar_Classificacao_Banco(conexao, id_campeonato))
                {
                    printf("Erro ao atualizar classificacao.\n");
                    Desconectar_Banco(conexao);
                    return 1;
                }

                qntd_resultados++;

                printf("Resultado cadastrado e salvo!\n");
            }
            else
            {
                printf("Nao foi possivel cadastrar o resultado.\n");
                Desconectar_Banco(conexao);
                return 1;
            }
        }

        // EXIBICAO DA MATRIZ DE RESULTADOS

        Exibir_Matriz_Resultados(
            matriz_resultados,
            equipes,
            qntd_equipes,
            qntd_partidas);

        // CLASSIFICACAO FINAL

        Ordenar_Classificacao(
            classificacao,
            qntd_classificacao);

        printf("Classificacao salva no MySQL!\n");

        printf("\n========== CLASSIFICACAO FINAL ==========\n");

        for (int i = 0; i < qntd_classificacao; i++)
        {

            int indice_equipe = -1;

            for (int j = 0; j < qntd_equipes; j++)
            {

                if (equipes[j].id_equipe ==
                    classificacao[i].id_equipe)
                {
                    indice_equipe = j;
                    break;
                }
            }

            if (indice_equipe != -1)
            {

                printf("%d lugar - %s | %d pontos\n",
                       classificacao[i].posicao,
                       equipes[indice_equipe].nome_equipe,
                       classificacao[i].pontuacao);
            }
        }

        Atualizar_Nome_Equipe_Banco(conexao);

        Excluir_Jogador_Banco(conexao);
    }

    printf("\nPrograma encerrado.\n");

    Desconectar_Banco(conexao);

    return 0;
}