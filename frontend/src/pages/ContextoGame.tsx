import React from 'react';
import { GuessInput } from '../components/GuessInput';
import { WordsList } from '../components/WordsList';
import { Celebration } from '../components/Celebration';
import { VoiceFeedback } from '../components/VoiceFeedback';
import { Word, GameState } from '../types';

interface ContextoGameProps {
  gameState: GameState;
  onGuess: (word: string) => void;
  onNewGame: () => void;
  feedbackMessage?: {
    text: string;
    type: 'feedback' | 'error' | 'success';
  };
}

export const ContextoGame: React.FC<ContextoGameProps> = ({ 
  gameState, 
  onGuess, 
  onNewGame,
  feedbackMessage
}) => {
  const { guessedWords, gameOver, targetWord } = gameState;

  return (
    <main className="container Contexto-game">
      <header className="game-header">
        <h1>Contexto</h1>
        <p className="game-description">
          Угадайте секретное слово. Слова в списке отсортированы по схожести с секретным.
        </p>
      </header>
      
      <WordsList 
        words={guessedWords} 
        gameOver={gameOver} 
        targetWord={targetWord} 
      />
      
      <div className="game-controls">
        <GuessInput onGuess={onGuess} disabled={gameOver} />
        
        {gameOver && (
          <button 
            className="new-game-button"
            onClick={onNewGame}
          >
            Новая игра
          </button>
        )}
      </div>

      {feedbackMessage && (
        <VoiceFeedback 
          message={feedbackMessage.text}
          type={feedbackMessage.type}
        />
      )}
      
      {gameOver && targetWord && (
        <Celebration word={targetWord} />
      )}
    </main>
  );
};