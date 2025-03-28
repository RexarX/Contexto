import React, { useEffect, useState } from 'react';
import '../styles/Celebration.css';

interface CelebrationProps {
  word: string;
}

export const Celebration: React.FC<CelebrationProps> = ({ word }) => {
  const [visible, setVisible] = useState(true);
  
  useEffect(() => {
    // Automatically hide after animation completes
    const timer = setTimeout(() => {
      setVisible(false);
    }, 4000);
    
    return () => clearTimeout(timer);
  }, []);
  
  if (!visible) return null;
  
  return (
    <div className="celebration-overlay">
      <div className="celebration-content">
        <div className="confetti-container">
          {Array.from({ length: 50 }).map((_, i) => (
            <div key={i} className="confetti" style={{
              left: `${Math.random() * 100}%`,
              animationDelay: `${Math.random() * 3}s`,
              backgroundColor: `hsl(${Math.random() * 360}, 100%, 50%)`
            }}></div>
          ))}
        </div>
        
        <h2 className="celebration-title">Поздравляем!</h2>
        <div className="celebration-word">{word}</div>
        <p className="celebration-message">Вы угадали секретное слово!</p>
      </div>
    </div>
  );
};