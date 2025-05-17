theme: /

    state: GuessWord
        q!: {(это|я думаю|я считаю|попробую|давай|может|может быть|возможно) [слово] * $Number::anyText}
        q!: {(это|я думаю|я считаю|попробую|давай|может|может быть|возможно) [слово] * $String::anyText}

        script:
            log('guessWord: context: ' + JSON.stringify($context));
            var request = get_request($context);

            if (is_game_over(request)) {
                $reactions.answer("Игра уже завершена. Скажите 'новая игра', чтобы начать заново.");
                return;
            }

            // Ensure the word is processed correctly
            var guess = $request.query;
            if (guess) {
                guess = guess.trim();
                guessWord(guess, $context);
            } else {
                $reactions.answer("Не расслышал слово, повторите пожалуйста.");
            }
