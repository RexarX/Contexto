theme: /

    state: GuessWord
        q!: * (слово|это|угадай|проверь|это слово|угадай слово|проверь слово) $GuessWord::word *

        script:
            log('guessWord: context: ' + JSON.stringify($context));
            var request = getRequest($context);

            if (is_game_over(request)) {
                $reactions.answer("Игра уже завершена. Скажите 'новая игра', чтобы начать заново.");
                return;
            }

            var guess = $parseTree._word;
            if (guess) {
                guess = guess.trim();
                guessWord(guess, $context);
            } else {
                $reactions.answer("Не совсем понятно, повторите пожалуйста.");
            }

    state: UnknownWord
        event!: unknown_word
        script:
            var eventData = getEventData($context);
            if (eventData.value) {
                $reactions.answer("Я не знаю этого слова. Попробуйте другое.");
            }
