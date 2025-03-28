import React from 'react';
import { createAssistant, createSmartappDebugger, AssistantAppState } from '@salutejs/client';

import './App.css';
import { ContextoGame } from './pages/ContextoGame';
import { 
  Word, 
  AssistantAction, 
  AssistantEvent, 
  AssistantState, 
  GameState,
  AssistantEventStart,
  AssistantSmartAppData,
  AssistantSmartAppError,
  AssistantSmartAppCallback
} from './types';

interface AppState {
  gameState: GameState;
  feedbackMessage?: {
    text: string;
    type: 'feedback' | 'error' | 'success';
  };
}

const API_BASE_URL = import.meta.env.VITE_API_BASE_URL || 'http://localhost:8080';

const initializeAssistant = (getState: () => AssistantAppState) => {
  if (import.meta.env.DEV) {
    return createSmartappDebugger({
      token: import.meta.env.VITE_APP_TOKEN || '',
      initPhrase: `Запусти ${import.meta.env.VITE_APP_SMARTAPP}`,
      getState,
      nativePanel: {
        defaultText: 'Попробуй угадать слово',
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
    console.log('constructor');

    this.state = {
      gameState: {
        guessedWords: [],
        gameOver: false
      }
    };

    this.assistant = initializeAssistant(() => this.getStateForAssistant());

    this.assistant.on('data', (event: AssistantEvent) => {
      console.log(`assistant.on(data)`, event);
      if (event.type === 'character') {
        console.log(`assistant.on(data): character: "${event.character?.id}"`);
      } else if (event.type === 'insets') {
        console.log(`assistant.on(data): insets`);
      } else if (event.action) {
        this.dispatchAssistantAction(event.action);
      }
    });
    
    this.assistant.on('start', ((event: AssistantEventStart) => {
      const initialData = this.assistant.getInitialData();
      console.log(`assistant.on(start)`, event, initialData);
      
      // Start a new game when the app loads
      this.startNewGame();
    }) as any);

    this.assistant.on('command', (event) => {
      console.log(`assistant.on(command)`, event);
    });

    this.assistant.on('error', (event) => {
      console.log(`assistant.on(error)`, event);
    });

    this.assistant.on('tts', (event) => {
      console.log(`assistant.on(tts)`, event);
    });
  }

  getStateForAssistant(): AssistantAppState {
    console.log('getStateForAssistant: this.state:', this.state);
    
    const state: AssistantAppState = {
      item_selector: {
        items: this.state.gameState.guessedWords.map(({ id, text, rank }) => ({
          number: rank,
          id,
          text,
        })),
        ignored_words: [
          'угадай', 'попробуй', 'может', 'быть', 'попробуем', 'новая', 'игра',
          'начать', 'начни', 'начнем'
        ],
      },
      game_state: this.state.gameState,
    };
    
    console.log('getStateForAssistant: state:', state);
    return state;
  }

  dispatchAssistantAction(action: AssistantAction): Promise<void> {
    console.log('dispatchAssistantAction', action);
    if (action) {
      switch (action.type) {
        case 'guess_word':
          return this.makeGuess(action);
          
        case 'new_game':
          return this.startNewGame();
  
        default:
          console.error(`Unhandled action type: ${action.type}`);
          return Promise.resolve();
      }
    }
    return Promise.resolve();
  }

  makeGuess(action: AssistantAction): Promise<void> {
    console.log('makeGuess', action);
    if (action.word && !this.state.gameState.gameOver) {
      return this.guessWord(action.word);
    }
    return Promise.resolve();
  }

  // Fetch from backend to check word and get ranking
  guessWord = async (word: string): Promise<void> => {
    try {
      const storedSessionId = localStorage.getItem('Contexto_session_id');
    
      const response = await fetch(`${API_BASE_URL}/api/guess`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ 
          word,
          session_id: storedSessionId // Include session_id in the request body as fallback
        }),
        credentials: 'include'
      });
      
      const data = await response.json();
      
      if (data.error) {
        // Handle error (e.g., invalid word)
        this.sendActionValue('error', data.error);
        this.setState({ feedbackMessage: { text: data.error, type: 'error' } });
        return;
      }
      
      // Update the game state with the new guess
      const newGuessedWords = [...this.state.gameState.guessedWords];
      newGuessedWords.push({
        id: crypto.randomUUID(),
        text: word,
        rank: data.rank
      });
      
      // Sort by rank (similarity to target word)
      newGuessedWords.sort((a, b) => a.rank - b.rank);
      
      // Check if the guess is correct (rank 1)
      const isCorrect = data.rank === 1;
      
      this.setState({
        gameState: {
          ...this.state.gameState,
          guessedWords: newGuessedWords,
          gameOver: isCorrect,
          targetWord: isCorrect ? word : this.state.gameState.targetWord
        }
      });
      
      if (isCorrect) {
        const successMessage = 'Поздравляем! Вы угадали секретное слово!';
        this.sendActionValue('success', successMessage);
        this.setState({ feedbackMessage: { text: successMessage, type: 'success' } });
      } else {
        // Provide feedback on how close they are
        let feedback = 'Очень далеко';
        if (data.rank <= 5) {
          feedback = 'Очень близко!';
        } else if (data.rank <= 20) {
          feedback = 'Теплее!';
        } else if (data.rank <= 50) {
          feedback = 'Холоднее';
        }
        
        this.sendActionValue('feedback', feedback);
        this.setState({ feedbackMessage: { text: feedback, type: 'feedback' } });
      }
      
    } catch (error) {
      console.error('Error making guess:', error);
      const errorMessage = 'Произошла ошибка при проверке слова';
      this.sendActionValue('error', errorMessage);
      this.setState({ feedbackMessage: { text: errorMessage, type: 'error' } });
    }
  }

  startNewGame = async (): Promise<void> => {
    try {
      this.setState({
        gameState: {
          guessedWords: [],
          gameOver: false
        },
        feedbackMessage: undefined
      });
  
      const response = await fetch(`${API_BASE_URL}/api/new-game`, {
        method: 'POST',
        credentials: 'include',
        headers: {
          'Content-Type': 'application/json'
        }
      });
      
      const data = await response.json();
      
      if (data.error) {
        console.error('Error starting new game:', data.error);
        this.setState({
          feedbackMessage: {
            text: `Ошибка при создании новой игры: ${data.error}`,
            type: 'error'
          }
        });
        return;
      }
      
      // Store session_id in localStorage as fallback
      if (data.session_id) {
        localStorage.setItem('Contexto_session_id', data.session_id);
      }
      
      console.log('New game started with session:', data.session_id);
    } catch (error) {
      console.error('Error starting new game:', error);
      this.setState({
        feedbackMessage: {
          text: 'Ошибка при создании новой игры',
          type: 'error'
        }
      });
    }
  }

  private sendActionValue(action_id: string, value: string): void {
    const data = {
      action: {
        action_id,
        parameters: {
          value,
        },
      },
    };
    const unsubscribe = this.assistant.sendData(data, (assistantData: any) => {
      const { type, payload } = assistantData || {};
      console.log('sendData onData:', type, payload);
      unsubscribe();
    });
  }

  render(): React.ReactNode {
    console.log('render');
    return (
      <>
        <ContextoGame
          gameState={this.state.gameState}
          onGuess={this.guessWord}
          onNewGame={this.startNewGame}
          feedbackMessage={this.state.feedbackMessage}
        />
      </>
    );
  }
}