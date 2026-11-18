#ifndef MODELS_H
#define MODELS_H

typedef struct {
    int id_campeonato;
    char nome[101];
    char jogo[51];
    char data_inicio[11];
    char tempo_duracao[21];
    int num_equipes;
} Campeonato;

typedef struct {
    int id_equipe;
    char nome_equipe[51];
    int qntd_membros;
    int id_campeonato;
} Equipe;

typedef struct {
    int id_jogador;
    char nome_jogador[101];
    int idade;
    char email[51];
    char telefone[15];
    int id_equipe;
} Jogador;

typedef struct {
    int id_partida;
    char data_partida[11];
    char horario[9];
    char tempo_duracao[21];
    int equipe_1;
    int equipe_2;
} Partida;

typedef struct {
    int id_classificacao;
    int pontuacao;
    int posicao;
    int id_equipe;
} Classificacao;

typedef struct {
    int id_resultado;
    char equipe_venc[51];
    int pont_eq1;
    int pont_eq2;
    int id_partida;
} ResultadoPartida;

#endif