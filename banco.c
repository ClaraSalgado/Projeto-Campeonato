
#include <stdio.h>
#include <string.h>
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

void Consultar_Classificacao_Banco(MYSQL *conexao) {

    const char *sql =
        "SELECT ID_classificacao, pontuacao, posicao, ID_equipe "
        "FROM classificacao ORDER BY posicao ASC";

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

    printf("\n========== CLASSIFICACAO ==========\n");

    while ((linha = mysql_fetch_row(resultado)) != NULL) {

        printf("\nID da classificacao: %s\n", linha[0]);
        printf("Pontuacao: %s\n", linha[1]);
        printf("Posicao: %s\n", linha[2]);
        printf("ID da equipe: %s\n", linha[3]);
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

int Atualizar_Classificacao_Banco(MYSQL *conexao) {

    // Inicia uma transacao
    if (mysql_query(conexao, "START TRANSACTION")) {
        printf("Erro ao iniciar transacao: %s\n",
               mysql_error(conexao));
        return 0;
    }

    // Remove a classificacao anterior
    if (mysql_query(conexao, "DELETE FROM classificacao")) {
        printf("Erro ao limpar classificacao: %s\n",
               mysql_error(conexao));

        mysql_query(conexao, "ROLLBACK");
        return 0;
    }

    // Recalcula os pontos de todas as equipes
    const char *sql =
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
        "GROUP BY e.ID_equipe "
        ") totais";

    if (mysql_query(conexao, sql)) {
        printf("Erro ao recalcular classificacao: %s\n",
               mysql_error(conexao));

        mysql_query(conexao, "ROLLBACK");
        return 0;
    }

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