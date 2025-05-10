theme: /

    state: GetHint
        q!: [дай|скажи|можешь дать|хочу] [мне] [пожалуйста] ~подсказка
        q!: ~подсказать [мне] [пожалуйста]
        q!: ~помочь [мне] [пожалуйста]
        q!: [я] [не] ~знать [что|как] [дальше] ~делать

        script:
            log('getHint: context: ' + JSON.stringify($context));
            var request = get_request($context);
            var gameState = get_game_state(request);
            var closestWord = get_closest_word(request);

            if (is_game_over(request)) {
                $reactions.answer("Игра уже завершена. Скажите 'новая игра', чтобы начать заново.");
            } else if (!closestWord) {
                provideHint($context);
                $reactions.answer("Попробуйте назвать любое слово, чтобы начать. Я помогу вам определить, насколько вы близки к разгадке.");
            } else {
                provideHint($context);
                $reactions.answer("Самое близкое слово к загаданному - \"" + closestWord.text + "\". Попробуйте подумать о словах, связанных с этим понятием!");
            }

            addSuggestions(["Правила", "Попробовать слово", "Новая игра"], $context);
