require: slotfilling/slotFilling.sc
  module = sys.zb-common

require: js/actions.js
require: js/getters.js
require: js/reply.js
require: js/stats.js

require: sc/guessWord.sc
require: sc/newGame.sc
require: sc/getHint.sc
require: sc/rules.sc

patterns:
    $AnyText = $nonEmptyGarbage
    $String = $nonEmptyGarbage
    $Number = $nonEmptyGarbage

theme: /
    state: Start
        q!: $regex</start>
        q!: (запусти|открой|вруби) [игру] [в] contexto
        a: Игра Contesto началась! Попробуйте угадать секретное слово.
        script:
            newGame($context);
            addSuggestions(["Правила", "Подсказка", "Новая игра"], $context);

    state: Rules
        q!: [расскажи|объясни] [мне] [пожалуйста] [~какой] ~правило [~игра]
        q!: как [в это] играть
        q!: что [мне] [нужно] [надо] делать
        q!: [я] не понимаю [как играть]

        random:
            a: В игре Contesto нужно угадать секретное слово. С каждой попыткой вы получаете подсказку - ваши слова отсортированы по схожести с секретным. Чем ближе слово к началу списка, тем оно ближе к секретному.
            a: Правила игры Contesto: нужно угадать секретное слово. Каждое слово, которое вы называете, появляется в списке и сортируется по схожести с загаданным словом. Чем выше в списке, тем ближе к разгадке!

    state: CatchWord
        q: $String
        go!: /GuessWord

    state: GameStatus
        q!: [как] [мои|у меня] [дела|результаты|прогресс]
        q!: [сколько] [я] [уже] (угадал|отгадал|назвал|проверил) [слов]

        script:
            var stats = getGameStats($context);
            var message = getEncouragingMessage(stats);

            if (stats.gameOver) {
                $reactions.answer("Вы уже выиграли эту игру! Скажите 'новая игра', чтобы начать заново.");
            } else if (stats.guessCount == 0) {
                $reactions.answer("Вы еще не назвали ни одного слова. Просто скажите любое слово, чтобы начать игру.");
            } else {
                $reactions.answer("Вы уже проверили " + stats.guessCount + " слов. " + message);
            }

            addSuggestions(["Правила", "Подсказка", "Новая игра"], $context);

    state: Fallback
        event!: noMatch
        script:
            log('entryPoint: Fallback: context: ' + JSON.stringify($context))
            var userInput = $request.query;

            if (userInput && userInput.length > 1) {
                // Skip certain system phrases
                if (is_system_phrase(userInput)) {
                    $reactions.answer("Не совсем понимаю. Попробуйте назвать слово для проверки или скажите 'правила' для информации об игре.");
                } else {
                    guessWord(userInput, $context);
                    $reactions.answer("Проверяю слово \"" + userInput + "\"...");
                    return;
                }
            }

        random:
            a: Я не понимаю. Попробуйте угадать слово или скажите "новая игра".
            a: Не совсем понятно. Назовите слово, которое может быть загаданным, или скажите "правила" для информации об игре.
        script:
            addSuggestions(["Правила", "Подсказка", "Новая игра"], $context);
