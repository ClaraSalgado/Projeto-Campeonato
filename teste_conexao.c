
#include <stdio.h>
#include <mysql.h>

int main() {

    MYSQL *conexao;

    conexao = mysql_init(NULL);

    if (conexao == NULL) {
        printf("Erro ao inicializar MySQL.\n");
        return 1;
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
        return 1;
    }

    printf("Conexao com MySQL realizada com sucesso!\n");

    mysql_close(conexao);

    return 0;
}
