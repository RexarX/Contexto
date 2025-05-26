import React, { useEffect, useState } from "react";
import styled, { keyframes } from "styled-components";
import { Headline2, Display1, Body1 } from "@salutejs/plasma-ui";

interface CelebrationProps {
  word: string;
}

const fadeIn = keyframes`
  from {
    opacity: 0;
  }
  to {
    opacity: 1;
  }
`;

const popIn = keyframes`
  0% {
    transform: scale(0.8);
    opacity: 0;
  }
  100% {
    transform: scale(1);
    opacity: 1;
  }
`;

const pulse = keyframes`
  0% {
    transform: scale(1);
  }
  50% {
    transform: scale(1.05);
  }
  100% {
    transform: scale(1);
  }
`;

const glow = keyframes`
  from {
    text-shadow:
      0 0 5px rgba(46, 204, 113, 0.5),
      0 0 10px rgba(46, 204, 113, 0.3);
  }
  to {
    text-shadow:
      0 0 15px rgba(46, 204, 113, 0.8),
      0 0 20px rgba(46, 204, 113, 0.5);
  }
`;

const confettiFall = keyframes`
  0% {
    transform: translateY(-100%) rotate(0deg);
    opacity: 1;
  }
  100% {
    transform: translateY(1000%) rotate(720deg);
    opacity: 0;
  }
`;

const ModalOverlay = styled.div`
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-color: rgba(0, 0, 0, 0.7);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
  animation: ${fadeIn} 0.5s ease-out;
`;

const ModalContent = styled.div`
  background-color: #222;
  border-radius: 16px;
  padding: 40px 20px; /* Reduced horizontal padding to give more space for the word */
  text-align: center;
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
  max-width: 90%;
  width: min(500px, 90vw); /* Responsive width */
  position: relative;
  animation: ${popIn} 0.5s cubic-bezier(0.175, 0.885, 0.32, 1.275);
  overflow: hidden; /* Ensure no overflow */
`;

const CelebrationTitle = styled(Headline2)`
  margin: 0 0 20px;
  color: #3498db;
  animation: ${pulse} 2s infinite;
`;

const CelebrationWord = styled(Display1)`
  margin: 20px 0;
  font-weight: bold;
  background: linear-gradient(45deg, #27ae60, #2ecc71);
  -webkit-background-clip: text;
  -webkit-text-fill-color: transparent;
  animation: ${glow} 2s infinite alternate;

  /* Better handling for long words */
  white-space: nowrap;
  display: block;
  width: 100%;
  text-align: center;

  /* Dynamic font size based on word length */
  font-size: ${(props) => {
    // Dynamically calculate font size based on word length
    const wordLength = props.children?.toString().length || 0;
    if (wordLength > 12) return "clamp(16px, 6vw, 36px)";
    if (wordLength > 8) return "clamp(18px, 8vw, 42px)";
    return "clamp(20px, 10vw, 48px)";
  }};

  /* Ensure padding for better visibility */
  padding: 0 10px;

  /* Ensure proper rendering on all browsers */
  transform: translateZ(0);
`;

const CelebrationMessage = styled(Body1)`
  margin-top: 20px;
  color: rgba(255, 255, 255, 0.7);
`;

const ConfettiContainer = styled.div`
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  overflow: hidden;
  pointer-events: none;
`;

const Confetti = styled.div`
  position: absolute;
  width: 10px;
  height: 10px;
  opacity: 0.7;
  animation: ${confettiFall} 5s linear forwards;
  transform: rotate(0deg);
`;

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
    <ModalOverlay onClick={() => setVisible(false)}>
      <ModalContent onClick={(e) => e.stopPropagation()}>
        <ConfettiContainer>
          {Array.from({ length: 50 }).map((_, i) => (
            <Confetti
              key={i}
              style={{
                left: `${Math.random() * 100}%`,
                animationDelay: `${Math.random() * 3}s`,
                backgroundColor: `hsl(${Math.random() * 360}, 100%, 50%)`,
              }}
            />
          ))}
        </ConfettiContainer>

        <CelebrationTitle>Поздравляем!</CelebrationTitle>
        <CelebrationWord>{word}</CelebrationWord>
        <CelebrationMessage>Вы угадали секретное слово!</CelebrationMessage>
      </ModalContent>
    </ModalOverlay>
  );
};
