function startGame(context) {
  addAction(
    {
      type: "start_game"
    },
    context
  );
}

function newGame(context) {
  addAction(
    {
      type: "new_game"
    },
    context
  );
}

function guessWord(word, context) {
  addAction(
    {
      type: "guess_word",
      word: word
    },
    context
  );
}

function giveUp(context) {
  addAction(
    {
      type: "give_up"
    },
    context
  );
}

function extractWordFromPhrase(phrase) {
  if (!phrase) return null;

  phrase = phrase.toLowerCase();

  // Keywords that precede the word to guess
  var keywords = [
    "слово",
    "это",
    "угадай",
    "проверь",
    "это слово",
    "угадай слово",
    "проверь слово"
  ];

  for (var i = 0; i < keywords.length; i++) {
    var keyword = keywords[i];
    var index = phrase.indexOf(keyword);
    
    if (index !== -1) {
      // Extract the part after the keyword
      var remainingText = phrase.substring(index + keyword.length);
      return remainingText.trim();
    }
  }

  return null;
}
