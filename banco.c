
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "banco.h"

MYSQL *Conectar_Banco(void) {

    MYSQL *conexao = mysql_init(NULL);

    if (conexao == NULL) {
        printf("Erro ao inicializar MySQL.\n");
        return NULL;
    }

    if (mysql_real_connect(
        conexao,
        "localhost",
        "root",
        "",
        "esports",
        3306,
        NULL,
        0
    ) == NULL) {

        printf("Erro na conexão: %s\n", mysql_error(conexao));

        mysql_close(conexao);
        return NULL;
    }

    printf("Conectado ao banco esports!\n");

    return conexao;
}

void Desconectar_Banco(MYSQL *conexao) {

    if (conexao != NULL) {
        mysql_close(conexao);
    }
}

int Salvar_Campeonato_Banco(MYSQL *conexao, Campeonato *campeonato) {

    MYSQL_STMT *stmt = mysql_stmt_init(conexao);

    if (stmt == NULL) {
        printf("Erro ao iniciar comando SQL.\n");
        return 0;
    }

    const char *sql =
        "INSERT INTO campeonato "
        "(nome, jogo, data_inicio, tempo_duracao, num_equipes) "
        "VALUES (?, ?, ?, ?, ?)";

    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        printf("Erro ao preparar SQL: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    MYSQL_BIND parametros[5];
    memset(parametros, 0, sizeof(parametros));

    unsigned long tamanhos[4] = {
        strlen(campeonato->nome),
        strlen(campeonato->jogo),
        strlen(campeonato->data_inicio),
        strlen(campeonato->tempo_duracao)
    };

    parametros[0].buffer_type = MYSQL_TYPE_STRING;
    parametros[0].buffer = campeonato->nome;
    parametros[0].buffer_length = sizeof(campeonato->nome);
    parametros[0].length = &tamanhos[0];

    parametros[1].buffer_type = MYSQL_TYPE_STRING;
    parametros[1].buffer = campeonato->jogo;
    parametros[1].buffer_length = sizeof(campeonato->jogo);
    parametros[1].length = &tamanhos[1];

    parametros[2].buffer_type = MYSQL_TYPE_STRING;
    parametros[2].buffer = campeonato->data_inicio;
    parametros[2].buffer_length = sizeof(campeonato->data_inicio);
    parametros[2].length = &tamanhos[2];

    parametros[3].buffer_type = MYSQL_TYPE_STRING;
    parametros[3].buffer = campeonato->tempo_duracao;
    parametros[3].buffer_length = sizeof(campeonato->tempo_duracao);
    parametros[3].length = &tamanhos[3];

    parametros[4].buffer_type = MYSQL_TYPE_LONG;
    parametros[4].buffer = &campeonato->num_equipes;

    if (mysql_stmt_bind_param(stmt, parametros)) {
        printf("Erro ao associar dados: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_execute(stmt)) {
        printf("Erro ao salvar campeonato: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    campeonato->id_campeonato = (int) mysql_stmt_insert_id(stmt);

    mysql_stmt_close(stmt);

    printf("Campeonato salvo no MySQL!\n");

    return 1;
}


int Salvar_Equipe_Banco(MYSQL *conexao, Equipe *equipe) {

    MYSQL_STMT *stmt = mysql_stmt_init(conexao);

    if (stmt == NULL) {
        printf("Erro ao iniciar comando SQL.\n");
        return 0;
    }

    const char *sql =
        "INSERT INTO equipes "
        "(nome_equipe, qntd_membros, ID_campeonato) "
        "VALUES (?, ?, ?)";

    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        printf("Erro ao preparar SQL: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    MYSQL_BIND parametros[3];
    memset(parametros, 0, sizeof(parametros));

    unsigned long tamanho_nome = strlen(equipe->nome_equipe);

    parametros[0].buffer_type = MYSQL_TYPE_STRING;
    parametros[0].buffer = equipe->nome_equipe;
    parametros[0].buffer_length = sizeof(equipe->nome_equipe);
    parametros[0].length = &tamanho_nome;

    parametros[1].buffer_type = MYSQL_TYPE_LONG;
    parametros[1].buffer = &equipe->qntd_membros;

    parametros[2].buffer_type = MYSQL_TYPE_LONG;
    parametros[2].buffer = &equipe->id_campeonato;

    if (mysql_stmt_bind_param(stmt, parametros)) {
        printf("Erro ao associar dados: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_execute(stmt)) {
        printf("Erro ao salvar equipe: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    equipe->id_equipe = (int) mysql_stmt_insert_id(stmt);

    mysql_stmt_close(stmt);

    printf("Equipe salva no MySQL! ID: %d\n", equipe->id_equipe);

    return 1;
}


int Salvar_Jogador_Banco(MYSQL *conexao, Jogador *jogador) {

    MYSQL_STMT *stmt = mysql_stmt_init(conexao);

    if (stmt == NULL) {
        printf("Erro ao iniciar comando SQL.\n");
        return 0;
    }

    const char *sql =
        "INSERT INTO jogadores "
        "(nome_jogador, idade, email, telefone, ID_equipe) "
        "VALUES (?, ?, ?, ?, ?)";

    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        printf("Erro ao preparar SQL: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    MYSQL_BIND parametros[5];
    memset(parametros, 0, sizeof(parametros));

    unsigned long tamanho_nome = strlen(jogador->nome_jogador);
    unsigned long tamanho_email = strlen(jogador->email);
    unsigned long tamanho_telefone = strlen(jogador->telefone);

    parametros[0].buffer_type = MYSQL_TYPE_STRING;
    parametros[0].buffer = jogador->nome_jogador;
    parametros[0].buffer_length = sizeof(jogador->nome_jogador);
    parametros[0].length = &tamanho_nome;

    parametros[1].buffer_type = MYSQL_TYPE_LONG;
    parametros[1].buffer = &jogador->idade;

    parametros[2].buffer_type = MYSQL_TYPE_STRING;
    parametros[2].buffer = jogador->email;
    parametros[2].buffer_length = sizeof(jogador->email);
    parametros[2].length = &tamanho_email;

    parametros[3].buffer_type = MYSQL_TYPE_STRING;
    parametros[3].buffer = jogador->telefone;
    parametros[3].buffer_length = sizeof(jogador->telefone);
    parametros[3].length = &tamanho_telefone;

    parametros[4].buffer_type = MYSQL_TYPE_LONG;
    parametros[4].buffer = &jogador->id_equipe;

    if (mysql_stmt_bind_param(stmt, parametros)) {
        printf("Erro ao associar dados: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_execute(stmt)) {
        printf("Erro ao salvar jogador: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    jogador->id_jogador = (int) mysql_stmt_insert_id(stmt);

    mysql_stmt_close(stmt);

    printf("Jogador salvo no MySQL! ID: %d\n",
           jogador->id_jogador);

    return 1;
}



int Salvar_Partida_Banco(MYSQL *conexao, Partida *partida) {

    MYSQL_STMT *stmt = mysql_stmt_init(conexao);

    if (stmt == NULL) {
        printf("Erro ao iniciar comando SQL.\n");
        return 0;
    }

    const char *sql =
        "INSERT INTO partidas "
        "(data_partida, horario, tempo_duracao, equipe_1, equipe_2) "
        "VALUES (?, ?, ?, ?, ?)";

    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        printf("Erro ao preparar SQL: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    MYSQL_BIND parametros[5];
    memset(parametros, 0, sizeof(parametros));

    unsigned long tamanho_data = strlen(partida->data_partida);
    unsigned long tamanho_horario = strlen(partida->horario);
    unsigned long tamanho_duracao = strlen(partida->tempo_duracao);

    parametros[0].buffer_type = MYSQL_TYPE_STRING;
    parametros[0].buffer = partida->data_partida;
    parametros[0].buffer_length = sizeof(partida->data_partida);
    parametros[0].length = &tamanho_data;

    parametros[1].buffer_type = MYSQL_TYPE_STRING;
    parametros[1].buffer = partida->horario;
    parametros[1].buffer_length = sizeof(partida->horario);
    parametros[1].length = &tamanho_horario;

    parametros[2].buffer_type = MYSQL_TYPE_STRING;
    parametros[2].buffer = partida->tempo_duracao;
    parametros[2].buffer_length = sizeof(partida->tempo_duracao);
    parametros[2].length = &tamanho_duracao;

    parametros[3].buffer_type = MYSQL_TYPE_LONG;
    parametros[3].buffer = &partida->equipe_1;

    parametros[4].buffer_type = MYSQL_TYPE_LONG;
    parametros[4].buffer = &partida->equipe_2;

    if (mysql_stmt_bind_param(stmt, parametros)) {
        printf("Erro ao associar dados: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_execute(stmt)) {
        printf("Erro ao salvar partida: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    partida->id_partida = (int) mysql_stmt_insert_id(stmt);

    mysql_stmt_close(stmt);

    printf("Partida salva no MySQL! ID: %d\n", partida->id_partida);

    return 1;
}


int Salvar_Resultado_Banco(
    MYSQL *conexao,
    ResultadoPartida *resultado
) {

    MYSQL_STMT *stmt = mysql_stmt_init(conexao);

    if (stmt == NULL) {
        printf("Erro ao iniciar comando SQL.\n");
        return 0;
    }

    const char *sql =
        "INSERT INTO resultado_partida "
        "(equipe_venc, pont_eq1, pont_eq2, ID_partida) "
        "VALUES (?, ?, ?, ?)";

    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        printf("Erro ao preparar SQL: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    MYSQL_BIND parametros[4];
    memset(parametros, 0, sizeof(parametros));

    unsigned long tamanho_vencedor =
        strlen(resultado->equipe_venc);

    parametros[0].buffer_type = MYSQL_TYPE_STRING;
    parametros[0].buffer = resultado->equipe_venc;
    parametros[0].buffer_length = sizeof(resultado->equipe_venc);
    parametros[0].length = &tamanho_vencedor;

    parametros[1].buffer_type = MYSQL_TYPE_LONG;
    parametros[1].buffer = &resultado->pont_eq1;

    parametros[2].buffer_type = MYSQL_TYPE_LONG;
    parametros[2].buffer = &resultado->pont_eq2;

    parametros[3].buffer_type = MYSQL_TYPE_LONG;
    parametros[3].buffer = &resultado->id_partida;

    if (mysql_stmt_bind_param(stmt, parametros)) {
        printf("Erro ao associar dados: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_execute(stmt)) {
        printf("Erro ao salvar resultado: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    resultado->id_resultado =
        (int) mysql_stmt_insert_id(stmt);

    mysql_stmt_close(stmt);

    printf("Resultado salvo no MySQL! ID: %d\n",
           resultado->id_resultado);

    return 1;
}

int Salvar_Classificacao_Banco(
    MYSQL *conexao,
    Classificacao *classificacao
) {

    MYSQL_STMT *stmt = mysql_stmt_init(conexao);

    if (stmt == NULL) {
        printf("Erro ao iniciar comando SQL.\n");
        return 0;
    }

    // Primeiro verificamos se a equipe já possui classificação.

    const char *sql_busca =
        "SELECT ID_classificacao FROM classificacao "
        "WHERE ID_equipe = ? LIMIT 1";

    if (mysql_stmt_prepare(stmt, sql_busca, strlen(sql_busca))) {
        printf("Erro ao preparar consulta: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    MYSQL_BIND parametro_busca;
    memset(&parametro_busca, 0, sizeof(parametro_busca));

    parametro_busca.buffer_type = MYSQL_TYPE_LONG;
    parametro_busca.buffer = &classificacao->id_equipe;

    if (mysql_stmt_bind_param(stmt, &parametro_busca) ||
        mysql_stmt_execute(stmt)) {

        printf("Erro ao consultar classificacao: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    int id_encontrado;
    bool existe = 0;

    MYSQL_BIND resultado_busca;
    memset(&resultado_busca, 0, sizeof(resultado_busca));

    resultado_busca.buffer_type = MYSQL_TYPE_LONG;
    resultado_busca.buffer = &id_encontrado;
    resultado_busca.is_null = &existe;

    if (mysql_stmt_bind_result(stmt, &resultado_busca) ||
        mysql_stmt_store_result(stmt)) {

        printf("Erro ao obter classificacao: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    int encontrou = mysql_stmt_fetch(stmt) == 0;

    mysql_stmt_free_result(stmt);
    mysql_stmt_close(stmt);

    // Se encontrou, atualiza. Caso contrario, insere.

    if (encontrou) {

        const char *sql_update =
            "UPDATE classificacao SET pontuacao = ?, posicao = ? "
            "WHERE ID_equipe = ?";

        stmt = mysql_stmt_init(conexao);

        if (stmt == NULL) {
            return 0;
        }

        if (mysql_stmt_prepare(stmt, sql_update, strlen(sql_update))) {
            printf("Erro ao preparar UPDATE: %s\n",
                   mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return 0;
        }

        MYSQL_BIND parametros[3];
        memset(parametros, 0, sizeof(parametros));

        parametros[0].buffer_type = MYSQL_TYPE_LONG;
        parametros[0].buffer = &classificacao->pontuacao;

        parametros[1].buffer_type = MYSQL_TYPE_LONG;
        parametros[1].buffer = &classificacao->posicao;

        parametros[2].buffer_type = MYSQL_TYPE_LONG;
        parametros[2].buffer = &classificacao->id_equipe;

        if (mysql_stmt_bind_param(stmt, parametros) ||
            mysql_stmt_execute(stmt)) {

            printf("Erro ao atualizar classificacao: %s\n",
                   mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return 0;
        }

        mysql_stmt_close(stmt);

        printf("Classificacao da equipe %d atualizada!\n",
               classificacao->id_equipe);

    } else {

        const char *sql_insert =
            "INSERT INTO classificacao "
            "(pontuacao, posicao, ID_equipe) VALUES (?, ?, ?)";

        stmt = mysql_stmt_init(conexao);

        if (stmt == NULL) {
            return 0;
        }

        if (mysql_stmt_prepare(stmt, sql_insert, strlen(sql_insert))) {
            printf("Erro ao preparar INSERT: %s\n",
                   mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return 0;
        }

        MYSQL_BIND parametros[3];
        memset(parametros, 0, sizeof(parametros));

        parametros[0].buffer_type = MYSQL_TYPE_LONG;
        parametros[0].buffer = &classificacao->pontuacao;

        parametros[1].buffer_type = MYSQL_TYPE_LONG;
        parametros[1].buffer = &classificacao->posicao;

        parametros[2].buffer_type = MYSQL_TYPE_LONG;
        parametros[2].buffer = &classificacao->id_equipe;

        if (mysql_stmt_bind_param(stmt, parametros) ||
            mysql_stmt_execute(stmt)) {

            printf("Erro ao inserir classificacao: %s\n",
                   mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return 0;
        }

        classificacao->id_classificacao =
            (int) mysql_stmt_insert_id(stmt);

        mysql_stmt_close(stmt);

        printf("Nova classificacao cadastrada!\n");
    }

    return 1;
}

/*int Salvar_Classificacao_Banco(
    MYSQL *conexao,
    Classificacao *classificacao
) {

    MYSQL_STMT *stmt = mysql_stmt_init(conexao);

    if (stmt == NULL) {
        printf("Erro ao iniciar comando SQL.\n");
        return 0;
    }

    const char *sql =
        "INSERT INTO classificacao "
        "(pontuacao, posicao, ID_equipe) "
        "VALUES (?, ?, ?)";

    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        printf("Erro ao preparar SQL: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    MYSQL_BIND parametros[3];
    memset(parametros, 0, sizeof(parametros));

    parametros[0].buffer_type = MYSQL_TYPE_LONG;
    parametros[0].buffer = &classificacao->pontuacao;

    parametros[1].buffer_type = MYSQL_TYPE_LONG;
    parametros[1].buffer = &classificacao->posicao;

    parametros[2].buffer_type = MYSQL_TYPE_LONG;
    parametros[2].buffer = &classificacao->id_equipe;

    if (mysql_stmt_bind_param(stmt, parametros)) {
        printf("Erro ao associar dados: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_execute(stmt)) {
        printf("Erro ao salvar classificação: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    classificacao->id_classificacao =
        (int) mysql_stmt_insert_id(stmt);

    mysql_stmt_close(stmt);

    return 1;
}*/



void Consultar_Campeonatos_Banco(MYSQL *conexao) {

    const char *sql =
        "SELECT ID_campeonato, nome, jogo, data_inicio, "
        "tempo_duracao, num_equipes FROM campeonato";

    if (mysql_query(conexao, sql)) {
        printf("Erro ao consultar campeonatos: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_RES *resultado = mysql_store_result(conexao);

    if (resultado == NULL) {
        printf("Erro ao obter resultados: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_ROW linha;

    printf("\n========== CAMPEONATOS CADASTRADOS ==========\n");

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        printf("\nID: %s\n", linha[0]);
        printf("Nome: %s\n", linha[1]);
        printf("Jogo: %s\n", linha[2]);
        printf("Data de inicio: %s\n", linha[3]);
        printf("Duracao: %s\n", linha[4]);
        printf("Quantidade de equipes: %s\n", linha[5]);
    }

    mysql_free_result(resultado);
}

void Consultar_Equipes_Banco(MYSQL *conexao) {

    const char *sql =
        "SELECT ID_equipe, nome_equipe, qntd_membros, ID_campeonato "
        "FROM equipes";

    if (mysql_query(conexao, sql)) {
        printf("Erro ao consultar equipes: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_RES *resultado = mysql_store_result(conexao);

    if (resultado == NULL) {
        printf("Erro ao obter resultados: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_ROW linha;

    printf("\n========== EQUIPES CADASTRADAS ==========\n");

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        printf("\nID da equipe: %s\n", linha[0]);
        printf("Nome: %s\n", linha[1]);
        printf("Quantidade de membros: %s\n", linha[2]);
        printf("ID do campeonato: %s\n", linha[3]);
    }

    mysql_free_result(resultado);
}

void Consultar_Jogadores_Banco(MYSQL *conexao) {

    const char *sql =
        "SELECT ID_jogador, nome_jogador, idade, email, telefone, ID_equipe "
        "FROM jogadores";

    if (mysql_query(conexao, sql)) {
        printf("Erro ao consultar jogadores: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_RES *resultado = mysql_store_result(conexao);

    if (resultado == NULL) {
        printf("Erro ao obter resultados: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_ROW linha;

    printf("\n========== JOGADORES CADASTRADOS ==========\n");

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        printf("\nID do jogador: %s\n", linha[0]);
        printf("Nome: %s\n", linha[1]);
        printf("Idade: %s\n", linha[2]);
        printf("Email: %s\n", linha[3]);
        printf("Telefone: %s\n", linha[4]);
        printf("ID da equipe: %s\n", linha[5]);
    }

    mysql_free_result(resultado);
}

void Consultar_Partidas_Banco(MYSQL *conexao) {

    const char *sql =
        "SELECT ID_partida, data_partida, horario, "
        "tempo_duracao, equipe_1, equipe_2 FROM partidas";

    if (mysql_query(conexao, sql)) {
        printf("Erro ao consultar partidas: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_RES *resultado = mysql_store_result(conexao);

    if (resultado == NULL) {
        printf("Erro ao obter resultados: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_ROW linha;

    printf("\n========== PARTIDAS CADASTRADAS ==========\n");

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        printf("\nID da partida: %s\n", linha[0]);
        printf("Data: %s\n", linha[1]);
        printf("Horario: %s\n", linha[2] ? linha[2] : "Nao informado");
        printf("Duracao: %s\n", linha[3] ? linha[3] : "Nao informada");
        printf("ID da equipe 1: %s\n", linha[4]);
        printf("ID da equipe 2: %s\n", linha[5]);
    }

    mysql_free_result(resultado);
}

void Consultar_Resultados_Banco(MYSQL *conexao) {

    const char *sql =
        "SELECT ID_resultado, equipe_venc, pont_eq1, "
        "pont_eq2, ID_partida FROM resultado_partida";

    if (mysql_query(conexao, sql)) {
        printf("Erro ao consultar resultados: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_RES *resultado = mysql_store_result(conexao);

    if (resultado == NULL) {
        printf("Erro ao obter resultados: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_ROW linha;

    printf("\n========== RESULTADOS DAS PARTIDAS ==========\n");

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        printf("\nID do resultado: %s\n", linha[0]);
        printf("Equipe vencedora: %s\n", linha[1]);
        printf("Pontuacao da equipe 1: %s\n", linha[2]);
        printf("Pontuacao da equipe 2: %s\n", linha[3]);
        printf("ID da partida: %s\n", linha[4]);
    }

    mysql_free_result(resultado);
}


void Consultar_Classificacao_Banco(MYSQL *conexao, int id_campeonato) {

    char sql[512];

    snprintf(sql, sizeof(sql),
        "SELECT c.ID_classificacao, c.pontuacao, "
        "c.posicao, c.ID_equipe, e.nome_equipe "
        "FROM classificacao c "
        "INNER JOIN equipes e ON c.ID_equipe = e.ID_equipe "
        "WHERE e.ID_campeonato = %d "
        "ORDER BY c.posicao ASC",
        id_campeonato
    );

    if (mysql_query(conexao, sql)) {
        printf("Erro ao consultar classificacao: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_RES *resultado = mysql_store_result(conexao);

    if (resultado == NULL) {
        printf("Erro ao obter resultados: %s\n",
               mysql_error(conexao));
        return;
    }

    MYSQL_ROW linha;

    printf("\n========== CLASSIFICACAO DO CAMPEONATO ==========\n");

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        printf("\nID da classificacao: %s\n", linha[0]);
        printf("Pontuacao: %s\n", linha[1]);
        printf("Posicao: %s\n", linha[2]);
        printf("ID da equipe: %s\n", linha[3]);
        printf("Nome da equipe: %s\n", linha[4]);
    }

    mysql_free_result(resultado);
}



int Atualizar_Nome_Equipe_Banco(MYSQL *conexao) {

    int id_equipe;
    char novo_nome[51];

    printf("\n========== ATUALIZAR EQUIPE ==========\n");

    printf("Digite o ID da equipe que deseja alterar: ");
    scanf("%d", &id_equipe);

    printf("Digite o novo nome da equipe: ");
    scanf(" %50[^\n]", novo_nome);

    MYSQL_STMT *stmt = mysql_stmt_init(conexao);

    if (stmt == NULL) {
        printf("Erro ao iniciar comando SQL.\n");
        return 0;
    }

    const char *sql =
        "UPDATE equipes SET nome_equipe = ? WHERE ID_equipe = ?";

    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        printf("Erro ao preparar SQL: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    MYSQL_BIND parametros[2];
    memset(parametros, 0, sizeof(parametros));

    unsigned long tamanho_nome = strlen(novo_nome);

    parametros[0].buffer_type = MYSQL_TYPE_STRING;
    parametros[0].buffer = novo_nome;
    parametros[0].buffer_length = sizeof(novo_nome);
    parametros[0].length = &tamanho_nome;

    parametros[1].buffer_type = MYSQL_TYPE_LONG;
    parametros[1].buffer = &id_equipe;

    if (mysql_stmt_bind_param(stmt, parametros)) {
        printf("Erro ao associar dados: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_execute(stmt)) {
        printf("Erro ao atualizar equipe: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_affected_rows(stmt) == 0) {
        printf("Nenhuma equipe foi alterada. Verifique o ID.\n");
    } else {
        printf("Nome da equipe atualizado com sucesso!\n");
    }

    mysql_stmt_close(stmt);

    return 1;
}

int Excluir_Jogador_Banco(MYSQL *conexao) {

    int id_jogador;

    printf("\n========== EXCLUIR JOGADOR ==========\n");
    printf("Digite o ID do jogador que deseja excluir: ");
    scanf("%d", &id_jogador);

    MYSQL_STMT *stmt = mysql_stmt_init(conexao);

    if (stmt == NULL) {
        printf("Erro ao iniciar comando SQL.\n");
        return 0;
    }

    const char *sql =
        "DELETE FROM jogadores WHERE ID_jogador = ?";

    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        printf("Erro ao preparar SQL: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    MYSQL_BIND parametro;
    memset(&parametro, 0, sizeof(parametro));

    parametro.buffer_type = MYSQL_TYPE_LONG;
    parametro.buffer = &id_jogador;

    if (mysql_stmt_bind_param(stmt, &parametro)) {
        printf("Erro ao associar parametro: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_execute(stmt)) {
        printf("Erro ao excluir jogador: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return 0;
    }

    if (mysql_stmt_affected_rows(stmt) > 0) {
        printf("Jogador excluido com sucesso!\n");
    } else {
        printf("Nenhum jogador encontrado com esse ID.\n");
    }

    mysql_stmt_close(stmt);

    return 1;
}


int Atualizar_Classificacao_Banco(MYSQL *conexao, int id_campeonato) {

    // Inicia uma transacao
    if (mysql_query(conexao, "START TRANSACTION")) {
        printf("Erro ao iniciar transacao: %s\n",
               mysql_error(conexao));
        return 0;
    }

    // Remove somente a classificacao das equipes deste campeonato
    const char *sql_delete =
        "DELETE c FROM classificacao c "
        "INNER JOIN equipes e ON c.ID_equipe = e.ID_equipe "
        "WHERE e.ID_campeonato = ?";

    MYSQL_STMT *stmt = mysql_stmt_init(conexao);

    if (stmt == NULL) {
        mysql_query(conexao, "ROLLBACK");
        return 0;
    }

    if (mysql_stmt_prepare(stmt, sql_delete, strlen(sql_delete))) {
        printf("Erro ao preparar exclusao: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_query(conexao, "ROLLBACK");
        return 0;
    }

    MYSQL_BIND parametro;
    memset(&parametro, 0, sizeof(parametro));

    parametro.buffer_type = MYSQL_TYPE_LONG;
    parametro.buffer = &id_campeonato;

    if (mysql_stmt_bind_param(stmt, &parametro) ||
        mysql_stmt_execute(stmt)) {

        printf("Erro ao limpar classificacao: %s\n",
               mysql_stmt_error(stmt));

        mysql_stmt_close(stmt);
        mysql_query(conexao, "ROLLBACK");
        return 0;
    }

    mysql_stmt_close(stmt);

    // Recalcula os pontos somente das equipes deste campeonato
    const char *sql_insert =
        "INSERT INTO classificacao (pontuacao, posicao, ID_equipe) "
        "SELECT totais.pontuacao, "
        "ROW_NUMBER() OVER (ORDER BY totais.pontuacao DESC, totais.ID_equipe ASC), "
        "totais.ID_equipe "
        "FROM ( "
        "SELECT e.ID_equipe, COALESCE(SUM(r.pontos), 0) AS pontuacao "
        "FROM equipes e "
        "LEFT JOIN ( "
        "SELECT p.equipe_1 AS ID_equipe, rp.pont_eq1 AS pontos "
        "FROM partidas p "
        "INNER JOIN resultado_partida rp "
        "ON p.ID_partida = rp.ID_partida "
        "UNION ALL "
        "SELECT p.equipe_2 AS ID_equipe, rp.pont_eq2 AS pontos "
        "FROM partidas p "
        "INNER JOIN resultado_partida rp "
        "ON p.ID_partida = rp.ID_partida "
        ") r ON e.ID_equipe = r.ID_equipe "
        "WHERE e.ID_campeonato = ? "
        "GROUP BY e.ID_equipe "
        ") totais";

    stmt = mysql_stmt_init(conexao);

    if (stmt == NULL) {
        mysql_query(conexao, "ROLLBACK");
        return 0;
    }

    if (mysql_stmt_prepare(stmt, sql_insert, strlen(sql_insert))) {
        printf("Erro ao preparar recalculo: %s\n",
               mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_query(conexao, "ROLLBACK");
        return 0;
    }

    memset(&parametro, 0, sizeof(parametro));

    parametro.buffer_type = MYSQL_TYPE_LONG;
    parametro.buffer = &id_campeonato;

    if (mysql_stmt_bind_param(stmt, &parametro) ||
        mysql_stmt_execute(stmt)) {

        printf("Erro ao recalcular classificacao: %s\n",
               mysql_stmt_error(stmt));

        mysql_stmt_close(stmt);
        mysql_query(conexao, "ROLLBACK");
        return 0;
    }

    mysql_stmt_close(stmt);

    // Confirma as alteracoes
    if (mysql_query(conexao, "COMMIT")) {
        printf("Erro ao confirmar classificacao: %s\n",
               mysql_error(conexao));
        mysql_query(conexao, "ROLLBACK");
        return 0;
    }

    printf("\nClassificacao atualizada com sucesso!\n");

    return 1;
}


int Carregar_Equipes_Banco(
    MYSQL *conexao,
    Equipe equipes[],
    int id_campeonato
) {
    char sql[300];

    snprintf(sql, sizeof(sql),
        "SELECT ID_equipe, nome_equipe, qntd_membros, ID_campeonato "
        "FROM equipes "
        "WHERE ID_campeonato = %d "
        "ORDER BY ID_equipe ASC",
        id_campeonato
    );

    if (mysql_query(conexao, sql)) {
        printf("Erro ao carregar equipes: %s\n",
               mysql_error(conexao));
        return -1;
    }

    MYSQL_RES *resultado = mysql_store_result(conexao);

    if (resultado == NULL) {
        printf("Erro ao obter equipes: %s\n",
               mysql_error(conexao));
        return -1;
    }

    MYSQL_ROW linha;
    int quantidade = 0;

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        if (quantidade >= 100) {
            printf("Limite de equipes atingido.\n");
            break;
        }

        equipes[quantidade].id_equipe = atoi(linha[0]);
        strcpy(equipes[quantidade].nome_equipe, linha[1]);
        equipes[quantidade].qntd_membros = atoi(linha[2]);
        equipes[quantidade].id_campeonato = atoi(linha[3]);

        quantidade++;
    }

    mysql_free_result(resultado);

    printf("\n%d equipes recuperadas do MySQL!\n", quantidade);

    return quantidade;
}

int Carregar_Jogadores_Banco(
    MYSQL *conexao,
    Jogador jogadores[],
    int id_campeonato
) {
    char sql[300];

    snprintf(sql, sizeof(sql),
        "SELECT j.ID_jogador, j.nome_jogador, j.idade, "
        "j.email, j.telefone, j.ID_equipe "
        "FROM jogadores j "
        "INNER JOIN equipes e ON j.ID_equipe = e.ID_equipe "
        "WHERE e.ID_campeonato = %d "
        "ORDER BY j.ID_jogador ASC",
        id_campeonato
    );

    if (mysql_query(conexao, sql)) {
        printf("Erro ao carregar jogadores: %s\n",
               mysql_error(conexao));
        return -1;
    }

    MYSQL_RES *resultado = mysql_store_result(conexao);

    if (resultado == NULL) {
        printf("Erro ao obter jogadores: %s\n",
               mysql_error(conexao));
        return -1;
    }

    MYSQL_ROW linha;
    int quantidade = 0;

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        if (quantidade >= 500) {
            printf("Limite de jogadores atingido.\n");
            break;
        }

        jogadores[quantidade].id_jogador = atoi(linha[0]);
        strcpy(jogadores[quantidade].nome_jogador, linha[1]);
        jogadores[quantidade].idade = atoi(linha[2]);
        strcpy(jogadores[quantidade].email, linha[3]);
        strcpy(jogadores[quantidade].telefone, linha[4]);
        jogadores[quantidade].id_equipe = atoi(linha[5]);

        quantidade++;
    }

    mysql_free_result(resultado);

    printf("\n%d jogadores recuperados do MySQL!\n", quantidade);

    return quantidade;
}


int Carregar_Partidas_Banco(
    MYSQL *conexao,
    Partida partidas[],
    int id_campeonato
) {
    char sql[500];

    snprintf(sql, sizeof(sql),
        "SELECT p.ID_partida, p.data_partida, p.horario, "
        "p.tempo_duracao, p.equipe_1, p.equipe_2 "
        "FROM partidas p "
        "INNER JOIN equipes e1 ON p.equipe_1 = e1.ID_equipe "
        "INNER JOIN equipes e2 ON p.equipe_2 = e2.ID_equipe "
        "WHERE e1.ID_campeonato = %d "
        "AND e2.ID_campeonato = %d "
        "ORDER BY p.ID_partida ASC",
        id_campeonato,
        id_campeonato
    );

    if (mysql_query(conexao, sql)) {
        printf("Erro ao carregar partidas: %s\n",
               mysql_error(conexao));
        return -1;
    }

    MYSQL_RES *resultado = mysql_store_result(conexao);

    if (resultado == NULL) {
        printf("Erro ao obter partidas: %s\n",
               mysql_error(conexao));
        return -1;
    }

    MYSQL_ROW linha;
    int quantidade = 0;

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        if (quantidade >= 100) {
            printf("Limite de partidas atingido.\n");
            break;
        }

        partidas[quantidade].id_partida = atoi(linha[0]);
        strcpy(partidas[quantidade].data_partida, linha[1]);
        strcpy(partidas[quantidade].horario, linha[2]);
        strcpy(partidas[quantidade].tempo_duracao, linha[3]);
        partidas[quantidade].equipe_1 = atoi(linha[4]);
        partidas[quantidade].equipe_2 = atoi(linha[5]);

        quantidade++;
    }

    mysql_free_result(resultado);

    printf("\n%d partidas recuperadas do MySQL!\n", quantidade);

    return quantidade;
}

int Carregar_Resultados_Banco(
    MYSQL *conexao,
    ResultadoPartida resultados[],
    int id_campeonato
) {
    char sql[500];

    snprintf(sql, sizeof(sql),
        "SELECT rp.ID_resultado, rp.equipe_venc, "
        "rp.pont_eq1, rp.pont_eq2, rp.ID_partida "
        "FROM resultado_partida rp "
        "INNER JOIN partidas p ON rp.ID_partida = p.ID_partida "
        "INNER JOIN equipes e1 ON p.equipe_1 = e1.ID_equipe "
        "INNER JOIN equipes e2 ON p.equipe_2 = e2.ID_equipe "
        "WHERE e1.ID_campeonato = %d "
        "AND e2.ID_campeonato = %d "
        "ORDER BY rp.ID_resultado ASC",
        id_campeonato,
        id_campeonato
    );

    if (mysql_query(conexao, sql)) {
        printf("Erro ao carregar resultados: %s\n",
               mysql_error(conexao));
        return -1;
    }

    MYSQL_RES *resultado = mysql_store_result(conexao);

    if (resultado == NULL) {
        printf("Erro ao obter resultados: %s\n",
               mysql_error(conexao));
        return -1;
    }

    MYSQL_ROW linha;
    int quantidade = 0;

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        if (quantidade >= 100) {
            printf("Limite de resultados atingido.\n");
            break;
        }

        resultados[quantidade].id_resultado = atoi(linha[0]);

        snprintf(resultados[quantidade].equipe_venc,
                 sizeof(resultados[quantidade].equipe_venc),
                 "%s", linha[1]);

        resultados[quantidade].pont_eq1 = atoi(linha[2]);
        resultados[quantidade].pont_eq2 = atoi(linha[3]);
        resultados[quantidade].id_partida = atoi(linha[4]);

        quantidade++;
    }

    mysql_free_result(resultado);

    printf("\n%d resultados recuperados do MySQL!\n", quantidade);

    return quantidade;
}


int Carregar_Classificacao_Banco(MYSQL *conexao,
                                 Classificacao classificacao[],
                                 int id_campeonato) {

    char sql[300];

    snprintf(sql, sizeof(sql),
        "SELECT c.ID_classificacao, c.pontuacao, "
        "c.posicao, c.ID_equipe "
        "FROM classificacao c "
        "INNER JOIN equipes e ON c.ID_equipe = e.ID_equipe "
        "WHERE e.ID_campeonato = %d "
        "ORDER BY c.posicao ASC",
        id_campeonato);

    if (mysql_query(conexao, sql)) {
        printf("Erro ao carregar classificacao: %s\n",
               mysql_error(conexao));
        return -1;
    }

    MYSQL_RES *resultado = mysql_store_result(conexao);

    if (resultado == NULL) {
        printf("Erro ao obter classificacao: %s\n",
               mysql_error(conexao));
        return -1;
    }

    MYSQL_ROW linha;
    int quantidade = 0;

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        if (quantidade >= 100) {
            printf("Limite de classificacao atingido.\n");
            break;
        }

        classificacao[quantidade].id_classificacao = atoi(linha[0]);

        classificacao[quantidade].pontuacao = atoi(linha[1]);

        classificacao[quantidade].posicao = atoi(linha[2]);

        classificacao[quantidade].id_equipe = atoi(linha[3]);

        quantidade++;
    }

    mysql_free_result(resultado);

    printf("\n%d classificacoes recuperadas do MySQL!\n", quantidade);

    return quantidade;
}
