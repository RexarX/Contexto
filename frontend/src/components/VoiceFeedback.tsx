import React, { useEffect, useState } from 'react';
import '../styles/VoiceFeedback.css';

interface VoiceFeedbackProps {
  message: string;
  type: 'feedback' | 'error' | 'success';
}

export const VoiceFeedback: React.FC<VoiceFeedbackProps> = ({ message, type }) => {
  const [visible, setVisible] = useState(true);
  
  useEffect(() => {
    const timer = setTimeout(() => {
      setVisible(false);
    }, 3000);
    
    return () => clearTimeout(timer);
  }, [message]);
  
  if (!visible) return null;
  
  return (
    <div className={`voice-bubble ${type}`}>
      <div className="voice-icon">
        {type === 'error' ? '!' : type === 'success' ? '✓' : '💬'}
      </div>
      <div className="bubble-message">{message}</div>
    </div>
  );
};