theme: /

    state: GiveUp
        q!: [я] (сдаюсь|сдаться|отказываюсь|отказаться|пас|без понятия|не знаю|подскажи ответ|скажи ответ|не могу угадать)
        q!: * {(я|хочу) (сдаюсь|сдаться)} *
        q!: * {(покажи|какое|какой) [правильный|верный] [ответ|слово]} *

        script:
            log('giveUp: context: ' + JSON.stringify($context));
            var request = get_request($context);

            if (is_game_over(request)) {
                $reactions.answer("Игра уже завершена. Скажите 'новая игра', чтобы начать заново.");
                return;
            }

            giveUp($context);

            $reactions.answer("Понимаю, иногда бывает трудно угадать. Показываю ответ.");
