import React from "react";
import {
  createAssistant,
  createSmartappDebugger,
  AssistantAppState,
} from "@salutejs/client";
import { Container } from "@salutejs/plasma-ui";

import { ContextoGame } from "./pages/ContextoGame";
import {
  AssistantAction,
  AssistantEvent,
  AssistantSendData,
  GameState,
  AssistantEventStart,
} from "./types";

interface AppState {
  gameState: GameState;
  feedbackMessage?: {
    text: string;
    type: "feedback" | "error" | "success";
  };
}

const API_BASE_URL =
  import.meta.env.VITE_API_BASE_URL ||
  (window.location.hostname === "localhost"
    ? "http://localhost:8081"
    : "http://backend:8080");

const initializeAssistant = (getState: () => AssistantAppState) => {
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
};

export class App extends React.Component<Record<string, never>, AppState> {
  private assistant: ReturnType<typeof createAssistant>;

  constructor(props: Record<string, never>) {
    super(props);
    console.log("constructor");

    this.state = {
      gameState: {
        guessedWords: [],
        gameOver: false,
      },
    };

    this.assistant = initializeAssistant(() => this.getStateForAssistant());

    this.assistant.on("data", (event: AssistantEvent) => {
      console.log(`assistant.on(data)`, event);
      if (event.type === "character") {
        console.log(`assistant.on(data): character: "${event.character?.id}"`);
      } else if (event.type === "insets") {
        console.log(`assistant.on(data): insets`);
      } else if (event.action) {
        this.dispatchAssistantAction(event.action);
      }
    });

    this.assistant.on("start", ((event: AssistantEventStart) => {
      const initialData = this.assistant.getInitialData();
      console.log(`assistant.on(start)`, event, initialData);

      // Start a new game when the app loads
      this.startNewGame();
    }) as any);

    this.assistant.on("command", (event) => {
      console.log(`assistant.on(command)`, event);
    });

    this.assistant.on("error", (event) => {
      console.log(`assistant.on(error)`, event);
    });

    this.assistant.on("tts", (event) => {
      console.log(`assistant.on(tts)`, event);
    });
  }

  getStateForAssistant(): AssistantAppState {
    console.log("getStateForAssistant: this.state:", this.state);

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

    console.log("getStateForAssistant: state:", state);
    return state;
  }

  dispatchAssistantAction(action: AssistantAction): Promise<void> {
    console.log("dispatchAssistantAction", action);
    if (action) {
      console.log(
        "dispatchAssistantAction: Received action:",
        JSON.stringify(action, null, 2),
      ); // Detailed log
      switch (action.type) {
        case "new_game":
          return this.startNewGame();

        case "guess_word":
          console.log(
            "dispatchAssistantAction: case 'guess_word', action.word:",
            action.word,
          );
          if (action.word) {
            console.log("Making guess with word:", action.word);
            return this.makeGuess(action);
          } else {
            console.warn(
              "dispatchAssistantAction: 'guess_word' action received without a word.",
            );
            // Optionally, provide feedback to the user via assistant.sendData if word is missing
            this.sendActionValue(
              "error",
              "Не расслышал слово, повторите пожалуйста.",
            );
          }
          break;

        case "get_hint":
          return Promise.resolve();

        default:
          console.error(`Unhandled action type: ${action.type}`);
          return Promise.resolve();
      }
    }
    return Promise.resolve();
  }

  startNewGame = async (): Promise<void> => {
    try {
      this.setState({
        gameState: {
          guessedWords: [],
          gameOver: false,
        },
        feedbackMessage: undefined,
      });

      const response = await fetch(`${API_BASE_URL}/api/new-game`, {
        method: "POST",
        credentials: "include",
        headers: {
          "Content-Type": "application/json",
        },
      });

      const data = await response.json();

      if (data.error) {
        console.error("Error starting new game:", data.error);
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

      console.log("New game started with session:", data.session_id);
    } catch (error) {
      console.error("Error starting new game:", error);
      this.setState({
        feedbackMessage: {
          text: "Ошибка при создании новой игры",
          type: "error",
        },
      });
    }
  };

  async makeGuess(action: AssistantAction): Promise<void> {
    console.log("makeGuess", action);
    if (action.word && !this.state.gameState.gameOver) {
      if (action.word) {
        return this.guessWord(action.word);
      }
    }
    return Promise.resolve();
  }

  guessWord = async (word: string): Promise<void> => {
    word = word.trim().toLocaleLowerCase();
    console.log("guessWord", word);
    try {
      const storedSessionId = localStorage.getItem("Contexto_session_id");

      const response = await fetch(`${API_BASE_URL}/api/guess`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          word,
          session_id: storedSessionId, // Include session_id in the request body as fallback
        }),
        credentials: "include",
      });

      if (!response.ok) {
        throw new Error(`HTTP error: ${response.status}`);
      }

      const data = await response.json();

      if (data.error) {
        // Handle error (e.g., invalid word)
        this.sendActionValue("error", data.error);
        this.setState({ feedbackMessage: { text: data.error, type: "error" } });
        return;
      }

      console.log("Backend response for guess:", data); // Log the whole data object
      console.log("Type of data.rank:", typeof data.rank, "Value:", data.rank); // Check type and value

      // Ensure rank is a number
      const rankAsNumber = parseInt(String(data.rank), 10);
      if (isNaN(rankAsNumber)) {
        console.error("Received non-numeric rank from backend:", data.rank);
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
        newGuessedWords.push({
          id: crypto.randomUUID(),
          text: word,
          rank: isNaN(rankAsNumber) ? -1 : rankAsNumber,
        });
      }

      // Sort by rank (similarity to target word)
      newGuessedWords.sort((a, b) => a.rank - b.rank);

      console.log("New guess with rank:", data.rank);

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
        this.setState({
          feedbackMessage: { text: successMessage, type: "success" },
        });
      } else {
        // Provide feedback on how close they are
        let feedback = "Очень далеко";
        if (data.rank <= 5) {
          feedback = "Очень близко!";
        } else if (data.rank <= 20) {
          feedback = "Теплее!";
        } else if (data.rank <= 50) {
          feedback = "Холоднее";
        }

        this.sendActionValue("feedback", feedback);
        this.setState({
          feedbackMessage: { text: feedback, type: "feedback" },
        });
      }
    } catch (error) {
      console.error("Error making guess:", error);
      const errorMessage = "Произошла ошибка при проверке слова";
      this.sendActionValue("error", errorMessage);
      this.setState({ feedbackMessage: { text: errorMessage, type: "error" } });
    }
  };

  private sendActionValue(action_id: string, value: string): void {
    const data: AssistantSendData = {
      action: {
        action_id,
        parameters: { value },
      },
    };

    this.assistant.sendData(data, (assistantData: any) => {
      const { type, payload } = assistantData || {};
      console.log("sendData onData:", type, payload);
    });
  }

  render(): React.ReactNode {
    console.log("render");
    return (
      <Container>
        <ContextoGame
          gameState={this.state.gameState}
          onGuess={this.guessWord}
          onNewGame={this.startNewGame}
          feedbackMessage={this.state.feedbackMessage}
        />
      </Container>
    );
  }
}
