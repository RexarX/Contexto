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

function logAction(message, context) {
  addAction(
    {
      type: "log",
      word: message
    },
    context
  );
}
