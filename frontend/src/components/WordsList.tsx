import React, { useEffect, useState } from "react";
import { Word } from '../types';
import "../App.css";

interface WordsListProps {
  words: Word[];
  gameOver: boolean;
  targetWord?: string;
}

export const WordsList: React.FC<WordsListProps> = ({ words, gameOver, targetWord }) => {
  const [lastAddedWordId, setLastAddedWordId] = useState<string | null>(null);
  
  useEffect(() => {
    // When a new word is added, mark it as the latest
    if (words.length > 0) {
      const latestWord = words[words.length - 1];
      setLastAddedWordId(latestWord.id);
      
      // Reset the animation after it completes
      const timer = setTimeout(() => {
        setLastAddedWordId(null);
      }, 1500);
      
      return () => clearTimeout(timer);
    }
  }, [words.length]);

  return (
    <div className="words-container">
      {gameOver && targetWord && (
        <div className="target-word">
          <h3>Секретное слово: <span>{targetWord}</span></h3>
        </div>
      )}
      
      <ul className="words-list">
        {words.length === 0 ? (
          <li className="hint-text">Попробуйте угадать секретное слово!</li>
        ) : (
          words.map((word) => (
            <li 
              key={word.id} 
              className={`word-item ${word.id === lastAddedWordId ? 'word-item-new' : ''}`}
            >
              <span className="word-rank">{word.rank}</span>
              <span className="word-text">{word.text}</span>
            </li>
          ))
        )}
      </ul>
    </div>
  );
};