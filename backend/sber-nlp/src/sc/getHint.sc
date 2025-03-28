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
            
            if (is_game_over(request)) {
                $reactions.answer("Игра уже завершена. Скажите 'новая игра', чтобы начать заново.");
            } else {
                provideHint($context);
                $reactions.answer("Подумайте о слове, связанном с самым близким словом в списке!");
            }