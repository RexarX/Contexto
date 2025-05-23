export interface Word {
  id: string;
  text: string;
  rank: number; // Position in similarity list
  isNew?: boolean;
}

export interface GameState {
  targetWord?: string;
  guessedWords: Word[];
  gameOver: boolean;
  userGaveUp?: boolean;
}

export interface AssistantState {
  item_selector: {
    items: Array<{
      number: number;
      id: string;
      text: string;
    }>;
    ignored_words: string[];
  };
  game_state?: GameState;
}

export interface AssistantAction {
  type: string;
  word?: string;
  id?: string;
  error_message?: string;
}

export interface AssistantEvent {
  type: string;
  character?: {
    id: string;
  };
  action?: AssistantAction;
  smart_app_data?: {
    action: AssistantAction;
    sdk_meta?: any;
  };
}

export interface AssistantSmartAppData<T = any> {
  type: string;
  payload?: T;
}

export interface AssistantSmartAppError {
  type: "error";
  error: {
    code: string;
    message: string;
  };
}

export type AssistantEventStart = {
  type: "start";
  payload?: any;
};

export type AssistantEventCharacter = {
  type: "character";
  character?: {
    id: string;
    [key: string]: any;
  };
};

export type AssistantEventInsets = {
  type: "insets";
  insets?: {
    [key: string]: any;
  };
};

export interface AssistantSendData {
  action: {
    action_id: string;
    parameters: {
      value: string;
    };
  };
}

export type AssistantSmartAppCallback = (
  data: AssistantSmartAppData | AssistantSmartAppError,
) => void;
