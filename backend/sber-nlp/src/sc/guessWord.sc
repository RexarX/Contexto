theme: /

    state: GuessWord
        q!: * {(слово|это|угадай|проверь|это слово|угадай слово|проверь слово) $String::anyText}

        script:
            log('guessWord: context: ' + JSON.stringify($context));
            var request = get_request($context);

            if (is_game_over(request)) {
                $reactions.answer("Игра уже завершена. Скажите 'новая игра', чтобы начать заново.");
                return;
            }

            // Extract the word from the phrase
            var fullText = $request.query;
            var guess = extractWordFromPhrase(fullText);

            if (guess) {
                guess = guess.trim();
                guessWord(guess, $context);
            } else {
                $reactions.answer("Не расслышал слово, повторите пожалуйста. Скажите 'слово [ваше слово]'.");
            }

    state: InvalidGuessFormat
        q!: $String

        script:
            var text = $request.query;
            if (text && text.length > 1 && !isSystemPhrase(text)) {
                $reactions.answer("Чтобы угадать слово, скажите 'слово [ваше слово]' или 'проверь [ваше слово]'.");
                addSuggestions(["Помощь", "Правила", "Новая игра"], $context);
            } else {
                $reactions.transition("/Fallback");
            }
