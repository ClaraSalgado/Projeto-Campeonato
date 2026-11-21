#ifndef BANCO_H
#define BANCO_H

#include <mysql.h>
#include "models.h"

MYSQL *Conectar_Banco(void);

void Desconectar_Banco(MYSQL *conexao);

int Salvar_Campeonato_Banco(MYSQL *conexao, Campeonato *campeonato);

int Salvar_Equipe_Banco(MYSQL *conexao, Equipe *equipe);

int Salvar_Jogador_Banco(MYSQL *conexao, Jogador *jogador);

int Salvar_Partida_Banco(MYSQL *conexao, Partida *partida);

int Salvar_Resultado_Banco(
    MYSQL *conexao,
    ResultadoPartida *resultado
);

int Salvar_Classificacao_Banco(
    MYSQL *conexao,
    Classificacao *classificacao
);


void Consultar_Campeonatos_Banco(MYSQL *conexao);

void Consultar_Equipes_Banco(MYSQL *conexao);

void Consultar_Jogadores_Banco(MYSQL *conexao);

void Consultar_Partidas_Banco(MYSQL *conexao);

void Consultar_Resultados_Banco(MYSQL *conexao);

void Consultar_Classificacao_Banco(MYSQL *conexao, int id_campeonato);


int Atualizar_Nome_Equipe_Banco(MYSQL *conexao);


int Excluir_Jogador_Banco(MYSQL *conexao);

int Atualizar_Classificacao_Banco(MYSQL *conexao, int id_campeonato);

int Carregar_Equipes_Banco(
    MYSQL *conexao,
    Equipe equipes[],
    int id_campeonato
);

int Carregar_Jogadores_Banco(
    MYSQL *conexao,
    Jogador jogadores[],
    int id_campeonato
);

int Carregar_Partidas_Banco(
    MYSQL *conexao,
    Partida partidas[],
    int id_campeonato
);

int Carregar_Resultados_Banco(
    MYSQL *conexao,
    ResultadoPartida resultados[],
    int id_campeonato
);

int Carregar_Classificacao_Banco(
    MYSQL *conexao,
    Classificacao classificacao[],
    int id_campeonato
);
#endif