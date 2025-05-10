theme: /

    state: GuessWord
        q!: {(это|я думаю|я считаю|попробую|давай|может|может быть|возможно) [слово] * $Number::anyText}
        q!: {(это|я думаю|я считаю|попробую|давай|может|может быть|возможно) [слово] * $String::anyText}
        q!: * {$Number::anyText} *
        q!: * {$String::anyText} *

        script:
            log('guessWord: context: ' + JSON.stringify($context));
            var request = get_request($context);
            var gameState = get_game_state(request);
            var ignoredWords = get_ignored_words(request);
            var guess = $request.query;

            if (is_game_over(request)) {
                $reactions.answer("Игра уже завершена. Скажите 'новая игра', чтобы начать заново.");
                return;
            }

            // Ensure the word is processed correctly
            if (guess) {
                guess = guess.trim();
                guessWord(guess, $context);
                $reactions.answer("Проверяю слово \"" + guess + "\"");
            } else {
                $reactions.answer("Не расслышал слово, повторите пожалуйста.");
            }
