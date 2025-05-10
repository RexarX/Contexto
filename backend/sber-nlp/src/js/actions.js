function guessWord(word, context) {
  addAction(
    {
      type: "guess_word",
      word: word,
    },
    context,
  );
}

function newGame(context) {
  addAction(
    {
      type: "new_game",
    },
    context,
  );
}
