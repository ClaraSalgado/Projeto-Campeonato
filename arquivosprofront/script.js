/* =========================================================
   script.js - comportamento do site
   Aqui NÃO existe banco nem regra de negócio duplicada: cada ação do site vira um pedido
   (fetch) para o servidor.c, que chama as MESMAS funções do banco.c e do funcoes.c e grava
   no MySQL "esports". Depois de cada ação, o site lê tudo de novo do banco e redesenha a tela.

   Para funcionar: rode o servidor (servidor.exe / ./servidor) dentro desta pasta
   e abra http://localhost:8080 (não abra o index.html direto pelo arquivo).
   ========================================================= */

/* ---------- 1. DADOS (cópia do que veio do banco) ----------
   Depois de "acessar" um campeonato, estas listas guardam os dados dele,
   como os vetores do main.c (Carregar_..._Banco). */
var campeonatos = [];     // todos os campeonatos do banco
var equipes = [];
var jogadores = [];
var partidas = [];
var resultados = [];
var classificacao = [];

var ativo = null;         // id do campeonato em uso ("Acessar campeonato existente")
var conectado = false;    // se o último pedido ao servidor deu certo

var LIMITE_EQUIPES = 100;
var LIMITE_JOGADORES = 500;
var LIMITE_PARTIDAS = 100;
var LIMITE_RESULTADOS = 100;
var LIMITE_MEMBROS = 500;

/* ---------- 2. CONVERSA COM O SERVIDOR (servidor.c) ---------- */

// Monta "a=1&b=texto" com os caracteres especiais protegidos (é o formato que o servidor.c lê)
function montarParametros(dados) {
  var partes = [];
  for (var chave in dados) {
    partes.push(encodeURIComponent(chave) + "=" + encodeURIComponent(dados[chave]));
  }
  return partes.join("&");
}

// Faz um pedido e devolve (via Promise) o JSON da resposta.
// Se o servidor responder { ok:false, erro:"..." } ou estiver desligado, rejeita com o texto do erro.
function pedir(metodo, caminho, dados) {
  var opcoes = { method: metodo, cache: "no-store" };
  var url = caminho;

  if (metodo === "GET") {
    if (dados) { url += "?" + montarParametros(dados); }
  } else {
    opcoes.headers = { "Content-Type": "application/x-www-form-urlencoded" };
    opcoes.body = montarParametros(dados || {});
  }

  return fetch(url, opcoes).then(function (resposta) {
    conectado = true;
    return resposta.json().then(function (json) {
      if (!json.ok) { throw new Error(json.erro || "Erro no servidor."); }
      return json;
    }, function () {
      throw new Error("Resposta inesperada do servidor.");
    });
  }, function () {
    conectado = false;
    throw new Error("Sem conexão com o servidor. Rode o servidor.c e abra http://localhost:8080");
  });
}

// GET /api/campeonatos (lista todos os campeonatos do banco)
function carregarCampeonatos() {
  return pedir("GET", "/api/campeonatos").then(function (json) {
    campeonatos = json.campeonatos;
  });
}

// GET /api/dados?id=N (Carregar_Equipes/Jogadores/Partidas/Resultados/Classificacao_Banco)
function carregarDadosDoCampeonato() {
  if (ativo === null) {
    equipes = []; jogadores = []; partidas = []; resultados = []; classificacao = [];
    return Promise.resolve();
  }
  return pedir("GET", "/api/dados", { id: ativo }).then(function (json) {
    equipes = json.equipes;
    jogadores = json.jogadores;
    partidas = json.partidas;
    resultados = json.resultados;
    classificacao = json.classificacao;
  });
}

// Lê do banco tudo o que a tela precisa. Se nenhum campeonato está em uso
// (ou o que estava sumiu), usa o último cadastrado.
function recarregar() {
  return carregarCampeonatos().then(function () {
    if (buscarPorId(campeonatos, "id_campeonato", ativo) === null) {
      ativo = campeonatos.length > 0 ? campeonatos[campeonatos.length - 1].id_campeonato : null;
    }
    guardarAtivo();
    return carregarDadosDoCampeonato();
  });
}

// Só o número do campeonato em uso fica no navegador (para continuar nele ao recarregar a página).
// Todos os dados mesmo ficam no MySQL.
var CHAVE_ATIVO = "esports_campeonato_ativo";

function guardarAtivo() {
  try {
    if (ativo === null) { localStorage.removeItem(CHAVE_ATIVO); }
    else { localStorage.setItem(CHAVE_ATIVO, String(ativo)); }
  } catch (erro) { /* sem localStorage: segue usando o último campeonato */ }
}

function lerAtivoGuardado() {
  try {
    var texto = localStorage.getItem(CHAVE_ATIVO);
    if (texto !== null && texto !== "") { return Number(texto); }
  } catch (erro) { /* ignora */ }
  return null;
}

/* ---------- 3. FUNÇÕES AUXILIARES ---------- */

// Atalho para pegar um elemento da página pelo id
function pegar(id) {
  return document.getElementById(id);
}

// Impede que o texto digitado seja interpretado como HTML
function proteger(texto) {
  return String(texto).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;").replace(/"/g, "&quot;");
}

// Troca 2026-11-17 por 17/11/2026
function formatarData(data) {
  var partes = String(data).split("-");
  if (partes.length !== 3) { return data; }
  return partes[2].slice(0, 2) + "/" + partes[1] + "/" + partes[0];
}

// O MySQL devolve o horário como 20:30:00; mostramos 20:30
function formatarHorario(horario) {
  if (horario === null || horario === "") { return "Não informado"; }
  return String(horario).slice(0, 5);
}

// Procura em uma lista o item cujo "campo" tem o "valor". Se não achar, devolve null
function buscarPorId(lista, campo, valor) {
  for (var i = 0; i < lista.length; i++) {
    if (lista[i][campo] === valor) {
      return lista[i];
    }
  }
  return null;
}

function nomeDaEquipe(id) {
  var equipe = buscarPorId(equipes, "id_equipe", id);
  if (equipe === null) { return "-"; }
  return equipe.nome_equipe;
}

function nomeDoCampeonato(id) {
  var campeonato = buscarPorId(campeonatos, "id_campeonato", id);
  if (campeonato === null) { return "-"; }
  return campeonato.nome;
}

// Quantos jogadores já foram cadastrados em uma equipe
function contarJogadoresDaEquipe(idEquipe) {
  var total = 0;
  for (var i = 0; i < jogadores.length; i++) {
    if (jogadores[i].id_equipe === idEquipe) { total++; }
  }
  return total;
}

// Soma dos membros de todas as equipes do campeonato (o C limita em 500)
function somarMembros() {
  var total = 0;
  for (var i = 0; i < equipes.length; i++) {
    total = total + equipes[i].qntd_membros;
  }
  return total;
}

// Pontos que uma equipe fez em uma partida (0 se a partida ainda não tem resultado)
function pontosDaEquipeNaPartida(idEquipe, partida) {
  var resultado = buscarPorId(resultados, "id_partida", partida.id_partida);
  if (resultado === null) { return 0; }
  if (partida.equipe_1 === idEquipe) { return resultado.pont_eq1; }
  if (partida.equipe_2 === idEquipe) { return resultado.pont_eq2; }
  return 0;
}

// Mostra uma mensagem no canto da tela por 4 segundos
function mostrarMensagem(texto, ehErro) {
  var caixa = document.createElement("div");
  caixa.className = ehErro ? "mensagem erro" : "mensagem";
  caixa.textContent = texto;
  pegar("mensagens").appendChild(caixa);
  setTimeout(function () { caixa.remove(); }, 4000);
}

/* ---------- 4. AÇÕES (cada uma vira um pedido ao servidor.c) ----------
   Todas devolvem uma Promise. As regras (limites, nomes repetidos, etc.) são checadas
   pelo próprio C; se ele recusar, a mensagem dele aparece na tela. */

function exigirCampeonato() {
  if (ativo === null) { return Promise.reject(new Error("Crie ou acesse um campeonato primeiro.")); }
  return Promise.resolve();
}

// POST /api/campeonato (Cadastrar_Campeonato + Salvar_Campeonato_Banco)
// Como no main.c, depois de criar o campeonato novo, segue trabalhando nele.
function cadastrarCampeonato(novo) {
  return pedir("POST", "/api/campeonato", novo).then(function (json) {
    ativo = json.id;
  });
}

// POST /api/equipe (Cadastrar_Equipe + Salvar_Equipe_Banco)
function cadastrarEquipe(nova) {
  return exigirCampeonato().then(function () {
    nova.id_campeonato = ativo;
    return pedir("POST", "/api/equipe", nova);
  });
}

// POST /api/equipe-nome (Atualizar_Nome_Equipe_Banco)
function atualizarNomeEquipe(id, novoNome) {
  return pedir("POST", "/api/equipe-nome", { id_equipe: id, novo_nome: novoNome });
}

// POST /api/jogador (validações do Cadastrar_Jogadores + Salvar_Jogador_Banco)
function cadastrarJogador(novo) {
  return pedir("POST", "/api/jogador", novo);
}

// POST /api/jogador-excluir (Excluir_Jogador_Banco)
function excluirJogador(id) {
  return pedir("POST", "/api/jogador-excluir", { id_jogador: id });
}

// POST /api/partida (Cadastrar_Partida + Salvar_Partida_Banco)
function cadastrarPartida(nova) {
  return exigirCampeonato().then(function () {
    nova.id_campeonato = ativo;
    return pedir("POST", "/api/partida", nova);
  });
}

// POST /api/resultado (Cadastrar_Resultado + Salvar_Resultado_Banco + Atualizar_Classificacao_Banco)
function cadastrarResultado(novo) {
  return exigirCampeonato().then(function () {
    novo.id_campeonato = ativo;
    return pedir("POST", "/api/resultado", novo);
  });
}

// GET /api/buscar-equipe (Buscar_Equipe_Pelo_Nome ou Buscar_Equipe_Pelo_ID).
// "filtro" é { nome: "..." } ou { id: 3 }
function buscarEquipe(filtro) {
  return exigirCampeonato().then(function () {
    filtro.id_campeonato = ativo;
    return pedir("GET", "/api/buscar-equipe", filtro);
  });
}

// Acessar campeonato existente (opção 2 do main.c)
function acessarCampeonato(id) {
  ativo = id;
  return Promise.resolve();
}

/* ---------- 5. DESENHAR A TELA ---------- */

// Monta uma tabela. "linhas" é uma lista de linhas; cada linha é uma lista de textos.
// A primeira coluna (ID) ganha destaque. Se destacarPrimeira = true, a 1ª linha também.
function criarTabela(idTabela, titulos, linhas, textoVazio, destacarPrimeira) {
  var html = "";
  if (linhas.length === 0) {
    html = "<tbody><tr><td class='vazio'>" + textoVazio + "</td></tr></tbody>";
  } else {
    html = "<thead><tr>";
    for (var t = 0; t < titulos.length; t++) { html += "<th>" + titulos[t] + "</th>"; }
    html += "</tr></thead><tbody>";
    for (var l = 0; l < linhas.length; l++) {
      var classe = (destacarPrimeira && l === 0) ? "primeiro" : "";
      html += "<tr class='" + classe + "'>";
      for (var c = 0; c < linhas[l].length; c++) {
        var classeCelula = (c === 0) ? "destaque-num" : "";
        html += "<td class='" + classeCelula + "'>" + linhas[l][c] + "</td>";
      }
      html += "</tr>";
    }
    html += "</tbody>";
  }
  pegar(idTabela).innerHTML = html;
}

// Preenche um <select>. "itens" é uma lista de { valor, texto }
function preencherSelect(idSelect, itens, textoVazio) {
  var html = "";
  if (itens.length === 0) {
    html = "<option value=''>" + textoVazio + "</option>";
  }
  for (var i = 0; i < itens.length; i++) {
    html += "<option value='" + itens[i].valor + "'>" + proteger(itens[i].texto) + "</option>";
  }
  pegar(idSelect).innerHTML = html;
}

// Uma barra de limite (ex.: Equipes 3 / 100)
function criarBarraLimite(rotulo, valor, maximo) {
  var porcentagem = Math.min(100, valor / maximo * 100);
  return "<div class='limite'><div><span>" + rotulo + "</span><span>" + valor + " / " + maximo + "</span></div>" +
         "<div class='barra'><i style='width:" + porcentagem + "%'></i></div></div>";
}

function desenharDashboard() {
  // Destaque: o campeonato em uso
  var c = buscarPorId(campeonatos, "id_campeonato", ativo);
  if (c === null) {
    pegar("destaque").innerHTML = "<div class='titulo-grande'>Nenhum campeonato cadastrado</div>" +
      "<p>Comece pela aba Campeonatos: ele vem antes das equipes, jogadores, partidas e resultados.</p>";
  } else {
    pegar("destaque").innerHTML = "<div class='titulo-grande'>" + proteger(c.nome) + "</div>" +
      "<p><span>" + proteger(c.jogo) + "</span> | início em " + formatarData(c.data_inicio) +
      " | duração: " + proteger(c.tempo_duracao) + " | " + equipes.length + " de " + c.num_equipes + " equipes</p>";
  }

  // Quadros com as quantidades (campeonatos = todos do banco; o resto = campeonato em uso)
  var nomes = ["Campeonatos", "Equipes", "Jogadores", "Partidas", "Resultados"];
  var valores = [campeonatos.length, equipes.length, jogadores.length, partidas.length, resultados.length];
  var html = "";
  for (var n = 0; n < nomes.length; n++) {
    html += "<div class='numero'><b>" + valores[n] + "</b><small>" + nomes[n] + "</small></div>";
  }
  pegar("numeros").innerHTML = html;

  // Barras de uso dos limites
  pegar("limites").innerHTML =
    criarBarraLimite("Equipes", equipes.length, LIMITE_EQUIPES) +
    criarBarraLimite("Membros declarados nas equipes", somarMembros(), LIMITE_MEMBROS) +
    criarBarraLimite("Jogadores", jogadores.length, LIMITE_JOGADORES) +
    criarBarraLimite("Partidas", partidas.length, LIMITE_PARTIDAS) +
    criarBarraLimite("Resultados", resultados.length, LIMITE_RESULTADOS);

  // Líder: vem da tabela classificacao do banco, que só é preenchida depois do primeiro resultado
  if (classificacao.length > 0) {
    pegar("lider").innerHTML = "<div class='titulo-grande'>" + proteger(nomeDaEquipe(classificacao[0].id_equipe)) +
      "</div><p class='dica'>" + classificacao[0].pontuacao + " pontos</p>";
  } else {
    pegar("lider").innerHTML = "<p class='vazio'>A classificação aparece quando houver resultados.</p>";
  }
}

function desenharTabelas() {
  var linhas, i;

  // Campeonatos: todos os do banco, com o botão para acessar
  linhas = [];
  for (i = 0; i < campeonatos.length; i++) {
    var c = campeonatos[i];
    var acao = (c.id_campeonato === ativo)
      ? "<span class='etiqueta ok'>Em uso</span>"
      : "<button type='button' class='botao contorno mini' data-acao='acessar' data-id='" + c.id_campeonato + "'>Acessar</button>";
    linhas.push([c.id_campeonato, proteger(c.nome), proteger(c.jogo), formatarData(c.data_inicio), proteger(c.tempo_duracao), c.num_equipes, acao]);
  }
  criarTabela("tabela-campeonatos", ["ID", "Nome", "Jogo", "Início", "Duração", "Equipes", ""], linhas, "Nenhum campeonato cadastrado.", false);

  linhas = [];
  for (i = 0; i < equipes.length; i++) {
    var e = equipes[i];
    var qtd = contarJogadoresDaEquipe(e.id_equipe);
    var classe = (qtd === e.qntd_membros) ? "etiqueta ok" : "etiqueta";
    linhas.push([e.id_equipe, proteger(e.nome_equipe), proteger(nomeDoCampeonato(e.id_campeonato)), e.qntd_membros,
                 "<span class='" + classe + "'>" + qtd + " / " + e.qntd_membros + "</span>"]);
  }
  criarTabela("tabela-equipes", ["ID", "Equipe", "Campeonato", "Membros", "Jogadores cadastrados"], linhas, "Nenhuma equipe cadastrada.", false);

  linhas = [];
  for (i = 0; i < jogadores.length; i++) {
    var j = jogadores[i];
    linhas.push([j.id_jogador, proteger(j.nome_jogador), j.idade, proteger(j.email), proteger(j.telefone), proteger(nomeDaEquipe(j.id_equipe)),
                 "<button type='button' class='botao mini perigo' data-acao='excluir-jogador' data-id='" + j.id_jogador + "'>Excluir</button>"]);
  }
  criarTabela("tabela-jogadores", ["ID", "Nome", "Idade", "Email", "Telefone", "Equipe", ""], linhas, "Nenhum jogador cadastrado.", false);

  linhas = [];
  for (i = 0; i < partidas.length; i++) {
    var p = partidas[i];
    var temResultado = buscarPorId(resultados, "id_partida", p.id_partida) !== null;
    var situacao = temResultado ? "<span class='etiqueta ok'>Cadastrado</span>" : "<span class='etiqueta'>Pendente</span>";
    var duracao = (p.tempo_duracao === null || p.tempo_duracao === "") ? "Não informada" : proteger(p.tempo_duracao);
    linhas.push([p.id_partida, formatarData(p.data_partida), proteger(formatarHorario(p.horario)), duracao,
                 proteger(nomeDaEquipe(p.equipe_1)) + " x " + proteger(nomeDaEquipe(p.equipe_2)), situacao]);
  }
  criarTabela("tabela-partidas", ["ID", "Data", "Horário", "Duração", "Confronto", "Resultado"], linhas, "Nenhuma partida cadastrada.", false);

  linhas = [];
  for (i = 0; i < resultados.length; i++) {
    var r = resultados[i];
    var partida = buscarPorId(partidas, "id_partida", r.id_partida);
    var placar = partida === null ? (r.pont_eq1 + " x " + r.pont_eq2)
      : proteger(nomeDaEquipe(partida.equipe_1)) + " " + r.pont_eq1 + " x " + r.pont_eq2 + " " + proteger(nomeDaEquipe(partida.equipe_2));
    linhas.push([r.id_resultado, r.id_partida, placar, proteger(r.equipe_venc)]);
  }
  criarTabela("tabela-resultados", ["ID", "Partida", "Placar", "Vencedor"], linhas, "Nenhum resultado cadastrado.", false);

  // Classificação: lida da tabela classificacao (Carregar_Classificacao_Banco)
  linhas = [];
  for (i = 0; i < classificacao.length; i++) {
    linhas.push([classificacao[i].posicao + "º", proteger(nomeDaEquipe(classificacao[i].id_equipe)), classificacao[i].pontuacao]);
  }
  criarTabela("tabela-classificacao", ["Posição", "Equipe", "Pontos"], linhas, "A classificação é gerada depois do primeiro resultado.", classificacao.length > 0);

  desenharMatriz();
}

// Exibir_Matriz_Resultados: uma linha por equipe, uma coluna por partida e o total
function desenharMatriz() {
  var titulos = ["Equipes"];
  var linhas = [];
  var i, j;

  for (j = 0; j < partidas.length; j++) {
    titulos.push("Partida " + (j + 1));
  }
  titulos.push("Total");

  for (i = 0; i < equipes.length; i++) {
    var total = 0;
    var linha = [proteger(equipes[i].nome_equipe)];
    for (j = 0; j < partidas.length; j++) {
      var pontos = pontosDaEquipeNaPartida(equipes[i].id_equipe, partidas[j]);
      linha.push(pontos);
      total += pontos;
    }
    linha.push(total);
    linhas.push(linha);
  }
  criarTabela("tabela-matriz", titulos, linhas, "Cadastre equipes para montar a matriz.", false);
}

function desenharSelects() {
  var itens, i;

  // Campeonato em uso (canto do topo)
  itens = [];
  for (i = 0; i < campeonatos.length; i++) {
    itens.push({ valor: campeonatos[i].id_campeonato, texto: campeonatos[i].nome });
  }
  preencherSelect("select-ativo", itens, "Nenhum campeonato");
  if (ativo !== null) { pegar("select-ativo").value = ativo; }

  // Só equipes que ainda têm vagas de jogador
  itens = [];
  for (i = 0; i < equipes.length; i++) {
    if (contarJogadoresDaEquipe(equipes[i].id_equipe) < equipes[i].qntd_membros) {
      itens.push({ valor: equipes[i].id_equipe, texto: equipes[i].nome_equipe });
    }
  }
  preencherSelect("select-equipe-jogador", itens, "Nenhuma equipe com vagas de jogador");

  // Todas as equipes (para escolher o confronto e para alterar o nome)
  itens = [];
  for (i = 0; i < equipes.length; i++) {
    itens.push({ valor: equipes[i].id_equipe, texto: equipes[i].nome_equipe });
  }
  preencherSelect("select-equipe1", itens, "Cadastre equipes primeiro");
  preencherSelect("select-equipe2", itens, "Cadastre equipes primeiro");
  preencherSelect("select-equipe-renomear", itens, "Cadastre equipes primeiro");

  // Só partidas que ainda não têm resultado
  itens = [];
  for (i = 0; i < partidas.length; i++) {
    var p = partidas[i];
    if (buscarPorId(resultados, "id_partida", p.id_partida) === null) {
      itens.push({ valor: p.id_partida, texto: "#" + p.id_partida + " - " + nomeDaEquipe(p.equipe_1) + " x " + nomeDaEquipe(p.equipe_2) });
    }
  }
  preencherSelect("select-partida", itens, "Nenhuma partida sem resultado");
  atualizarRotulosDoPlacar();
}

// Mostra o nome das equipes ao lado dos campos de pontos
function atualizarRotulosDoPlacar() {
  var partida = buscarPorId(partidas, "id_partida", Number(pegar("select-partida").value));
  if (partida === null) {
    pegar("rotulo-pontos1").textContent = "Pontos da equipe 1";
    pegar("rotulo-pontos2").textContent = "Pontos da equipe 2";
  } else {
    pegar("rotulo-pontos1").textContent = "Pontos da equipe " + nomeDaEquipe(partida.equipe_1);
    pegar("rotulo-pontos2").textContent = "Pontos da equipe " + nomeDaEquipe(partida.equipe_2);
  }
}
pegar("select-partida").onchange = atualizarRotulosDoPlacar;

// Passo a passo do dashboard: mostra em que etapa do fluxo do programa você está.
// Cada passo é um link para a página onde ele é feito.
function desenharPassos() {
  // Equipes previstas no campeonato em uso (num_equipes)
  var campeonato = buscarPorId(campeonatos, "id_campeonato", ativo);
  var equipesPrevistas = (campeonato === null) ? 0 : campeonato.num_equipes;

  var passos = [
    { pagina: "campeonatos", titulo: "Cadastrar campeonato", info: campeonatos.length + " cadastrado(s)", feito: campeonatos.length > 0 },
    { pagina: "equipes", titulo: "Cadastrar equipes", info: equipes.length + " de " + equipesPrevistas + " previstas", feito: equipesPrevistas > 0 && equipes.length >= equipesPrevistas },
    { pagina: "jogadores", titulo: "Cadastrar jogadores", info: jogadores.length + " de " + somarMembros() + " membros", feito: equipes.length > 0 && jogadores.length >= somarMembros() },
    { pagina: "partidas", titulo: "Cadastrar partidas", info: partidas.length + " cadastrada(s)", feito: partidas.length > 0 },
    { pagina: "resultados", titulo: "Cadastrar resultados", info: resultados.length + " de " + partidas.length + " partidas", feito: partidas.length > 0 && resultados.length >= partidas.length }
  ];

  var html = "";
  var achouAtual = false;
  for (var p = 0; p < passos.length; p++) {
    var classe = "passo";
    if (passos[p].feito) {
      classe += " feito";
    } else if (!achouAtual) {
      classe += " atual";        // o primeiro passo que falta fica destacado
      achouAtual = true;
    }
    html += "<a class='" + classe + "' href='#" + passos[p].pagina + "'>" +
            "<i>" + (p + 1) + "</i><b>" + passos[p].titulo + "</b><small>" + passos[p].info + "</small></a>";
  }
  pegar("passos").innerHTML = html;
}

// Pódio com os 3 primeiros da classificação (só aparece depois do primeiro resultado)
function desenharPodio() {
  if (classificacao.length === 0) {
    pegar("podio").innerHTML = "";
    return;
  }
  var ordem = [1, 0, 2];            // 2º lugar, 1º lugar, 3º lugar (o 1º fica no meio)
  var html = "";
  for (var i = 0; i < ordem.length; i++) {
    var posicao = ordem[i];
    if (posicao < classificacao.length) {
      html += "<div class='podio-item lugar" + (posicao + 1) + "'><small>" + (posicao + 1) + "º lugar</small>" +
              "<b>" + proteger(nomeDaEquipe(classificacao[posicao].id_equipe)) + "</b>" +
              "<span>" + classificacao[posicao].pontuacao + " pts</span></div>";
    }
  }
  pegar("podio").innerHTML = html;
}

// Texto no rodapé do menu lateral
function desenharAviso() {
  pegar("aviso-banco").textContent = conectado
    ? "Banco esports | conectado (MySQL via servidor.c)"
    : "Banco esports | sem conexão com o servidor.c";
}

// Redesenha tudo (chamada depois de cada ação)
function desenharTudo() {
  desenharAviso();
  desenharDashboard();
  desenharPassos();
  desenharPodio();
  desenharTabelas();
  desenharSelects();
}

/* ---------- 6. FORMULÁRIOS ----------
   Todos seguem o mesmo caminho: lê os campos -> pede ao servidor -> relê o banco -> mostra mensagem */

// "acao" é uma Promise (o pedido ao servidor). Se deu certo: mensagem verde, limpa o formulário
// e relê tudo do banco. Se o C recusou: mostra a mensagem de erro dele.
function finalizar(formulario, acao, mensagemSucesso) {
  var botoes = formulario === null ? [] : formulario.querySelectorAll("button");
  for (var b = 0; b < botoes.length; b++) { botoes[b].disabled = true; }

  acao.then(function () {
    return recarregar();
  }).then(function () {
    mostrarMensagem(mensagemSucesso, false);
    if (formulario !== null) { formulario.reset(); }
  }).catch(function (erro) {
    mostrarMensagem(erro.message, true);
  }).then(function () {
    for (var b = 0; b < botoes.length; b++) { botoes[b].disabled = false; }
    desenharTudo();
  });
}

pegar("form-campeonato").onsubmit = function (evento) {
  evento.preventDefault();                 // impede a página de recarregar
  var campos = evento.target.elements;
  var novo = {
    nome: campos.nome.value.trim(),
    jogo: campos.jogo.value.trim(),
    data_inicio: campos.data_inicio.value,
    tempo_duracao: campos.tempo_duracao.value.trim(),
    num_equipes: campos.num_equipes.value
  };
  finalizar(evento.target, cadastrarCampeonato(novo), "Campeonato cadastrado e salvo! Ele já está em uso.");
};

pegar("form-equipe").onsubmit = function (evento) {
  evento.preventDefault();
  var campos = evento.target.elements;
  var nova = {
    nome_equipe: campos.nome_equipe.value.trim(),
    qntd_membros: campos.qntd_membros.value
  };
  finalizar(evento.target, cadastrarEquipe(nova), "Equipe cadastrada e salva!");
};

pegar("form-jogador").onsubmit = function (evento) {
  evento.preventDefault();
  var campos = evento.target.elements;
  var novo = {
    id_equipe: campos.id_equipe.value,
    nome_jogador: campos.nome_jogador.value.trim(),
    idade: campos.idade.value,
    email: campos.email.value.trim(),
    telefone: campos.telefone.value.trim()
  };
  finalizar(evento.target, cadastrarJogador(novo), "Jogador cadastrado e salvo!");
};

pegar("form-partida").onsubmit = function (evento) {
  evento.preventDefault();
  var campos = evento.target.elements;
  var nova = {
    data_partida: campos.data_partida.value,
    horario: campos.horario.value,
    tempo_duracao: campos.tempo_duracao.value.trim(),
    equipe_1: campos.equipe_1.value,
    equipe_2: campos.equipe_2.value
  };
  finalizar(evento.target, cadastrarPartida(nova), "Partida cadastrada e salva!");
};

pegar("form-resultado").onsubmit = function (evento) {
  evento.preventDefault();
  var campos = evento.target.elements;
  var novo = {
    id_partida: campos.id_partida.value,
    pont_eq1: campos.pont_eq1.value,
    pont_eq2: campos.pont_eq2.value
  };
  finalizar(evento.target, cadastrarResultado(novo), "Resultado cadastrado e salvo! Classificação atualizada.");
};

// Mostra o resultado de uma busca de equipe (Buscar_Equipe_Pelo_Nome / Buscar_Equipe_Pelo_ID)
function mostrarBusca(idTexto, promessa) {
  promessa.then(function (json) {
    pegar(idTexto).textContent = json.encontrada
      ? "Equipe encontrada: " + json.equipe.nome_equipe + " | ID " + json.equipe.id_equipe
      : "Equipe não encontrada.";
  }).catch(function (erro) {
    pegar(idTexto).textContent = erro.message;
  }).then(desenharAviso);
}

// Busca de equipe pelo nome (busca linear, feita pelo C)
pegar("form-busca").onsubmit = function (evento) {
  evento.preventDefault();
  mostrarBusca("busca-resultado", buscarEquipe({ nome: pegar("busca-nome").value.trim() }));
};

// Busca de equipe pelo ID (busca binária, feita pelo C)
pegar("form-busca-id").onsubmit = function (evento) {
  evento.preventDefault();
  mostrarBusca("busca-id-resultado", buscarEquipe({ id: pegar("busca-id").value }));
};

// Alterar o nome de uma equipe
pegar("form-renomear").onsubmit = function (evento) {
  evento.preventDefault();
  var campos = evento.target.elements;
  finalizar(evento.target, atualizarNomeEquipe(campos.id_equipe.value, campos.novo_nome.value.trim()), "Nome da equipe atualizado com sucesso!");
};

// Trocar o campeonato em uso (Acessar campeonato existente)
pegar("select-ativo").onchange = function () {
  finalizar(null, acessarCampeonato(Number(pegar("select-ativo").value)), "Campeonato encontrado! Dados recuperados do banco.");
};

// Botões dentro das tabelas (Acessar campeonato e Excluir jogador)
document.onclick = function (evento) {
  var botao = evento.target.closest("button[data-acao]");
  if (botao === null) { return; }

  var acao = botao.getAttribute("data-acao");
  var id = Number(botao.getAttribute("data-id"));

  if (acao === "acessar") {
    finalizar(null, acessarCampeonato(id), "Campeonato encontrado! Dados recuperados do banco.");
  }

  if (acao === "excluir-jogador") {
    var jogador = buscarPorId(jogadores, "id_jogador", id);
    if (jogador !== null && confirm("Excluir o jogador " + jogador.nome_jogador + "?")) {
      finalizar(null, excluirJogador(id), "Jogador excluído com sucesso!");
    }
  }
};

/* ---------- 7. NAVEGAÇÃO ----------
   O menu muda o endereço (#equipes, #partidas...). Aqui mostramos só a página escolhida. */
var paginas = ["dashboard", "campeonatos", "equipes", "jogadores", "partidas", "resultados", "classificacao"];

// Texto de apoio mostrado embaixo do título de cada página
var descricoes = {
  dashboard: "Resumo do campeonato em uso.",
  campeonatos: "Crie um campeonato novo ou acesse um que já está no banco.",
  equipes: "Cadastre as equipes, busque por nome ou ID e altere nomes.",
  jogadores: "Cada equipe recebe a quantidade de jogadores que foi informada.",
  partidas: "Escolha data, horário e as duas equipes do confronto.",
  resultados: "Informe os pontos de cada equipe em uma partida.",
  classificacao: "Cada equipe soma os pontos que fez em todas as partidas."
};

function mostrarPagina() {
  var atual = location.hash.replace("#", "");
  if (paginas.indexOf(atual) === -1) { atual = "dashboard"; }   // endereço desconhecido -> dashboard

  for (var i = 0; i < paginas.length; i++) {
    pegar(paginas[i]).classList.toggle("ativa", paginas[i] === atual);
  }
  var links = document.querySelectorAll(".sidebar a");
  for (var j = 0; j < links.length; j++) {
    var ehAtual = links[j].getAttribute("href") === "#" + atual;
    links[j].classList.toggle("ativo", ehAtual);
    if (ehAtual) { pegar("titulo").textContent = links[j].textContent.trim(); }
  }
  pegar("subtitulo").textContent = descricoes[atual];
  pegar("sidebar").classList.remove("aberto");   // no celular/tablet, fecha o menu ao escolher
  pegar("fundo-menu").classList.remove("aberto");
  window.scrollTo(0, 0);
}
window.onhashchange = mostrarPagina;

// Botão ☰ (celular e tablet)
pegar("botao-menu").onclick = function () {
  pegar("sidebar").classList.toggle("aberto");
  pegar("fundo-menu").classList.toggle("aberto");
};
// Clicar no fundo escuro fecha o menu
pegar("fundo-menu").onclick = function () {
  pegar("sidebar").classList.remove("aberto");
  pegar("fundo-menu").classList.remove("aberto");
};

/* ---------- 8. INÍCIO ---------- */
ativo = lerAtivoGuardado();
mostrarPagina();
desenharTudo();                 // primeiro desenho (vazio) enquanto o banco responde

recarregar().catch(function (erro) {
  mostrarMensagem(erro.message, true);
}).then(desenharTudo);
