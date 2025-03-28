require: slotfilling/slotFilling.sc
  module = sys.zb-common
  
require: js/getters.js
require: js/reply.js
require: js/actions.js

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

    state: Fallback
        event!: noMatch
        script:
            log('entryPoint: Fallback: context: ' + JSON.stringify($context))
            var userInput = $request.query;
            
            if (userInput && userInput.length > 1) {
                guessWord(userInput, $context);
                $reactions.transition('/GuessWord');
                return;
            }
            
        random:
            a: Я не понимаю. Попробуйте угадать слово или скажите "новая игра".
            a: Не совсем понятно. Назовите слово, которое может быть загаданным, или скажите "правила" для информации об игре.
        script:
            addSuggestions(["Правила", "Подсказка", "Новая игра"], $context);