#include <stdio.h>
#include <string.h>
#include "funcoes.h"

// CADASTRAR CAMPEONATO
int Cadastrar_Campeonato(
    Campeonato campeonatos[],
    int quantidade,
    Campeonato *novo_campeonato
) {
    if (quantidade >= 50) {
        return 0;
    }

    if (novo_campeonato->num_equipes < 2 ||
        novo_campeonato->num_equipes > 100) {
        return 0;
    }

    novo_campeonato->id_campeonato = quantidade + 1;

    campeonatos[quantidade] = *novo_campeonato;

    return 1;
}


int Cadastrar_Equipe(
    Equipe equipes[],
    int quantidade,
    Equipe nova_equipe,
    int id_campeonato
) {
    int total_membros = 0;

    if (quantidade >= 100) {
        return 0;
    }

    if (nova_equipe.qntd_membros < 1 ||
        nova_equipe.qntd_membros > 500) {
        return 0;
    }

    for (int i = 0; i < quantidade; i++) {

        total_membros += equipes[i].qntd_membros;

        if (strcmp(equipes[i].nome_equipe,
                   nova_equipe.nome_equipe) == 0) {
            printf("Nome de equipe ja cadastrado.\n");
            return 0;
        }
    }

    if (total_membros + nova_equipe.qntd_membros > 500) {
        return 0;
    }

    nova_equipe.id_equipe = quantidade + 1;
    nova_equipe.id_campeonato = id_campeonato;

    equipes[quantidade] = nova_equipe;

    return 1;
}


int Buscar_Equipe_Pelo_Nome(
    Equipe equipes[],
    int quantidade,
    char nome[]
) {
    for (int i = 0; i < quantidade; i++) {

        if (strcmp(equipes[i].nome_equipe, nome) == 0) {
            return i;
        }
    }

    return -1;
}
int Buscar_Equipe_Pelo_ID(
    Equipe equipes[],
    int quantidade,
    int id
) {
    int inicio = 0;
    int fim = quantidade - 1;

    while (inicio <= fim) {

        int meio = inicio + (fim - inicio) / 2;

        if (equipes[meio].id_equipe == id) {
            return meio;
        }

        if (equipes[meio].id_equipe < id) {
            inicio = meio + 1;
        } else {
            fim = meio - 1;
        }
    }

    return -1;
}


int Cadastrar_Jogadores(
    Jogador jogadores[],
    Equipe equipes[],
    int qntd_equipes
) {
    int total_jogadores = 0;

    for (int i = 0; i < qntd_equipes; i++) {

        for (int j = 0; j < equipes[i].qntd_membros; j++) {

            if (total_jogadores >= 500) {
                return total_jogadores;
            }

            jogadores[total_jogadores].id_jogador =
                total_jogadores + 1;

            jogadores[total_jogadores].id_equipe =
                equipes[i].id_equipe;

            printf("\n--- Jogador %d da equipe %s ---\n",
                   j + 1, equipes[i].nome_equipe);

            printf("Nome: ");
            scanf(" %100[^\n]",
                  jogadores[total_jogadores].nome_jogador);

            printf("Idade: ");
            scanf("%d", &jogadores[total_jogadores].idade);

            while (jogadores[total_jogadores].idade < 1 ||
                   jogadores[total_jogadores].idade > 120) {

                printf("Idade invalida. Digite novamente: ");
                scanf("%d", &jogadores[total_jogadores].idade);
            }

            printf("Email: ");
            scanf("%50s", jogadores[total_jogadores].email);

            printf("Telefone: ");
            scanf("%14s", jogadores[total_jogadores].telefone);

            total_jogadores++;
        }
    }

    return total_jogadores;
}

// CADASTRAR PARTIDA
int Cadastrar_Partida(
    Partida partidas[],
    int quantidade,
    Equipe equipes[],
    int qntd_equipes
) {
    int equipe1_encontrada = 0;
    int equipe2_encontrada = 0;

    if (quantidade >= 100 || qntd_equipes < 2) {
        return 0;
    }

    partidas[quantidade].id_partida = quantidade + 1;

    printf("\n--- Cadastro de partida ---\n");

    printf("Data (AAAA-MM-DD): ");
    scanf("%10s", partidas[quantidade].data_partida);

    printf("Horario (HH:MM): ");
    scanf("%8s", partidas[quantidade].horario);

    printf("Tempo de duracao: ");
    scanf(" %20[^\n]", partidas[quantidade].tempo_duracao);

    printf("\nEquipes cadastradas:\n");

    for (int i = 0; i < qntd_equipes; i++) {
        printf("ID: %d | Equipe: %s\n",
               equipes[i].id_equipe,
               equipes[i].nome_equipe);
    }

    printf("ID da primeira equipe: ");
    scanf("%d", &partidas[quantidade].equipe_1);

    printf("ID da segunda equipe: ");
    scanf("%d", &partidas[quantidade].equipe_2);

    for (int i = 0; i < qntd_equipes; i++) {

        if (equipes[i].id_equipe ==
            partidas[quantidade].equipe_1) {
            equipe1_encontrada = 1;
        }

        if (equipes[i].id_equipe ==
            partidas[quantidade].equipe_2) {
            equipe2_encontrada = 1;
        }
    }

    if (!equipe1_encontrada ||
        !equipe2_encontrada ||
        partidas[quantidade].equipe_1 ==
        partidas[quantidade].equipe_2) {

        printf("Equipes invalidas para a partida.\n");
        return 0;
    }

    return 1;
}

// CADASTRAR RESULTADO
int Cadastrar_Resultado(
    ResultadoPartida resultados[],
    int quantidade_resultados,
    Partida partidas[],
    int qntd_partidas,
    Equipe equipes[],
    int qntd_equipes,
    Classificacao classificacao[],
    int qntd_classificacao,
    int matriz_resultados[100][100]
) {
    int id_partida;
    int indice_partida = -1;
    int indice_equipe1 = -1;
    int indice_equipe2 = -1;
    int indice_classificacao1 = -1;
    int indice_classificacao2 = -1;

    if (quantidade_resultados >= 100) {
        return 0;
    }

    printf("\n--- Cadastro de resultado ---\n");

    printf("ID da partida: ");
    scanf("%d", &id_partida);

    for (int i = 0; i < qntd_partidas; i++) {
        if (partidas[i].id_partida == id_partida) {
            indice_partida = i;
            break;
        }
    }

    if (indice_partida == -1) {
        printf("Partida nao encontrada.\n");
        return 0;
    }

    for (int i = 0; i < quantidade_resultados; i++) {
        if (resultados[i].id_partida == id_partida) {
            printf("Essa partida ja possui resultado.\n");
            return 0;
        }
    }

    int id1 = partidas[indice_partida].equipe_1;
    int id2 = partidas[indice_partida].equipe_2;

    for (int i = 0; i < qntd_equipes; i++) {

        if (equipes[i].id_equipe == id1) {
            indice_equipe1 = i;
        }

        if (equipes[i].id_equipe == id2) {
            indice_equipe2 = i;
        }
    }

    for (int i = 0; i < qntd_classificacao; i++) {

        if (classificacao[i].id_equipe == id1) {
            indice_classificacao1 = i;
        }

        if (classificacao[i].id_equipe == id2) {
            indice_classificacao2 = i;
        }
    }

    if (indice_equipe1 == -1 ||
        indice_equipe2 == -1 ||
        indice_classificacao1 == -1 ||
        indice_classificacao2 == -1) {
        return 0;
    }

    int pontos1;
    int pontos2;

    printf("Pontos da equipe %s: ",
           equipes[indice_equipe1].nome_equipe);
    scanf("%d", &pontos1);

    printf("Pontos da equipe %s: ",
           equipes[indice_equipe2].nome_equipe);
    scanf("%d", &pontos2);

    if (pontos1 < 0 || pontos2 < 0) {
        printf("Pontuacao invalida.\n");
        return 0;
    }

    /* Guarda os pontos feitos por cada equipe contra a adversaria.
       Os indices da matriz correspondem as posicoes no vetor equipes. */
       
    matriz_resultados[indice_equipe1][indice_partida] = pontos1;

    matriz_resultados[indice_equipe2][indice_partida] = pontos2;

    resultados[quantidade_resultados].id_resultado =
        quantidade_resultados + 1;

    resultados[quantidade_resultados].id_partida = id_partida;
    resultados[quantidade_resultados].pont_eq1 = pontos1;
    resultados[quantidade_resultados].pont_eq2 = pontos2;



    classificacao[indice_classificacao1].pontuacao += pontos1;

    classificacao[indice_classificacao2].pontuacao += pontos2;

    strcpy(resultados[quantidade_resultados].equipe_venc, "N/A");

    return 1;
}

// SELECTION SORT DA CLASSIFICACAO
void Ordenar_Classificacao(
    Classificacao classificacao[],
    int quantidade
) {
    Classificacao auxiliar;
    int maior;

    for (int i = 0; i < quantidade - 1; i++) {

        maior = i;

        for (int j = i + 1; j < quantidade; j++) {

            if (classificacao[j].pontuacao >
                classificacao[maior].pontuacao) {
                maior = j;
            }
        }

        auxiliar = classificacao[i];
        classificacao[i] = classificacao[maior];
        classificacao[maior] = auxiliar;
    }

    for (int i = 0; i < quantidade; i++) {
        classificacao[i].posicao = i + 1;
    }
}

// EXIBIR MATRIZ DE RESULTADOS
void Exibir_Matriz_Resultados(
    int matriz_resultados[100][100],
    Equipe equipes[],
    int qntd_equipes,
    int qntd_partidas
) {

    printf("\n========== MATRIZ DE RESULTADOS ==========\n\n");

    printf("%-20s", "EQUIPES");

    for (int j = 0; j < qntd_partidas; j++) {
        printf("%-15s", "Partida");
        printf("%-1d", j + 1);
    }

    printf("%-15s\n", "TOTAL");

    for (int i = 0; i < qntd_equipes; i++) {

        int total = 0;

        printf("%-20.20s", equipes[i].nome_equipe);

        for (int j = 0; j < qntd_partidas; j++) {

            printf("%-15d", matriz_resultados[i][j]);

            total += matriz_resultados[i][j];
        }

        printf("%-15d\n", total);
    }
}