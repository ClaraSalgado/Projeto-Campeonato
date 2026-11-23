#ifndef FUNCOES_H
#define FUNCOES_H

#include <mysql.h>
#include "models.h"

int Cadastrar_Campeonato(
    Campeonato campeonatos[],
    int quantidade,
    Campeonato *novo_campeonato
);

int Cadastrar_Equipe(
    Equipe equipes[],
    int quantidade,
    Equipe nova_equipe,
    int id_campeonato
);

int Buscar_Equipe_Pelo_Nome(
    Equipe equipes[],
    int quantidade,
    char nome[]
);

int Buscar_Equipe_Pelo_ID(
    Equipe equipes[],
    int quantidade,
    int id
);

int Cadastrar_Jogadores(
    MYSQL *conexao,
    Jogador jogadores[],
    Equipe equipes[],
    int qntd_equipes
);

int Cadastrar_Partida(
    Partida partidas[],
    int quantidade,
    Equipe equipes[],
    int qntd_equipes
);

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
);

void Ordenar_Classificacao(
    Classificacao classificacao[],
    int quantidade
);

void Exibir_Matriz_Resultados(
    int matriz_resultados[100][100],
    Equipe equipes[],
    int qntd_equipes,
    int qntd_partidas
);

#endif