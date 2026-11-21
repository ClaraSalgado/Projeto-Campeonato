#include <stdio.h>
#include "funcoes.h"
#include "banco.h"

int main(void) {

        MYSQL *conexao = Conectar_Banco();

    if (conexao == NULL) {
        printf("Nao foi possivel conectar ao banco.\n");
        return 1;
    }

    Consultar_Campeonatos_Banco(conexao);

    Consultar_Equipes_Banco(conexao);

    Consultar_Jogadores_Banco(conexao);

    Consultar_Partidas_Banco(conexao);

    Consultar_Resultados_Banco(conexao);

    Equipe equipes_recuperadas[100];

    int qntd_equipes_recuperadas =
        Carregar_Equipes_Banco(conexao, equipes_recuperadas);

    if (qntd_equipes_recuperadas >= 0) {
        printf("\n===== EQUIPES RECUPERADAS =====\n");

        for (int i = 0; i < qntd_equipes_recuperadas; i++) {
            printf("ID: %d | Nome: %s | Campeonato: %d\n",
                equipes_recuperadas[i].id_equipe,
                equipes_recuperadas[i].nome_equipe,
                equipes_recuperadas[i].id_campeonato);
        }
    }
    
    Jogador jogadores_recuperados[500];

    int qntd_jogadores_recuperados =
        Carregar_Jogadores_Banco(conexao, jogadores_recuperados);

    if (qntd_jogadores_recuperados >= 0) {

        printf("\n===== JOGADORES RECUPERADOS =====\n");

        for (int i = 0; i < qntd_jogadores_recuperados; i++) {

            printf("ID: %d | Nome: %s | Idade: %d | Equipe: %d\n",
                jogadores_recuperados[i].id_jogador,
                jogadores_recuperados[i].nome_jogador,
                jogadores_recuperados[i].idade,
                jogadores_recuperados[i].id_equipe);
        }
    }

    Partida partidas_recuperadas[100];

int qntd_partidas_recuperadas =
    Carregar_Partidas_Banco(conexao, partidas_recuperadas);

if (qntd_partidas_recuperadas >= 0) {

    printf("\n===== PARTIDAS RECUPERADAS =====\n");

    for (int i = 0; i < qntd_partidas_recuperadas; i++) {

        printf("ID: %d | Data: %s | Horario: %s | "
               "Equipe 1: %d | Equipe 2: %d\n",
               partidas_recuperadas[i].id_partida,
               partidas_recuperadas[i].data_partida,
               partidas_recuperadas[i].horario,
               partidas_recuperadas[i].equipe_1,
               partidas_recuperadas[i].equipe_2);
    }
}

ResultadoPartida resultados_recuperados[100];

int qntd_resultados_recuperados =
    Carregar_Resultados_Banco(conexao, resultados_recuperados);

if (qntd_resultados_recuperados >= 0) {

    printf("\n===== RESULTADOS RECUPERADOS =====\n");

    for (int i = 0; i < qntd_resultados_recuperados; i++) {

        printf("ID: %d | Equipe vencedora: %s | "
               "Pontuacao 1: %d | Pontuacao 2: %d | Partida: %d\n",
               resultados_recuperados[i].id_resultado,
               resultados_recuperados[i].equipe_venc,
               resultados_recuperados[i].pont_eq1,
               resultados_recuperados[i].pont_eq2,
               resultados_recuperados[i].id_partida);
    }
}

Classificacao classificacao_recuperada[100];

int qntd_classificacao_recuperada =
    Carregar_Classificacao_Banco(conexao,
                                 classificacao_recuperada,
                                 9);

if (qntd_classificacao_recuperada >= 0) {

    printf("\n===== CLASSIFICACAO RECUPERADA =====\n");

    for (int i = 0; i < qntd_classificacao_recuperada; i++) {

        printf("Posicao: %d | Equipe: %d | Pontuacao: %d\n",
               classificacao_recuperada[i].posicao,
               classificacao_recuperada[i].id_equipe,
               classificacao_recuperada[i].pontuacao);
    }
}

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

    // CADASTRO DO CAMPEONATO

    printf("========== CAMPEONATO ==========\n");

    printf("Nome: ");
    scanf(" %100[^\n]", novo_campeonato.nome);

    printf("Jogo: ");
    scanf(" %50[^\n]", novo_campeonato.jogo);

    printf("Data de inicio (AAAA-MM-DD): ");
    scanf("%10s", novo_campeonato.data_inicio);

    printf("Tempo de duracao: ");
    scanf(" %20[^\n]", novo_campeonato.tempo_duracao);

    printf("Quantidade de equipes: ");
    scanf("%d", &novo_campeonato.num_equipes);

    if (!Cadastrar_Campeonato(
        campeonatos,
        qntd_campeonatos,
        &novo_campeonato
    )) {
        printf("Erro ao cadastrar campeonato.\n");
        return 1;
    }

    if (!Salvar_Campeonato_Banco(conexao, &novo_campeonato)) {
        printf("Erro ao salvar campeonato no banco.\n");
        Desconectar_Banco(conexao);
        return 1;
    }

    campeonatos[qntd_campeonatos] = novo_campeonato;

    id_campeonato = novo_campeonato.id_campeonato;
    qntd_campeonatos++;

printf("\nCampeonato cadastrado com sucesso!\n");

    // CADASTRO DAS EQUIPES

    for (int i = 0; i < novo_campeonato.num_equipes; i++) {

        printf("\n========== EQUIPE %d ==========\n", i + 1);

        printf("Nome da equipe: ");
        scanf(" %50[^\n]", nova_equipe.nome_equipe);

        printf("Quantidade de membros: ");
        scanf("%d", &nova_equipe.qntd_membros);

        
        if (Cadastrar_Equipe(
            equipes,
            qntd_equipes,
            nova_equipe,
            id_campeonato
        )) {

            if (!Salvar_Equipe_Banco(conexao, &equipes[qntd_equipes])) {
                printf("Erro ao salvar equipe no banco.\n");
                Desconectar_Banco(conexao);
                return 1;
            }

            qntd_equipes++;

            printf("Equipe cadastrada e salva!\n");

        } else {
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
    qntd_equipes
);

    
for (int i = 0; i < qntd_jogadores; i++) {

    if (!Salvar_Jogador_Banco(conexao, &jogadores[i])) {

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
        nome_busca
    );

    if (posicao != -1) {
        printf("Equipe encontrada: %s\n",
               equipes[posicao].nome_equipe);
        printf("ID: %d\n", equipes[posicao].id_equipe);
    } else {
        printf("Equipe nao encontrada.\n");
    }

    // BUSCA BINARIA POR ID

    printf("\n========== BUSCA ID ==========\n");

    printf("Digite o ID da equipe: ");
    scanf("%d", &id_busca);

    posicao = Buscar_Equipe_Pelo_ID(
        equipes,
        qntd_equipes,
        id_busca
    );

    if (posicao != -1) {
        printf("Equipe encontrada: %s\n",
               equipes[posicao].nome_equipe);
        printf("ID: %d\n", equipes[posicao].id_equipe);
    } else {
        printf("Equipe nao encontrada.\n");
    }

    // CADASTRO DAS PARTIDAS

    printf("\n========== PARTIDAS ==========\n");

    printf("Quantidade de partidas: ");
    scanf("%d", &quantidade_partidas);

    if (quantidade_partidas < 1 ||
        quantidade_partidas > 100) {
        printf("Quantidade de partidas invalida.\n");
        return 1;
    }

    
for (int i = 0; i < quantidade_partidas; i++) {

    if (Cadastrar_Partida(
        partidas,
        qntd_partidas,
        equipes,
        qntd_equipes
    )) {

        if (!Salvar_Partida_Banco(
            conexao,
            &partidas[qntd_partidas]
        )) {
            printf("Erro ao salvar partida no banco.\n");
            Desconectar_Banco(conexao);
            return 1;
        }

        qntd_partidas++;

        printf("Partida cadastrada e salva!\n");

    } else {
        printf("Erro ao cadastrar partida.\n");
        Desconectar_Banco(conexao);
        return 1;
    }
}


    // INICIALIZACAO DA CLASSIFICACAO

    for (int i = 0; i < qntd_equipes; i++) {

        classificacao[i].id_classificacao = i + 1;
        classificacao[i].id_equipe = equipes[i].id_equipe;
        classificacao[i].pontuacao = 0;
        classificacao[i].posicao = 0;

        qntd_classificacao++;
    }

    Consultar_Classificacao_Banco(conexao, id_campeonato);

    // CADASTRO DOS RESULTADOS

    printf("\n========== RESULTADOS ==========\n");

    

       
for (int i = 0; i < qntd_partidas; i++) {

    if (Cadastrar_Resultado(
        resultados,
        qntd_resultados,
        partidas,
        qntd_partidas,
        equipes,
        qntd_equipes,
        classificacao,
        qntd_classificacao,
        matriz_resultados
    )) {

        if (!Salvar_Resultado_Banco(
            conexao,
            &resultados[qntd_resultados]
        )) {
            printf("Erro ao salvar resultado no banco.\n");
            Desconectar_Banco(conexao);
            return 1;
        }

       if (!Atualizar_Classificacao_Banco(conexao, id_campeonato)) {
            printf("Erro ao atualizar classificacao.\n");
            Desconectar_Banco(conexao);
            return 1;
        }

        qntd_resultados++;

        printf("Resultado cadastrado e salvo!\n");

    } else {
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
        qntd_partidas
    );

    // CLASSIFICACAO FINAL

    Ordenar_Classificacao(
        classificacao,
        qntd_classificacao
    );



printf("Classificacao salva no MySQL!\n");


    printf("\n========== CLASSIFICACAO FINAL ==========\n");

    for (int i = 0; i < qntd_classificacao; i++) {

        int indice_equipe = -1;

        for (int j = 0; j < qntd_equipes; j++) {

            if (equipes[j].id_equipe ==
                classificacao[i].id_equipe) {
                indice_equipe = j;
                break;
            }
        }

        if (indice_equipe != -1) {

            printf("%d lugar - %s | %d pontos\n",
                   classificacao[i].posicao,
                   equipes[indice_equipe].nome_equipe,
                   classificacao[i].pontuacao);
        }
    }

    
    Atualizar_Nome_Equipe_Banco(conexao);

    Excluir_Jogador_Banco(conexao);

    printf("\nPrograma encerrado.\n");

    Desconectar_Banco(conexao);

    return 0;

}