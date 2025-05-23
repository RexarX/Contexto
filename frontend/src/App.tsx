import React from "react";
import {
  createAssistant,
  createSmartappDebugger,
  AssistantAppState,
} from "@salutejs/client";
import { Container } from "@salutejs/plasma-ui";

import { ContextoGame } from "./pages/ContextoGame";
import { WelcomeScreen } from "./components/WelcomeScreen";
import {
  AssistantAction,
  AssistantEvent,
  GameState,
  AssistantEventStart,
} from "./types";
import logger from "./logger-init";

interface AppState {
  gameState: GameState;
  feedbackMessage?: {
    text: string;
    type: "feedback" | "error" | "success";
  };
  gameInitialized: boolean;
  showWelcomeScreen: boolean;
}

const API_BASE_URL = (() => {
  const envUrl = import.meta.env.VITE_API_BASE_URL;
  if (envUrl) return envUrl;

  // For production when running on the same domain, use relative URLs to avoid CORS issues
  if (window.location.hostname !== "localhost") {
    return ""; // Use relative URLs in production
  }

  // For local development
  return `${window.location.protocol}//${window.location.hostname}:8080`;
})();

const initializeAssistant = (getState: () => AssistantAppState) => {
  try {
    if (import.meta.env.DEV) {
      return createSmartappDebugger({
        token: import.meta.env.VITE_APP_TOKEN || "",
        initPhrase: `Запусти ${import.meta.env.VITE_APP_SMARTAPP}`,
        getState,
        nativePanel: {
          defaultText: "Попробуй угадать слово",
          screenshotMode: false,
          tabIndex: -1,
        },
      });
    } else {
      return createAssistant({ getState });
    }
  } catch (error) {
    logger.warn("Failed to initialize assistant:", error);
    return null;
  }
};

export class App extends React.Component<Record<string, never>, AppState> {
  private assistant: ReturnType<typeof createAssistant> | null = null;
  private gameInitializing = false;

  constructor(props: Record<string, never>) {
    super(props);
    logger.log("constructor");

    this.state = {
      gameState: {
        guessedWords: [],
        gameOver: false,
      },
      gameInitialized: false,
      showWelcomeScreen: true,
    };

    // Try to initialize assistant, but don't fail if it doesn't work
    try {
      this.assistant = initializeAssistant(() => this.getStateForAssistant());

      if (this.assistant) {
        this.assistant.on("data", this.handleAssistantData);
        this.assistant.on("start", this.handleAssistantStart);
        this.assistant.on("command", (event) => {
          logger.log(`assistant.on(command)`, event);
        });
        this.assistant.on("error", (event) => {
          logger.log(`assistant.on(error)`, event);
        });
      }
    } catch (error) {
      logger.warn("Error setting up assistant:", error);
      this.assistant = null;
    }
  }

  startGame = async (): Promise<void> => {
    this.setState({ showWelcomeScreen: false }, () => {
      this.startNewGame();
    });
  };

  startNewGame = async (): Promise<void> => {
    if (this.gameInitializing) return;

    this.gameInitializing = true;
    try {
      this.setState({
        gameState: {
          guessedWords: [],
          gameOver: false,
          userGaveUp: false,
        },
        feedbackMessage: undefined,
        gameInitialized: true,
      });

      logger.log(`Fetching from: ${API_BASE_URL}/api/new-game`);

      const response = await fetch(`${API_BASE_URL}/api/new-game`, {
        method: "POST",
        credentials: "include",
        headers: {
          "Content-Type": "application/json",
        },
      });

      const data = await response.json();

      if (data.error) {
        logger.error("Error starting new game:", data.error);
        this.setState({
          feedbackMessage: {
            text: `Ошибка при создании новой игры: ${data.error}`,
            type: "error",
          },
        });
        return;
      }

      // Store session_id in localStorage as fallback
      if (data.session_id) {
        localStorage.setItem("Contexto_session_id", data.session_id);
      }

      logger.log("New game started with session:", data.session_id);
    } catch (error) {
      logger.error("Error starting new game:", error);
      this.setState({
        feedbackMessage: {
          text: "Ошибка при создании новой игры. Пожалуйста, проверьте подключение к серверу.",
          type: "error",
        },
      });
    } finally {
      this.gameInitializing = false;
    }
  };

  async makeGuess(action: AssistantAction): Promise<void> {
    logger.log("makeGuess", action);
    if (action.word && !this.state.gameState.gameOver) {
      if (action.word) {
        return this.guessWord(action.word);
      }
    }
    return Promise.resolve();
  }

  giveUp = async (): Promise<void> => {
    try {
      const storedSessionId = localStorage.getItem("Contexto_session_id");

      const response = await fetch(`${API_BASE_URL}/api/give-up`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          session_id: storedSessionId,
        }),
        credentials: "include",
      });

      if (!response.ok) {
        throw new Error(`HTTP error: ${response.status}`);
      }

      const data = await response.json();

      if (data.error) {
        this.setState({ feedbackMessage: { text: data.error, type: "error" } });
        return;
      }

      if (data.target_word) {
        this.setState({
          gameState: {
            ...this.state.gameState,
            gameOver: true,
            targetWord: data.target_word,
            userGaveUp: true,
          },
        });

        this.sendActionValue(
          "give_up",
          `Загаданное слово было: ${data.target_word}`,
        );
      }
    } catch (error) {
      logger.error("Error giving up:", error);
      this.setState({
        feedbackMessage: {
          text: "Произошла ошибка при получении ответа",
          type: "error",
        },
      });
    }
  };

  private guessWord = async (word: string): Promise<void> => {
    word = word.trim().toLocaleLowerCase();
    logger.log("guessWord", word);
    try {
      const storedSessionId = localStorage.getItem("Contexto_session_id");

      const response = await fetch(`${API_BASE_URL}/api/guess`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          word,
          session_id: storedSessionId,
        }),
        credentials: "include",
      });

      let data;
      try {
        data = await response.json();
      } catch (e) {
        logger.error("Failed to parse response:", e);
        data = { error: "Failed to parse response" };
      }

      if (!response.ok || data.error) {
        // Specific handling for unknown words
        if (
          data.error &&
          (data.error.includes("Invalid word") ||
            data.error.includes("word not found"))
        ) {
          const errorMessage = "Неизвестное слово";

          // Update UI
          this.setState({
            feedbackMessage: { text: errorMessage, type: "error" },
          });

          // Send to assistant using the correct format
          this.sendActionValue("unknown_word", word);

          return;
        }

        // Handle other errors
        const errorMessage = data.error || `HTTP error: ${response.status}`;
        this.sendActionValue("error", errorMessage);
        this.setState({
          feedbackMessage: { text: errorMessage, type: "error" },
        });
        return;
      }

      logger.log("Backend response for guess:", data);

      // Ensure rank is a number
      const rankAsNumber = parseInt(String(data.rank), 10);
      if (isNaN(rankAsNumber)) {
        logger.error("Received non-numeric rank from backend:", data.rank);
        // Handle error or set a default rank
      }

      // Update the game state with the new guess
      const newGuessedWords = [...this.state.gameState.guessedWords];

      // Check if the word already exists in the list (by text)
      const wordExists = newGuessedWords.some(
        (existingWord) =>
          existingWord.text.toLowerCase() === word.toLowerCase(),
      );

      // Only add if the word doesn't already exist
      if (!wordExists) {
        const newWord = {
          id: crypto.randomUUID(), // Generate a unique ID
          text: word,
          rank: isNaN(rankAsNumber) ? -1 : rankAsNumber,
          isNew: true, // Mark this as the new word
        };

        // First, remove the isNew flag from any existing words
        newGuessedWords.forEach((existingWord) => {
          if (existingWord.isNew) delete existingWord.isNew;
        });

        // Then add the new word with the isNew flag
        newGuessedWords.push(newWord);
      }

      // Sort by rank (similarity to target word)
      newGuessedWords.sort((a, b) => a.rank - b.rank);

      logger.log("New guess with rank:", data.rank);

      // Check if the guess is correct (rank 1)
      const isCorrect = data.rank === 1;

      this.setState({
        gameState: {
          ...this.state.gameState,
          guessedWords: newGuessedWords,
          gameOver: isCorrect,
          targetWord: isCorrect ? word : this.state.gameState.targetWord,
        },
      });

      if (isCorrect) {
        const successMessage = "Поздравляем! Вы угадали секретное слово!";
        this.sendActionValue("success", successMessage);
      }
    } catch (error) {
      logger.error("Error making guess:", error);
      const errorMessage = "Произошла ошибка при проверке слова";
      this.sendActionValue("error", errorMessage);
      this.setState({ feedbackMessage: { text: errorMessage, type: "error" } });
    }
  };

  private getStateForAssistant(): AssistantAppState {
    logger.log("getStateForAssistant: this.state:", this.state);

    const state: AssistantAppState = {
      item_selector: {
        items: this.state.gameState.guessedWords.map(({ id, text, rank }) => ({
          number: rank,
          id,
          text,
        })),
        ignored_words: [
          "угадай",
          "попробуй",
          "может",
          "быть",
          "попробуем",
          "новая",
          "игра",
          "начать",
          "начни",
          "начнем",
        ],
      },
      game_state: this.state.gameState,
    };

    logger.log("getStateForAssistant: state:", state);
    return state;
  }

  private dispatchAssistantAction(action: AssistantAction): Promise<void> {
    logger.log("dispatchAssistantAction", action);
    if (action) {
      logger.log(
        "dispatchAssistantAction: Received action:",
        JSON.stringify(action, null, 2),
      );
      switch (action.type) {
        case "start_game":
          return this.startGame();

        case "new_game":
          return this.startNewGame();

        case "guess_word":
          logger.log(
            "dispatchAssistantAction: case 'guess_word', action.word:",
            action.word,
          );
          if (action.word) {
            logger.log("Making guess with word:", action.word);
            return this.makeGuess(action);
          } else {
            logger.warn(
              "dispatchAssistantAction: 'guess_word' action received without a word.",
            );
            this.sendActionValue(
              "error",
              "Не расслышал слово, повторите пожалуйста.",
            );
          }
          break;

        case "give_up":
          return this.giveUp();

        case "get_hint":
          return Promise.resolve();

        case "log":
          logger.log(action.word);
          return Promise.resolve();

        default:
          logger.error(`Unhandled action type: ${action.type}`);
          return Promise.resolve();
      }
    }
    return Promise.resolve();
  }

  private handleAssistantData = (event: AssistantEvent) => {
    logger.log(`assistant.on(data)`, event);
    if (event.type === "character") {
      logger.log(`assistant.on(data): character: "${event.character?.id}"`);
    } else if (event.type === "insets") {
      logger.log(`assistant.on(data): insets`);
    } else if (event.action) {
      this.dispatchAssistantAction(event.action);
    } else if (
      event.type === "smart_app_data" &&
      event.smart_app_data?.action
    ) {
      // Handle actions that come through smart_app_data
      this.dispatchAssistantAction(event.smart_app_data.action);
    }
  };

  private handleAssistantStart = ((event: AssistantEventStart) => {
    logger.log(`assistant.on(start)`, event);

    if (this.assistant) {
      try {
        const initialData = this.assistant.getInitialData();
        logger.log(`assistant initial data:`, initialData);
      } catch (error) {
        logger.warn("Could not get initial data:", error);
      }
    }
  }) as any;

  private sendActionValue(action_id: string, value: string): void {
    if (!this.assistant) {
      logger.log("Assistant not initialized, skipping sendActionValue");
      return;
    }

    try {
      const data = {
        action: {
          action_id: action_id,
          parameters: {
            value: value,
          },
        },
      };

      logger.log(`Sending action to assistant: ${action_id}, value: ${value}`);

      const unsubscribe = this.assistant.sendData(
        data,
        (assistantData: any) => {
          const { type, payload } = assistantData || {};
          logger.log("sendData onData:", type, payload);
          unsubscribe(); // Properly unsubscribe after handling the response
        },
      );
    } catch (error) {
      logger.warn("Error sending action to assistant:", error);
    }
  }

  render(): React.ReactNode {
    logger.log("render");

    // If showing welcome screen
    if (this.state.showWelcomeScreen) {
      return (
        <div
          style={{
            paddingBottom: "180px",
            minHeight: "100vh",
          }}
        >
          <Container>
            <WelcomeScreen onStartGame={this.startGame} />
          </Container>
        </div>
      );
    }

    return (
      <div
        style={{
          paddingBottom: "180px",
          minHeight: "100vh",
        }}
      >
        <Container>
          <ContextoGame
            gameState={this.state.gameState}
            onGuess={this.guessWord}
            onNewGame={this.startNewGame}
            onGiveUp={this.giveUp}
            feedbackMessage={this.state.feedbackMessage}
          />
        </Container>
      </div>
    );
  }
}
