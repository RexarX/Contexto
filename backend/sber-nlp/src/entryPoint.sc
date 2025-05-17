require: slotfilling/slotFilling.sc
  module = sys.zb-common

require: js/actions.js
require: js/getters.js
require: js/reply.js
require: js/stats.js

require: sc/guessWord.sc
require: sc/newGame.sc
require: sc/giveUp.sc
require: sc/rules.sc

patterns:
    $AnyText = $nonEmptyGarbage
    $String = $nonEmptyGarbage
    $Number = $nonEmptyGarbage

theme: /
    state: Start
        q!: $regex</start>
        q!: (запусти|открой|вруби) [игру] [в] contexto
        a: Игра Contexto началась! Попробуйте угадать секретное слово.

        script:
            newGame($context);
            addSuggestions(["Правила", "Подсказка", "Новая игра"], $context);

    state: GameStatus
        q!: [как] [мои|у меня] [дела|результаты|прогресс]
        q!: [сколько] [я] [уже] (угадал|отгадал|назвал|проверил) [слов]

        script:
            var stats = getGameStats($context);

            if (stats.gameOver) {
                $reactions.answer("Вы уже выиграли эту игру! Скажите 'новая игра', чтобы начать заново.");
            } else if (stats.guessCount == 0) {
                $reactions.answer("Вы еще не назвали ни одного слова. Просто скажите любое слово, чтобы начать игру.");
            } else {
                var message = getEncouragingMessage(stats);
                $reactions.answer("Вы уже проверили " + stats.guessCount + " слов. " + message);
            }

            addSuggestions(["Правила", "Новая игра"], $context);

    state: Fallback
        event!: noMatch

        script:
            log('entryPoint: Fallback: context: ' + JSON.stringify($context));
            var userInput = $request.query;

            // Only process non-empty inputs
            if (userInput && userInput.length > 1) {
                // Check if it's a system phrase
                if (isSystemPhrase(userInput)) {
                    $reactions.answer("Не совсем понимаю. Попробуйте назвать слово для проверки или скажите 'правила' для информации об игре.");
                    addSuggestions(["Правила", "Подсказка", "Новая игра"], $context);
                } else {
                    // Process as word guess - only show the processing message
                    guessWord(userInput, $context);
                }
            } else {
                // Very short or empty input
                $reactions.answer("Я не понимаю. Попробуйте угадать слово или скажите \"новая игра\".");
                addSuggestions(["Правила", "Подсказка", "Новая игра"], $context);
            }
