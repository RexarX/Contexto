function guessWord(word, context) {
  addAction(
    {
      type: "guess_word",
      word: word
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

function provideHint(context) {
  addAction(
    {
      type: "get_hint"
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
