function get_request(context) {
  if (context && context.request) return context.request.rawRequest;
  return {};
}

function get_server_action(request) {
  if (
    request &&
    request.payload &&
    request.payload.data &&
    request.payload.data.server_action
  ) {
    return request.payload.data.server_action;
  }
  return {};
}

function get_game_state(request) {
  if (
    request &&
    request.payload &&
    request.payload.meta &&
    request.payload.meta.current_app &&
    request.payload.meta.current_app.state &&
    request.payload.meta.current_app.state.game_state
  ) {
    return request.payload.meta.current_app.state.game_state;
  }
  return {};
}

function get_items(request) {
  if (
    request &&
    request.payload &&
    request.payload.meta &&
    request.payload.meta.current_app &&
    request.payload.meta.current_app.state &&
    request.payload.meta.current_app.state.item_selector
  ) {
    return request.payload.meta.current_app.state.item_selector.items;
  }
  return [];
}

function get_ignored_words(request) {
  if (
    request &&
    request.payload &&
    request.payload.meta &&
    request.payload.meta.current_app &&
    request.payload.meta.current_app.state &&
    request.payload.meta.current_app.state.item_selector
  ) {
    return (
      request.payload.meta.current_app.state.item_selector.ignored_words || []
    );
  }
  return [];
}

function get_last_guesses(request, count) {
  count = count || 3;
  var items = get_items(request);
  if (items && items.length > 0) {
    return items.slice(0, Math.min(count, items.length));
  }
  return [];
}

function get_closest_word(request) {
  var items = get_items(request);
  if (items && items.length > 0) {
    // Sort by rank and get the closest one
    items.sort(function (a, b) {
      return a.number - b.number;
    });
    return items[0];
  }
  return null;
}

function is_game_over(request) {
  var gameState = get_game_state(request);
  return gameState.gameOver === true;
}

function isSystemPhrase(text) {
  if (!text) return true;

  var lowercaseText = text.toLowerCase();
  var systemPhrases = [
    "что ты умеешь",
    "помощь",
    "выход",
    "закрыть",
    "стоп",
    "спасибо",
    "да",
    "нет",
    "отмена",
  ];

  for (var i = 0; i < systemPhrases.length; i++) {
    if (lowercaseText.indexOf(systemPhrases[i]) !== -1) {
      return true;
    }
  }
  return false;
}
