function getGameStats(context) {
  var request = get_request(context);
  var items = get_items(request);
  var gameState = get_game_state(request);

  var bestRank = null;
  if (items && items.length > 0) {
    var numbers = [];
    for (var i = 0; i < items.length; i++) {
      numbers.push(items[i].number);
    }
    bestRank = Math.min.apply(null, numbers);
  }

  return {
    guessCount: items.length,
    gameOver: gameState.gameOver === true,
    bestRank: bestRank,
  };
}

function getEncouragingMessage(stats) {
  if (!stats.guessCount) {
    return "Начните игру, назвав любое слово!";
  }

  if (stats.bestRank <= 5) {
    return "Вы уже очень близко к загадке!";
  } else if (stats.bestRank <= 20) {
    return "Вы на правильном пути!";
  } else if (stats.bestRank <= 50) {
    return "Продолжайте думать, вы приближаетесь!";
  } else {
    return "Пробуйте разные слова, чтобы найти верное направление.";
  }
}
