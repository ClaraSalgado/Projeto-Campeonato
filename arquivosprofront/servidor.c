/* =========================================================
   servidor.c - liga o site ao back em C e ao MySQL

   O navegador não chama C nem MySQL. Este programa fica rodando, recebe os pedidos
   do site (HTTP) e chama as MESMAS funções do banco.c e do funcoes.c.
   Tudo que o site cadastra, altera ou exclui vai para o banco "esports" de verdade.

   Ele também entrega o index.html, o style.css e o script.js, então o site abre em
   http://localhost:8080 (rode o programa dentro da pasta onde estão esses 3 arquivos).

   COMO COMPILAR (junto com banco.c e funcoes.c):
   Mac:
     clang servidor.c banco.c funcoes.c -o servidor $(mysql_config --cflags --libs)
   Windows (MSYS2 UCRT64):
     gcc servidor.c banco.c funcoes.c -o servidor.exe -IC:/msys64/ucrt64/include/mariadb -lmariadb -lws2_32

   O servidor só aceita pedidos do próprio computador (127.0.0.1).
   ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define FECHAR_SOCKET closesocket
#define SOCKET_INVALIDO INVALID_SOCKET
#else
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
typedef int socket_t;
#define FECHAR_SOCKET close
#define SOCKET_INVALIDO (-1)
#endif

#include "banco.h"
#include "funcoes.h"

#define PORTA 8080
#define TAM_REQUISICAO 65536
#define TAM_TEXTO 2048

/* Vetores do campeonato que está sendo consultado (iguais aos do main.c) */
static Equipe g_equipes[100];
static Jogador g_jogadores[500];
static Partida g_partidas[100];
static ResultadoPartida g_resultados[100];
static Classificacao g_classificacao[100];

static char g_requisicao[TAM_REQUISICAO + 1];

/* ---------- 1. TEXTO DINÂMICO (para montar o JSON) ---------- */

typedef struct
{
    char *dados;
    size_t tamanho;
    size_t capacidade;
} Texto;

static void texto_iniciar(Texto *t)
{
    t->capacidade = 1024;
    t->tamanho = 0;
    t->dados = malloc(t->capacidade);
    t->dados[0] = '\0';
}

static void texto_liberar(Texto *t)
{
    free(t->dados);
    t->dados = NULL;
}

static void texto_add(Texto *t, const char *formato, ...)
{
    va_list args;
    va_list copia;
    int necessario;

    va_start(args, formato);
    va_copy(copia, args);
    necessario = vsnprintf(NULL, 0, formato, copia);
    va_end(copia);

    if (necessario > 0)
    {
        while (t->tamanho + (size_t)necessario + 1 > t->capacidade)
        {
            t->capacidade *= 2;
            t->dados = realloc(t->dados, t->capacidade);
        }
        vsnprintf(t->dados + t->tamanho, (size_t)necessario + 1, formato, args);
        t->tamanho += (size_t)necessario;
    }
    va_end(args);
}

/* Escreve um texto entre aspas, protegendo aspas, barras e caracteres de controle */
static void texto_add_string(Texto *t, const char *s)
{
    texto_add(t, "\"");
    for (; s != NULL && *s; s++)
    {
        unsigned char c = (unsigned char)*s;
        if (c == '"')
            texto_add(t, "\\\"");
        else if (c == '\\')
            texto_add(t, "\\\\");
        else if (c == '\n')
            texto_add(t, "\\n");
        else if (c == '\r')
            texto_add(t, "\\r");
        else if (c == '\t')
            texto_add(t, "\\t");
        else if (c < 0x20)
            texto_add(t, "\\u%04x", c);
        else
            texto_add(t, "%c", c);
    }
    texto_add(t, "\"");
}

/* ---------- 2. HTTP: LER PEDIDO E ENVIAR RESPOSTA ---------- */

typedef struct
{
    char metodo[8];
    char caminho[256];
    char query[1024];
    char *corpo;
} Requisicao;

static int comparar_ci(const char *a, const char *b, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        int x = tolower((unsigned char)a[i]);
        int y = tolower((unsigned char)b[i]);
        if (x != y)
            return x - y;
        if (x == '\0')
            return 0;
    }
    return 0;
}

static long tamanho_do_corpo(const char *cabecalhos)
{
    const char *p = cabecalhos;

    while (p != NULL && *p)
    {
        const char *fim = strstr(p, "\r\n");
        size_t tam = fim ? (size_t)(fim - p) : strlen(p);

        if (tam > 15 && comparar_ci(p, "Content-Length:", 15) == 0)
        {
            return atol(p + 15);
        }
        if (fim == NULL)
            break;
        p = fim + 2;
    }
    return 0;
}

/* Lê o pedido inteiro (cabeçalho + corpo). Devolve 1 se deu certo */
static int ler_requisicao(socket_t cliente, Requisicao *req)
{
    size_t total = 0;
    char *fim_cabecalho = NULL;
    long tam_corpo = 0;
    char alvo[1300];
    char *interrogacao;

    while (total < TAM_REQUISICAO)
    {
        int n = recv(cliente, g_requisicao + total, (int)(TAM_REQUISICAO - total), 0);
        if (n <= 0)
            break;
        total += (size_t)n;
        g_requisicao[total] = '\0';

        if (fim_cabecalho == NULL)
        {
            fim_cabecalho = strstr(g_requisicao, "\r\n\r\n");
            if (fim_cabecalho != NULL)
            {
                tam_corpo = tamanho_do_corpo(g_requisicao);
            }
        }
        if (fim_cabecalho != NULL)
        {
            size_t cabecalho_tam = (size_t)(fim_cabecalho + 4 - g_requisicao);
            if (total >= cabecalho_tam + (size_t)tam_corpo)
                break;
        }
    }

    if (fim_cabecalho == NULL)
        return 0;

    req->corpo = fim_cabecalho + 4;

    memset(alvo, 0, sizeof(alvo));
    if (sscanf(g_requisicao, "%7s %1299s", req->metodo, alvo) != 2)
        return 0;

    req->query[0] = '\0';
    interrogacao = strchr(alvo, '?');
    if (interrogacao != NULL)
    {
        *interrogacao = '\0';
        snprintf(req->query, sizeof(req->query), "%s", interrogacao + 1);
    }
    snprintf(req->caminho, sizeof(req->caminho), "%s", alvo);
    return 1;
}

static const char *texto_do_status(int status)
{
    if (status == 200)
        return "OK";
    if (status == 204)
        return "No Content";
    if (status == 400)
        return "Bad Request";
    if (status == 404)
        return "Not Found";
    return "Internal Server Error";
}

static void enviar_tudo(socket_t cliente, const char *dados, size_t tamanho)
{
    size_t enviado = 0;
    while (enviado < tamanho)
    {
        int n = send(cliente, dados + enviado, (int)(tamanho - enviado), 0);
        if (n <= 0)
            return;
        enviado += (size_t)n;
    }
}

static void enviar_resposta(socket_t cliente, int status, const char *tipo, const char *corpo, size_t tamanho)
{
    char cabecalho[512];
    int n = snprintf(cabecalho, sizeof(cabecalho),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: %s\r\n"
                     "Content-Length: %zu\r\n"
                     "Cache-Control: no-store\r\n"
                     "Access-Control-Allow-Origin: *\r\n"
                     "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                     "Access-Control-Allow-Headers: Content-Type\r\n"
                     "Connection: close\r\n\r\n",
                     status, texto_do_status(status), tipo, tamanho);

    enviar_tudo(cliente, cabecalho, (size_t)n);
    if (tamanho > 0)
        enviar_tudo(cliente, corpo, tamanho);
}

static void responder_json(socket_t cliente, int status, Texto *t)
{
    enviar_resposta(cliente, status, "application/json; charset=utf-8", t->dados, t->tamanho);
}

static void responder_erro(socket_t cliente, int status, const char *mensagem)
{
    Texto t;
    texto_iniciar(&t);
    texto_add(&t, "{\"ok\":false,\"erro\":");
    texto_add_string(&t, mensagem);
    texto_add(&t, "}");
    responder_json(cliente, status, &t);
    texto_liberar(&t);
}

static void responder_ok(socket_t cliente, int id)
{
    Texto t;
    texto_iniciar(&t);
    texto_add(&t, "{\"ok\":true,\"id\":%d}", id);
    responder_json(cliente, 200, &t);
    texto_liberar(&t);
}

/* Entrega um dos 3 arquivos do site */
static void servir_arquivo(socket_t cliente, const char *nome, const char *tipo)
{
    FILE *arquivo = fopen(nome, "rb");
    long tamanho;
    char *dados;

    if (arquivo == NULL)
    {
        const char *aviso = "Arquivo do site nao encontrado. Rode o servidor dentro da pasta do site.";
        enviar_resposta(cliente, 404, "text/plain; charset=utf-8", aviso, strlen(aviso));
        return;
    }

    fseek(arquivo, 0, SEEK_END);
    tamanho = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

    dados = malloc((size_t)tamanho + 1);
    if (dados == NULL)
    {
        fclose(arquivo);
        responder_erro(cliente, 500, "Sem memoria.");
        return;
    }
    tamanho = (long)fread(dados, 1, (size_t)tamanho, arquivo);
    fclose(arquivo);

    enviar_resposta(cliente, 200, tipo, dados, (size_t)tamanho);
    free(dados);
}

/* ---------- 3. PARÂMETROS (texto que chega do site) ---------- */

/* Decodifica %XX e '+'. Devolve o tamanho, ou -1 se não couber em "tam" */
static int decodificar_url(const char *origem, size_t n, char *destino, size_t tam)
{
    size_t j = 0;

    for (size_t i = 0; i < n; i++)
    {
        char c = origem[i];

        if (c == '+')
        {
            c = ' ';
        }
        else if (c == '%' && i + 2 < n && isxdigit((unsigned char)origem[i + 1]) && isxdigit((unsigned char)origem[i + 2]))
        {
            char hex[3] = {origem[i + 1], origem[i + 2], '\0'};
            c = (char)strtol(hex, NULL, 16);
            i += 2;
        }

        if (j + 1 >= tam)
            return -1;
        destino[j++] = c;
    }
    destino[j] = '\0';
    return (int)j;
}

static void aparar(char *s)
{
    size_t inicio = 0;
    size_t fim = strlen(s);

    while (s[inicio] && isspace((unsigned char)s[inicio]))
        inicio++;
    while (fim > inicio && isspace((unsigned char)s[fim - 1]))
        fim--;

    memmove(s, s + inicio, fim - inicio);
    s[fim - inicio] = '\0';
}

/* Procura "chave=valor" em "a=1&b=2". Devolve 1 achou, 0 não achou, -1 grande demais */
static int param_texto(const char *dados, const char *chave, char *saida, size_t tam)
{
    size_t tam_chave = strlen(chave);
    const char *p = dados;

    saida[0] = '\0';

    while (p != NULL && *p)
    {
        const char *fim = strchr(p, '&');
        size_t tam_par = fim ? (size_t)(fim - p) : strlen(p);

        if (tam_par > tam_chave && strncmp(p, chave, tam_chave) == 0 && p[tam_chave] == '=')
        {
            if (decodificar_url(p + tam_chave + 1, tam_par - tam_chave - 1, saida, tam) < 0)
            {
                saida[0] = '\0';
                return -1;
            }
            aparar(saida);
            return 1;
        }
        p = fim ? fim + 1 : NULL;
    }
    return 0;
}

/* Lê um número inteiro. Devolve 1 se achou e é número */
static int param_int(const char *dados, const char *chave, int *valor)
{
    char texto[32];
    char *fim;
    long numero;

    if (param_texto(dados, chave, texto, sizeof(texto)) != 1 || texto[0] == '\0')
        return 0;

    numero = strtol(texto, &fim, 10);
    if (*fim != '\0' || numero < -2000000000L || numero > 2000000000L)
        return 0;

    *valor = (int)numero;
    return 1;
}

static int data_valida(const char *s)
{
    if (strlen(s) != 10)
        return 0;
    for (int i = 0; i < 10; i++)
    {
        if (i == 4 || i == 7)
        {
            if (s[i] != '-')
                return 0;
        }
        else if (!isdigit((unsigned char)s[i]))
        {
            return 0;
        }
    }
    return 1;
}

static int hora_valida(const char *s)
{
    size_t n = strlen(s);

    if (n != 5 && n != 8)
        return 0;
    for (size_t i = 0; i < n; i++)
    {
        if (i == 2 || i == 5)
        {
            if (s[i] != ':')
                return 0;
        }
        else if (!isdigit((unsigned char)s[i]))
        {
            return 0;
        }
    }
    return 1;
}

/* ---------- 4. CONSULTAS DIRETAS (o banco.c não tem função para estas) ---------- */

static void copiar(char *destino, size_t tam, const char *origem)
{
    snprintf(destino, tam, "%s", origem ? origem : "");
}

/* Devolve 1 se achou, 0 se não existe, -1 se deu erro no banco */
static int buscar_campeonato(MYSQL *conexao, int id, Campeonato *c)
{
    char sql[300];
    MYSQL_RES *resultado;
    MYSQL_ROW linha;

    snprintf(sql, sizeof(sql),
             "SELECT ID_campeonato, nome, jogo, data_inicio, tempo_duracao, num_equipes "
             "FROM campeonato WHERE ID_campeonato = %d",
             id);

    if (mysql_query(conexao, sql))
        return -1;
    resultado = mysql_store_result(conexao);
    if (resultado == NULL)
        return -1;

    linha = mysql_fetch_row(resultado);
    if (linha == NULL)
    {
        mysql_free_result(resultado);
        return 0;
    }

    c->id_campeonato = atoi(linha[0]);
    copiar(c->nome, sizeof(c->nome), linha[1]);
    copiar(c->jogo, sizeof(c->jogo), linha[2]);
    copiar(c->data_inicio, sizeof(c->data_inicio), linha[3]);
    copiar(c->tempo_duracao, sizeof(c->tempo_duracao), linha[4]);
    c->num_equipes = linha[5] ? atoi(linha[5]) : 0;

    mysql_free_result(resultado);
    return 1;
}

/* UPDATE do Atualizar_Nome_Equipe_Banco, mas recebendo os dados em vez de usar scanf.
   Devolve as linhas alteradas, -1 se deu erro, -2 se o nome já existe */
static int atualizar_nome_equipe(MYSQL *conexao, int id_equipe, const char *novo_nome)
{
    MYSQL_STMT *stmt = mysql_stmt_init(conexao);
    const char *sql = "UPDATE equipes SET nome_equipe = ? WHERE ID_equipe = ?";
    MYSQL_BIND parametros[2];
    unsigned long tamanho_nome = (unsigned long)strlen(novo_nome);
    int linhas;

    if (stmt == NULL)
        return -1;

    if (mysql_stmt_prepare(stmt, sql, strlen(sql)))
    {
        printf("Erro ao preparar SQL: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    memset(parametros, 0, sizeof(parametros));

    parametros[0].buffer_type = MYSQL_TYPE_STRING;
    parametros[0].buffer = (char *)novo_nome;
    parametros[0].buffer_length = tamanho_nome;
    parametros[0].length = &tamanho_nome;

    parametros[1].buffer_type = MYSQL_TYPE_LONG;
    parametros[1].buffer = &id_equipe;

    if (mysql_stmt_bind_param(stmt, parametros))
    {
        printf("Erro ao associar dados: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    if (mysql_stmt_execute(stmt))
    {
        int duplicado = (mysql_stmt_errno(stmt) == 1062);
        printf("Erro ao atualizar equipe: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return duplicado ? -2 : -1;
    }

    linhas = (int)mysql_stmt_affected_rows(stmt);
    mysql_stmt_close(stmt);
    return linhas;
}

/* DELETE do Excluir_Jogador_Banco, mas recebendo o ID em vez de usar scanf.
   Devolve as linhas apagadas ou -1 se deu erro */
static int excluir_jogador(MYSQL *conexao, int id_jogador)
{
    MYSQL_STMT *stmt = mysql_stmt_init(conexao);
    const char *sql = "DELETE FROM jogadores WHERE ID_jogador = ?";
    MYSQL_BIND parametro;
    int linhas;

    if (stmt == NULL)
        return -1;

    if (mysql_stmt_prepare(stmt, sql, strlen(sql)))
    {
        printf("Erro ao preparar SQL: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    memset(&parametro, 0, sizeof(parametro));
    parametro.buffer_type = MYSQL_TYPE_LONG;
    parametro.buffer = &id_jogador;

    if (mysql_stmt_bind_param(stmt, &parametro) || mysql_stmt_execute(stmt))
    {
        printf("Erro ao excluir jogador: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return -1;
    }

    linhas = (int)mysql_stmt_affected_rows(stmt);
    mysql_stmt_close(stmt);
    return linhas;
}

/* ---------- 5. JSON DAS TABELAS ---------- */

static void json_campeonato(Texto *t, const Campeonato *c)
{
    texto_add(t, "{\"id_campeonato\":%d,\"nome\":", c->id_campeonato);
    texto_add_string(t, c->nome);
    texto_add(t, ",\"jogo\":");
    texto_add_string(t, c->jogo);
    texto_add(t, ",\"data_inicio\":");
    texto_add_string(t, c->data_inicio);
    texto_add(t, ",\"tempo_duracao\":");
    texto_add_string(t, c->tempo_duracao);
    texto_add(t, ",\"num_equipes\":%d}", c->num_equipes);
}

static void json_equipe(Texto *t, const Equipe *e)
{
    texto_add(t, "{\"id_equipe\":%d,\"nome_equipe\":", e->id_equipe);
    texto_add_string(t, e->nome_equipe);
    texto_add(t, ",\"qntd_membros\":%d,\"id_campeonato\":%d}", e->qntd_membros, e->id_campeonato);
}

static void json_jogador(Texto *t, const Jogador *j)
{
    texto_add(t, "{\"id_jogador\":%d,\"nome_jogador\":", j->id_jogador);
    texto_add_string(t, j->nome_jogador);
    texto_add(t, ",\"idade\":%d,\"email\":", j->idade);
    texto_add_string(t, j->email);
    texto_add(t, ",\"telefone\":");
    texto_add_string(t, j->telefone);
    texto_add(t, ",\"id_equipe\":%d}", j->id_equipe);
}

static void json_partida(Texto *t, const Partida *p)
{
    texto_add(t, "{\"id_partida\":%d,\"data_partida\":", p->id_partida);
    texto_add_string(t, p->data_partida);
    texto_add(t, ",\"horario\":");
    texto_add_string(t, p->horario);
    texto_add(t, ",\"tempo_duracao\":");
    texto_add_string(t, p->tempo_duracao);
    texto_add(t, ",\"equipe_1\":%d,\"equipe_2\":%d}", p->equipe_1, p->equipe_2);
}

static void json_resultado(Texto *t, const ResultadoPartida *r)
{
    texto_add(t, "{\"id_resultado\":%d,\"equipe_venc\":", r->id_resultado);
    texto_add_string(t, r->equipe_venc);
    texto_add(t, ",\"pont_eq1\":%d,\"pont_eq2\":%d,\"id_partida\":%d}", r->pont_eq1, r->pont_eq2, r->id_partida);
}

static void json_classificacao(Texto *t, const Classificacao *c)
{
    texto_add(t, "{\"id_classificacao\":%d,\"pontuacao\":%d,\"posicao\":%d,\"id_equipe\":%d}",
              c->id_classificacao, c->pontuacao, c->posicao, c->id_equipe);
}

/* ---------- 6. ROTAS (o que o site pode pedir) ---------- */

/* GET /api/campeonatos - lista todos os campeonatos do banco */
static void rota_campeonatos(socket_t cliente, MYSQL *conexao)
{
    const char *sql =
        "SELECT ID_campeonato, nome, jogo, data_inicio, tempo_duracao, num_equipes "
        "FROM campeonato ORDER BY ID_campeonato ASC";
    MYSQL_RES *resultado;
    MYSQL_ROW linha;
    Texto t;
    int primeiro = 1;

    if (mysql_query(conexao, sql) || (resultado = mysql_store_result(conexao)) == NULL)
    {
        responder_erro(cliente, 500, "Erro ao consultar campeonatos no banco.");
        return;
    }

    texto_iniciar(&t);
    texto_add(&t, "{\"ok\":true,\"campeonatos\":[");

    while ((linha = mysql_fetch_row(resultado)) != NULL)
    {
        Campeonato c;
        c.id_campeonato = atoi(linha[0]);
        copiar(c.nome, sizeof(c.nome), linha[1]);
        copiar(c.jogo, sizeof(c.jogo), linha[2]);
        copiar(c.data_inicio, sizeof(c.data_inicio), linha[3]);
        copiar(c.tempo_duracao, sizeof(c.tempo_duracao), linha[4]);
        c.num_equipes = linha[5] ? atoi(linha[5]) : 0;

        if (!primeiro)
            texto_add(&t, ",");
        json_campeonato(&t, &c);
        primeiro = 0;
    }
    mysql_free_result(resultado);

    texto_add(&t, "]}");
    responder_json(cliente, 200, &t);
    texto_liberar(&t);
}

/* GET /api/dados?id=N - "Acessar campeonato existente": carrega tudo do campeonato */
static void rota_dados(socket_t cliente, MYSQL *conexao, const char *dados)
{
    int id_campeonato;
    Campeonato campeonato;
    int achou;
    int qntd_equipes, qntd_jogadores, qntd_partidas, qntd_resultados, qntd_classificacao;
    Texto t;

    if (!param_int(dados, "id", &id_campeonato))
    {
        responder_erro(cliente, 400, "Informe o ID do campeonato.");
        return;
    }

    achou = buscar_campeonato(conexao, id_campeonato, &campeonato);
    if (achou < 0)
    {
        responder_erro(cliente, 500, "Erro ao consultar campeonato.");
        return;
    }
    if (achou == 0)
    {
        responder_erro(cliente, 404, "Campeonato nao encontrado.");
        return;
    }

    qntd_equipes = Carregar_Equipes_Banco(conexao, g_equipes, id_campeonato);
    qntd_jogadores = Carregar_Jogadores_Banco(conexao, g_jogadores, id_campeonato);
    qntd_partidas = Carregar_Partidas_Banco(conexao, g_partidas, id_campeonato);
    qntd_resultados = Carregar_Resultados_Banco(conexao, g_resultados, id_campeonato);
    qntd_classificacao = Carregar_Classificacao_Banco(conexao, g_classificacao, id_campeonato);

    if (qntd_equipes < 0 || qntd_jogadores < 0 || qntd_partidas < 0 ||
        qntd_resultados < 0 || qntd_classificacao < 0)
    {
        responder_erro(cliente, 500, "Erro ao recuperar os dados do campeonato.");
        return;
    }

    texto_iniciar(&t);
    texto_add(&t, "{\"ok\":true,\"campeonato\":");
    json_campeonato(&t, &campeonato);

    texto_add(&t, ",\"equipes\":[");
    for (int i = 0; i < qntd_equipes; i++)
    {
        if (i > 0)
            texto_add(&t, ",");
        json_equipe(&t, &g_equipes[i]);
    }

    texto_add(&t, "],\"jogadores\":[");
    for (int i = 0; i < qntd_jogadores; i++)
    {
        if (i > 0)
            texto_add(&t, ",");
        json_jogador(&t, &g_jogadores[i]);
    }

    texto_add(&t, "],\"partidas\":[");
    for (int i = 0; i < qntd_partidas; i++)
    {
        if (i > 0)
            texto_add(&t, ",");
        json_partida(&t, &g_partidas[i]);
    }

    texto_add(&t, "],\"resultados\":[");
    for (int i = 0; i < qntd_resultados; i++)
    {
        if (i > 0)
            texto_add(&t, ",");
        json_resultado(&t, &g_resultados[i]);
    }

    texto_add(&t, "],\"classificacao\":[");
    for (int i = 0; i < qntd_classificacao; i++)
    {
        if (i > 0)
            texto_add(&t, ",");
        json_classificacao(&t, &g_classificacao[i]);
    }
    texto_add(&t, "]}");

    responder_json(cliente, 200, &t);
    texto_liberar(&t);
}

/* GET /api/buscar-equipe?id_campeonato=N&nome=X  (Buscar_Equipe_Pelo_Nome, busca linear)
   GET /api/buscar-equipe?id_campeonato=N&id=Y    (Buscar_Equipe_Pelo_ID, busca binária) */
static void rota_buscar_equipe(socket_t cliente, MYSQL *conexao, const char *dados)
{
    int id_campeonato;
    int id_busca;
    char nome[51];
    int qntd_equipes;
    int posicao;
    int por_id;
    Texto t;

    if (!param_int(dados, "id_campeonato", &id_campeonato))
    {
        responder_erro(cliente, 400, "Informe o ID do campeonato.");
        return;
    }

    por_id = param_int(dados, "id", &id_busca);

    if (!por_id && param_texto(dados, "nome", nome, sizeof(nome)) != 1)
    {
        responder_erro(cliente, 400, "Informe o nome (ate 50 caracteres) ou o ID da equipe.");
        return;
    }

    qntd_equipes = Carregar_Equipes_Banco(conexao, g_equipes, id_campeonato);
    if (qntd_equipes < 0)
    {
        responder_erro(cliente, 500, "Erro ao carregar equipes.");
        return;
    }

    if (por_id)
        posicao = Buscar_Equipe_Pelo_ID(g_equipes, qntd_equipes, id_busca);
    else
        posicao = Buscar_Equipe_Pelo_Nome(g_equipes, qntd_equipes, nome);

    texto_iniciar(&t);
    if (posicao == -1)
    {
        texto_add(&t, "{\"ok\":true,\"encontrada\":false}");
    }
    else
    {
        texto_add(&t, "{\"ok\":true,\"encontrada\":true,\"equipe\":");
        json_equipe(&t, &g_equipes[posicao]);
        texto_add(&t, "}");
    }
    responder_json(cliente, 200, &t);
    texto_liberar(&t);
}

/* POST /api/campeonato - Cadastrar_Campeonato + Salvar_Campeonato_Banco */
static void rota_cadastrar_campeonato(socket_t cliente, MYSQL *conexao, const char *dados)
{
    Campeonato novo;
    Campeonato lista[1];
    int leitura[4];

    memset(&novo, 0, sizeof(novo));

    leitura[0] = param_texto(dados, "nome", novo.nome, sizeof(novo.nome));
    leitura[1] = param_texto(dados, "jogo", novo.jogo, sizeof(novo.jogo));
    leitura[2] = param_texto(dados, "data_inicio", novo.data_inicio, sizeof(novo.data_inicio));
    leitura[3] = param_texto(dados, "tempo_duracao", novo.tempo_duracao, sizeof(novo.tempo_duracao));

    if (leitura[0] == -1 || leitura[1] == -1 || leitura[3] == -1)
    {
        responder_erro(cliente, 400, "Algum texto e maior do que o permitido.");
        return;
    }
    if (novo.nome[0] == '\0')
    {
        responder_erro(cliente, 400, "Nome do campeonato não pode ser vazio.");
        return;
    }
    if (novo.jogo[0] == '\0')
    {
        responder_erro(cliente, 400, "Jogo do campeonato não pode ser vazio.");
        return;
    }
    if (!data_valida(novo.data_inicio))
    {
        responder_erro(cliente, 400, "Data de inicio invalida. Use AAAA-MM-DD.");
        return;
    }
    if (novo.tempo_duracao[0] == '\0')
    {
        responder_erro(cliente, 400, "Tempo de duracao do campeonato não pode ser vazio.");
        return;
    }
    if (!param_int(dados, "num_equipes", &novo.num_equipes))
    {
        responder_erro(cliente, 400, "Quantidade de equipes invalida.");
        return;
    }

    if (!Cadastrar_Campeonato(lista, 0, &novo))
    {
        responder_erro(cliente, 400, "A quantidade de equipes deve ficar entre 2 e 100.");
        return;
    }

    if (!Salvar_Campeonato_Banco(conexao, &novo))
    {
        responder_erro(cliente, 500, "Erro ao salvar campeonato no banco.");
        return;
    }

    responder_ok(cliente, novo.id_campeonato);
}

/* POST /api/equipe - Cadastrar_Equipe + Salvar_Equipe_Banco */
static void rota_cadastrar_equipe(socket_t cliente, MYSQL *conexao, const char *dados)
{
    int id_campeonato;
    Campeonato campeonato;
    Equipe nova;
    int achou;
    int qntd_equipes;
    int existe;

    memset(&nova, 0, sizeof(nova));

    if (!param_int(dados, "id_campeonato", &id_campeonato) ||
        !param_int(dados, "qntd_membros", &nova.qntd_membros))
    {
        responder_erro(cliente, 400, "Dados da equipe incompletos.");
        return;
    }
    if (param_texto(dados, "nome_equipe", nova.nome_equipe, sizeof(nova.nome_equipe)) != 1 ||
        nova.nome_equipe[0] == '\0')
    {
        responder_erro(cliente, 400, "Nome da equipe nao pode ser vazio (ate 50 caracteres).");
        return;
    }

    achou = buscar_campeonato(conexao, id_campeonato, &campeonato);
    if (achou < 0)
    {
        responder_erro(cliente, 500, "Erro ao consultar campeonato.");
        return;
    }
    if (achou == 0)
    {
        responder_erro(cliente, 404, "Campeonato nao encontrado.");
        return;
    }

    qntd_equipes = Carregar_Equipes_Banco(conexao, g_equipes, id_campeonato);
    if (qntd_equipes < 0)
    {
        responder_erro(cliente, 500, "Erro ao carregar equipes.");
        return;
    }

    /* o main.c cadastra exatamente "num_equipes" equipes por campeonato */
    if (qntd_equipes >= campeonato.num_equipes)
    {
        char mensagem[120];
        snprintf(mensagem, sizeof(mensagem), "Esse campeonato já tem as %d equipes previstas.", campeonato.num_equipes);
        responder_erro(cliente, 400, mensagem);
        return;
    }

    existe = nome_equipe_existe(conexao, nova.nome_equipe);
    if (existe == -1)
    {
        responder_erro(cliente, 500, "Erro ao consultar o banco.");
        return;
    }
    if (existe == 1)
    {
        responder_erro(cliente, 400, "Já existe uma equipe com esse nome. Digite outro nome.");
        return;
    }

    if (!Cadastrar_Equipe(g_equipes, qntd_equipes, nova, id_campeonato))
    {
        responder_erro(cliente, 400,
                       "Equipe recusada: de 1 a 500 membros, no máximo 100 equipes e a soma de membros do campeonato não pode passar de 500.");
        return;
    }

    if (!Salvar_Equipe_Banco(conexao, &g_equipes[qntd_equipes]))
    {
        responder_erro(cliente, 500, "Erro ao salvar equipe no banco.");
        return;
    }

    responder_ok(cliente, g_equipes[qntd_equipes].id_equipe);
}

/* POST /api/equipe-nome - Atualizar_Nome_Equipe_Banco */
static void rota_atualizar_nome(socket_t cliente, MYSQL *conexao, const char *dados)
{
    int id_equipe;
    char novo_nome[51];
    int linhas;

    if (!param_int(dados, "id_equipe", &id_equipe))
    {
        responder_erro(cliente, 400, "Informe o ID da equipe.");
        return;
    }
    if (param_texto(dados, "novo_nome", novo_nome, sizeof(novo_nome)) != 1 || novo_nome[0] == '\0')
    {
        responder_erro(cliente, 400, "Novo nome da equipe nao pode ser vazio (ate 50 caracteres).");
        return;
    }

    linhas = atualizar_nome_equipe(conexao, id_equipe, novo_nome);

    if (linhas == -2)
    {
        responder_erro(cliente, 400, "Já existe uma equipe com esse nome. Digite outro nome.");
        return;
    }
    if (linhas < 0)
    {
        responder_erro(cliente, 500, "Erro ao atualizar equipe.");
        return;
    }
    if (linhas == 0)
    {
        responder_erro(cliente, 400, "Nenhuma equipe foi alterada. Verifique o ID e o nome.");
        return;
    }

    responder_ok(cliente, id_equipe);
}

/* POST /api/jogador - validações do Cadastrar_Jogadores + Salvar_Jogador_Banco */
static void rota_cadastrar_jogador(socket_t cliente, MYSQL *conexao, const char *dados)
{
    Jogador novo;
    char sql[300];
    char nome_equipe[51];
    int qntd_membros;
    int cadastrados;
    int existe;
    MYSQL_RES *resultado;
    MYSQL_ROW linha;

    memset(&novo, 0, sizeof(novo));

    if (!param_int(dados, "id_equipe", &novo.id_equipe) || !param_int(dados, "idade", &novo.idade))
    {
        responder_erro(cliente, 400, "Dados do jogador incompletos.");
        return;
    }
    if (param_texto(dados, "nome_jogador", novo.nome_jogador, sizeof(novo.nome_jogador)) != 1 ||
        novo.nome_jogador[0] == '\0')
    {
        responder_erro(cliente, 400, "Nome do jogador não pode ser vazio (ate 100 caracteres).");
        return;
    }
    if (novo.idade < 1 || novo.idade > 120)
    {
        responder_erro(cliente, 400, "Idade invalida (de 1 a 120).");
        return;
    }
    if (param_texto(dados, "email", novo.email, sizeof(novo.email)) != 1 || novo.email[0] == '\0')
    {
        responder_erro(cliente, 400, "E-mail invalido (ate 50 caracteres).");
        return;
    }
    if (param_texto(dados, "telefone", novo.telefone, sizeof(novo.telefone)) != 1 || novo.telefone[0] == '\0')
    {
        responder_erro(cliente, 400, "Telefone invalido (ate 14 caracteres).");
        return;
    }

    /* a equipe recebe só a quantidade de jogadores que foi informada */
    snprintf(sql, sizeof(sql),
             "SELECT nome_equipe, qntd_membros, "
             "(SELECT COUNT(*) FROM jogadores WHERE ID_equipe = %d) "
             "FROM equipes WHERE ID_equipe = %d",
             novo.id_equipe, novo.id_equipe);

    if (mysql_query(conexao, sql) || (resultado = mysql_store_result(conexao)) == NULL)
    {
        responder_erro(cliente, 500, "Erro ao consultar equipe.");
        return;
    }
    linha = mysql_fetch_row(resultado);
    if (linha == NULL)
    {
        mysql_free_result(resultado);
        responder_erro(cliente, 404, "Equipe nao encontrada.");
        return;
    }
    copiar(nome_equipe, sizeof(nome_equipe), linha[0]);
    qntd_membros = atoi(linha[1]);
    cadastrados = atoi(linha[2]);
    mysql_free_result(resultado);

    if (cadastrados >= qntd_membros)
    {
        char mensagem[160];
        snprintf(mensagem, sizeof(mensagem), "%s já tem os %d jogadores informados.", nome_equipe, qntd_membros);
        responder_erro(cliente, 400, mensagem);
        return;
    }

    existe = email_jogador_existe(conexao, novo.email);
    if (existe == -1)
    {
        responder_erro(cliente, 500, "Erro ao consultar o banco.");
        return;
    }
    if (existe == 1)
    {
        responder_erro(cliente, 400, "Já existe um jogador com esse e-mail. Digite outro e-mail.");
        return;
    }

    existe = telefone_jogador_existe(conexao, novo.telefone);
    if (existe == -1)
    {
        responder_erro(cliente, 500, "Erro ao consultar o banco.");
        return;
    }
    if (existe == 1)
    {
        responder_erro(cliente, 400, "Já existe um jogador com esse telefone. Digite outro telefone.");
        return;
    }

    if (!Salvar_Jogador_Banco(conexao, &novo))
    {
        responder_erro(cliente, 500, "Erro ao salvar jogador no banco.");
        return;
    }

    responder_ok(cliente, novo.id_jogador);
}

/* POST /api/jogador-excluir - Excluir_Jogador_Banco */
static void rota_excluir_jogador(socket_t cliente, MYSQL *conexao, const char *dados)
{
    int id_jogador;
    int linhas;

    if (!param_int(dados, "id_jogador", &id_jogador))
    {
        responder_erro(cliente, 400, "Informe o ID do jogador.");
        return;
    }

    linhas = excluir_jogador(conexao, id_jogador);

    if (linhas < 0)
    {
        responder_erro(cliente, 500, "Erro ao excluir jogador.");
        return;
    }
    if (linhas == 0)
    {
        responder_erro(cliente, 404, "Nenhum jogador encontrado com esse ID.");
        return;
    }

    responder_ok(cliente, id_jogador);
}

/* POST /api/partida - validações do Cadastrar_Partida + Salvar_Partida_Banco */
static void rota_cadastrar_partida(socket_t cliente, MYSQL *conexao, const char *dados)
{
    int id_campeonato;
    Partida nova;
    int qntd_equipes;
    int qntd_partidas;

    memset(&nova, 0, sizeof(nova));

    if (!param_int(dados, "id_campeonato", &id_campeonato) ||
        !param_int(dados, "equipe_1", &nova.equipe_1) ||
        !param_int(dados, "equipe_2", &nova.equipe_2))
    {
        responder_erro(cliente, 400, "Dados da partida incompletos.");
        return;
    }
    if (param_texto(dados, "data_partida", nova.data_partida, sizeof(nova.data_partida)) != 1 ||
        !data_valida(nova.data_partida))
    {
        responder_erro(cliente, 400, "Data invalida. Use AAAA-MM-DD.");
        return;
    }
    if (param_texto(dados, "horario", nova.horario, sizeof(nova.horario)) != 1 ||
        !hora_valida(nova.horario))
    {
        responder_erro(cliente, 400, "Horario invalido. Use HH:MM.");
        return;
    }
    if (param_texto(dados, "tempo_duracao", nova.tempo_duracao, sizeof(nova.tempo_duracao)) != 1 ||
        nova.tempo_duracao[0] == '\0')
    {
        responder_erro(cliente, 400, "Tempo de duracao nao pode ser vazio (ate 20 caracteres).");
        return;
    }

    qntd_equipes = Carregar_Equipes_Banco(conexao, g_equipes, id_campeonato);
    qntd_partidas = Carregar_Partidas_Banco(conexao, g_partidas, id_campeonato);
    if (qntd_equipes < 0 || qntd_partidas < 0)
    {
        responder_erro(cliente, 500, "Erro ao carregar dados do campeonato.");
        return;
    }

    if (qntd_partidas >= 100 || qntd_equipes < 2)
    {
        responder_erro(cliente, 400, "Cadastre pelo menos 2 equipes (limite de 100 partidas).");
        return;
    }

    if (Buscar_Equipe_Pelo_ID(g_equipes, qntd_equipes, nova.equipe_1) == -1 ||
        Buscar_Equipe_Pelo_ID(g_equipes, qntd_equipes, nova.equipe_2) == -1 ||
        nova.equipe_1 == nova.equipe_2)
    {
        responder_erro(cliente, 400, "Equipes invalidas para a partida.");
        return;
    }

    if (!Salvar_Partida_Banco(conexao, &nova))
    {
        responder_erro(cliente, 500, "Erro ao salvar partida no banco.");
        return;
    }

    responder_ok(cliente, nova.id_partida);
}

/* POST /api/resultado - validações do Cadastrar_Resultado + Salvar_Resultado_Banco
   + Atualizar_Classificacao_Banco */
static void rota_cadastrar_resultado(socket_t cliente, MYSQL *conexao, const char *dados)
{
    int id_campeonato;
    ResultadoPartida novo;
    int qntd_partidas;
    int qntd_resultados;
    int achou = 0;

    memset(&novo, 0, sizeof(novo));

    if (!param_int(dados, "id_campeonato", &id_campeonato) ||
        !param_int(dados, "id_partida", &novo.id_partida) ||
        !param_int(dados, "pont_eq1", &novo.pont_eq1) ||
        !param_int(dados, "pont_eq2", &novo.pont_eq2))
    {
        responder_erro(cliente, 400, "Dados do resultado incompletos.");
        return;
    }

    qntd_partidas = Carregar_Partidas_Banco(conexao, g_partidas, id_campeonato);
    qntd_resultados = Carregar_Resultados_Banco(conexao, g_resultados, id_campeonato);
    if (qntd_partidas < 0 || qntd_resultados < 0)
    {
        responder_erro(cliente, 500, "Erro ao carregar dados do campeonato.");
        return;
    }

    if (qntd_resultados >= 100)
    {
        responder_erro(cliente, 400, "Limite de 100 resultados atingido.");
        return;
    }

    for (int i = 0; i < qntd_partidas; i++)
    {
        if (g_partidas[i].id_partida == novo.id_partida)
            achou = 1;
    }
    if (!achou)
    {
        responder_erro(cliente, 404, "Partida nao encontrada.");
        return;
    }

    for (int i = 0; i < qntd_resultados; i++)
    {
        if (g_resultados[i].id_partida == novo.id_partida)
        {
            responder_erro(cliente, 400, "Essa partida ja possui resultado.");
            return;
        }
    }

    if (novo.pont_eq1 < 0 || novo.pont_eq2 < 0)
    {
        responder_erro(cliente, 400, "Pontuacao invalida.");
        return;
    }

    /* o Cadastrar_Resultado do funcoes.c grava "N/A" como equipe vencedora */
    strcpy(novo.equipe_venc, "N/A");

    if (!Salvar_Resultado_Banco(conexao, &novo))
    {
        responder_erro(cliente, 500, "Erro ao salvar resultado no banco.");
        return;
    }

    if (!Atualizar_Classificacao_Banco(conexao, id_campeonato))
    {
        responder_erro(cliente, 500, "Erro ao atualizar classificacao.");
        return;
    }

    responder_ok(cliente, novo.id_resultado);
}

/* ---------- 7. ATENDER UM PEDIDO ---------- */

static void atender(socket_t cliente, MYSQL **conexao)
{
    Requisicao req;
    const char *dados;
    int eh_get;
    int eh_post;

    memset(&req, 0, sizeof(req));

    if (!ler_requisicao(cliente, &req))
        return;

    printf("%s %s\n", req.metodo, req.caminho);

    eh_get = strcmp(req.metodo, "GET") == 0;
    eh_post = strcmp(req.metodo, "POST") == 0;

    if (strcmp(req.metodo, "OPTIONS") == 0)
    {
        enviar_resposta(cliente, 204, "text/plain", "", 0);
        return;
    }

    /* arquivos do site */
    if (eh_get && (strcmp(req.caminho, "/") == 0 || strcmp(req.caminho, "/index.html") == 0))
    {
        servir_arquivo(cliente, "index.html", "text/html; charset=utf-8");
        return;
    }
    if (eh_get && strcmp(req.caminho, "/style.css") == 0)
    {
        servir_arquivo(cliente, "style.css", "text/css; charset=utf-8");
        return;
    }
    if (eh_get && strcmp(req.caminho, "/script.js") == 0)
    {
        servir_arquivo(cliente, "script.js", "text/javascript; charset=utf-8");
        return;
    }

    if (strncmp(req.caminho, "/api/", 5) != 0)
    {
        responder_erro(cliente, 404, "Pagina nao encontrada.");
        return;
    }

    /* se o MySQL derrubou a conexao por inatividade, conecta de novo */
    if (*conexao == NULL || mysql_ping(*conexao) != 0)
    {
        Desconectar_Banco(*conexao);
        *conexao = Conectar_Banco();
        if (*conexao == NULL)
        {
            responder_erro(cliente, 500, "Sem conexao com o banco de dados.");
            return;
        }
    }

    dados = eh_post ? req.corpo : req.query;

    if (eh_get && strcmp(req.caminho, "/api/campeonatos") == 0)
        rota_campeonatos(cliente, *conexao);
    else if (eh_get && strcmp(req.caminho, "/api/dados") == 0)
        rota_dados(cliente, *conexao, dados);
    else if (eh_get && strcmp(req.caminho, "/api/buscar-equipe") == 0)
        rota_buscar_equipe(cliente, *conexao, dados);
    else if (eh_post && strcmp(req.caminho, "/api/campeonato") == 0)
        rota_cadastrar_campeonato(cliente, *conexao, dados);
    else if (eh_post && strcmp(req.caminho, "/api/equipe") == 0)
        rota_cadastrar_equipe(cliente, *conexao, dados);
    else if (eh_post && strcmp(req.caminho, "/api/equipe-nome") == 0)
        rota_atualizar_nome(cliente, *conexao, dados);
    else if (eh_post && strcmp(req.caminho, "/api/jogador") == 0)
        rota_cadastrar_jogador(cliente, *conexao, dados);
    else if (eh_post && strcmp(req.caminho, "/api/jogador-excluir") == 0)
        rota_excluir_jogador(cliente, *conexao, dados);
    else if (eh_post && strcmp(req.caminho, "/api/partida") == 0)
        rota_cadastrar_partida(cliente, *conexao, dados);
    else if (eh_post && strcmp(req.caminho, "/api/resultado") == 0)
        rota_cadastrar_resultado(cliente, *conexao, dados);
    else
        responder_erro(cliente, 404, "Rota nao encontrada.");
}

/* ---------- 8. PROGRAMA PRINCIPAL ---------- */

int main(void)
{
    MYSQL *conexao;
    socket_t servidor;
    struct sockaddr_in endereco;

    setvbuf(stdout, NULL, _IONBF, 0);

    conexao = Conectar_Banco();
    if (conexao == NULL)
    {
        printf("Nao foi possivel conectar ao banco.\n");
        return 1;
    }

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Erro ao iniciar o Winsock.\n");
        Desconectar_Banco(conexao);
        return 1;
    }
#else
    signal(SIGPIPE, SIG_IGN); /* cliente que fechou a conexao nao derruba o servidor */
#endif

    servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor == SOCKET_INVALIDO)
    {
        printf("Erro ao criar o socket.\n");
        Desconectar_Banco(conexao);
        return 1;
    }

#ifndef _WIN32
    int sim = 1;
    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, (const char *)&sim, sizeof(sim));
#endif

    memset(&endereco, 0, sizeof(endereco));
    endereco.sin_family = AF_INET;
    endereco.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* só este computador */
    endereco.sin_port = htons(PORTA);

    if (bind(servidor, (struct sockaddr *)&endereco, sizeof(endereco)) != 0 ||
        listen(servidor, 8) != 0)
    {
        printf("Erro ao abrir a porta %d (ja esta em uso?).\n", PORTA);
        FECHAR_SOCKET(servidor);
        Desconectar_Banco(conexao);
        return 1;
    }

    printf("\nServidor rodando! Abra no navegador: http://localhost:%d\n", PORTA);
    printf("Para parar, feche esta janela (Ctrl+C).\n\n");

    for (;;)
    {
        socket_t cliente = accept(servidor, NULL, NULL);

        if (cliente == SOCKET_INVALIDO)
            continue;

        atender(cliente, &conexao);
        FECHAR_SOCKET(cliente);
    }

    FECHAR_SOCKET(servidor);
    Desconectar_Banco(conexao);
    return 0;
}
