function getGameStats(context) {
  var request = getRequest(context);
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

function getGameStatusMessage(context) {
  var stats = getGameStats(context);
  
  if (stats.gameOver) {
    return "Вы уже выиграли эту игру! Скажите 'новая игра', чтобы начать заново.";
  } 
  
  if (!stats.guessCount) {
    return "Вы еще не назвали ни одного слова. Чтобы угадать слово, скажите 'слово [ваше слово]'.";
  }
  
  // Get best ranking word
  var request = getRequest(context);
  var closestWord = get_closest_word(request);
  var bestRank = stats.bestRank;
  
  var message = "Вы проверили " + stats.guessCount + " слов. ";
  
  if (closestWord) {
    message += "Ваше ближайшее слово к загаданному: '" + closestWord.value + "' с рангом " + closestWord.number + ". ";
  }
  
  message += getEncouragingMessage(stats);
  
  return message;
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
