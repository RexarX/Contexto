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
require: sc/help.sc

patterns:
    $AnyText = $nonEmptyGarbage
    $String = $nonEmptyGarbage
    $Number = $nonEmptyGarbage

theme: /
    state: Start
        q!: $regex</start>
        q!: (запусти|открой|вруби) [игру] [в] contexto
        a: Начинаем игру Contexto! Чтобы узнать, как играть, скажите "помощь".

        script:
            addSuggestions(["Помощь", "Правила", "Новая игра"], $context);

    state: StartGame
        q!: (начать|начни|начинай|давай|поехали|старт|стартуй) [новую|новый] [игру|раунд]
        q!: начать игру

        script:
            startGame($context);

        a: Игра началась!

        script:
            addSuggestions(["Правила", "Новая игра", "Сдаюсь"], $context);

    state: GameStatus
        q!: [как] [мои|у меня] [дела|результаты|прогресс]
        q!: [сколько] [я] [уже] (угадал|отгадал|назвал|проверил) [слов]

        script:
            var stats = getGameStats($context);

            if (stats.gameOver) {
                $reactions.answer("Вы уже выиграли эту игру! Скажите 'новая игра', чтобы начать заново.");
            } else if (stats.guessCount == 0) {
                $reactions.answer("Вы еще не назвали ни одного слова. Чтобы угадать слово, скажите 'слово [ваше слово]'.");
            } else {
                var message = getEncouragingMessage(stats);
                $reactions.answer("Вы уже проверили " + stats.guessCount + " слов. " + message);
            }

            addSuggestions(["Помощь", "Правила", "Новая игра"], $context);

    state: Fallback
        event!: noMatch

        script:
            log('entryPoint: Fallback: context: ' + JSON.stringify($context));
            var userInput = $request.query;

            // Only process non-empty inputs
            if (userInput && userInput.length > 1) {
                // Now we don't try to process every input as a word guess
                $reactions.answer("Я не понимаю эту команду. Скажите 'помощь', чтобы узнать доступные команды.");
            } else {
                // Very short or empty input
                $reactions.answer("Я не понимаю. Попробуйте угадать слово, сказав 'слово [ваше слово]' или скажите 'помощь'.");
            }

            addSuggestions(["Помощь", "Правила", "Новая игра"], $context);
