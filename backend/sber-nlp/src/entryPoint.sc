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
    $GameCommands = (новая игра|сдаюсь|правила|помощь|начать игру)
    $GuessWord = $nonEmptyGarbage

theme: /

    state: Start
        q!: $regex</start>
        q!: (запусти|открой|вруби) [игру] [в] (contexto|контексто)
        a: Привет! Добро пожаловать в игру "Контексто"! Это игра, в которой вам нужно угадать загаданное слово, опираясь на семантическую близость. Чтобы узнать больше о приложении, скажите "О приложении".

        script:
            addSuggestions(["О приложении", "Помощь", "Начать игру"], $context);

    state: AboutApp
        q!: (о приложении|что это|расскажи [больше]|зачем это)

        a: Контексто - это языковая игра, основанная на семантической близости слов. Я загадываю слово, а вы пытаетесь его угадать. После каждой попытки я показываю, насколько ваше слово близко к загаданному по смыслу. Чем меньше ранг (число от 1 до 1000), тем ближе слово к загаданному. Эта игра развивает языковое чутье и помогает изучать связи между словами.

        script:
            addSuggestions(["Помощь", "Правила","Начать игру"], $context);

    state: StartGame
        q!: (начать|начни|начинай|давай|поехали|старт|стартуй) [новую|новый] [игру|раунд]
        q!: начать игру

        a: Игра началась!

        script:
            startGame($context);
            $session.gameStarted = true;

            // Show rules on first game only
            if (!$session.rulesShown) {
                $reactions.answer("Подсказка: чтобы узнать правила игры, скажите 'правила'.");
                $session.rulesShown = true;
            }

            addSuggestions(["Правила", "Новая игра", "Сдаюсь"], $context);

    state: GameStatus
        q!: [как] [мои|у меня] [дела|результаты|прогресс]
        q!: [сколько] [я] [уже] (угадал|отгадал|назвал|проверил) [слов]
        q!: [какое] [моё|мое] [самое] близкое слово
        q!: [какой] [у меня] [лучший] результат

        script:
            var statusMessage = getGameStatusMessage($context);
            $reactions.answer(statusMessage);

            // Add appropriate suggestions based on game state
            var stats = getGameStats($context);
            if (stats.gameOver) {
                addSuggestions(["Новая игра", "Правила"], $context);
            } else {
                addSuggestions(["Помощь", "Правила", "Сдаюсь"], $context);
            }

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
