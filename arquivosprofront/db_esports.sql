DROP DATABASE IF EXISTS esports;

CREATE DATABASE IF NOT EXISTS esports;

USE esports;

CREATE TABLE IF NOT EXISTS campeonato(
	ID_campeonato INT NOT NULL AUTO_INCREMENT UNIQUE,
	nome VARCHAR(100) NOT NULL,
	jogo VARCHAR(50) NOT NULL,
	data_inicio DATE NOT NULL,
	tempo_duracao VARCHAR(20),
	num_equipes INT,
	PRIMARY KEY(ID_campeonato)
);

CREATE TABLE IF NOT EXISTS equipes(
	ID_equipe INT NOT NULL AUTO_INCREMENT UNIQUE,
	nome_equipe VARCHAR(50) NOT NULL UNIQUE,
	qntd_membros INT NOT NULL,
	ID_campeonato INT NOT NULL,
	PRIMARY KEY(ID_equipe),
	FOREIGN KEY (ID_campeonato) REFERENCES campeonato(ID_campeonato)
);

CREATE TABLE IF NOT EXISTS jogadores(
	ID_jogador INT NOT NULL AUTO_INCREMENT UNIQUE,
	nome_jogador VARCHAR(100) NOT NULL,
	idade INT NOT NULL,
	email VARCHAR(50) UNIQUE,
	telefone CHAR(14) NOT NULL UNIQUE,
	ID_equipe INT NOT NULL,
	PRIMARY KEY (ID_jogador),
    FOREIGN KEY (ID_equipe) REFERENCES equipes(ID_equipe)
);

CREATE TABLE IF NOT EXISTS partidas(
    ID_partida INT NOT NULL AUTO_INCREMENT UNIQUE,
    data_partida DATE NOT NULL,
    horario TIME,
    tempo_duracao VARCHAR(20),
    equipe_1 INT NOT NULL,
	equipe_2 INT NOT NULL,
	PRIMARY KEY (ID_partida),
	FOREIGN KEY (equipe_1) REFERENCES equipes(ID_equipe),
	FOREIGN KEY (equipe_2) REFERENCES equipes(ID_equipe)
);

CREATE TABLE IF NOT EXISTS classificacao(
	ID_classificacao INT NOT NULL AUTO_INCREMENT UNIQUE,
	pontuacao INT NOT NULL DEFAULT 0,
	posicao INT NOT NULL,
	ID_equipe INT NOT NULL,
	PRIMARY KEY (ID_classificacao),
	FOREIGN KEY (ID_equipe) REFERENCES equipes(ID_equipe)
);

CREATE TABLE IF NOT EXISTS resultado_partida(
	ID_resultado INT NOT NULL AUTO_INCREMENT UNIQUE,
	equipe_venc VARCHAR(50) NOT NULL,
	pont_eq1 INT NOT NULL,
	pont_eq2 INT NOT NULL,
	ID_partida INT NOT NULL,
	PRIMARY KEY (ID_resultado),
	FOREIGN KEY (ID_partida) REFERENCES partidas(ID_partida)
);