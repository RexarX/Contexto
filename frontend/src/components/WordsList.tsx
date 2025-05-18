import React, { useEffect, useState } from "react";
import { Word } from "../types";
import { Card, Body1, Headline3 } from "@salutejs/plasma-ui";
import styled from "styled-components";

interface WordsListProps {
  words: Word[];
  gameOver: boolean;
  targetWord?: string;
  layout: "desktop" | "mobile";
}

// Common styled components
const WordsContainerBase = styled(Card)`
  margin-bottom: 1rem;
  flex: 1;
  display: flex;
  flex-direction: column;
  overflow: visible;
  background: rgba(25, 25, 25, 0.7);
  backdrop-filter: blur(10px);
  border: 1px solid rgba(255, 255, 255, 0.1);
`;

// Desktop-specific components
const DesktopWordsContainer = styled(WordsContainerBase)`
  padding: 1.5rem;
  min-height: 400px;
  max-height: calc(100vh - 350px); // Dynamic max height based on viewport
`;

const DesktopTargetWordCard = styled(Card)`
  padding: 1.5rem;
  margin-bottom: 1.5rem;
  text-align: center;
  background-color: rgba(46, 204, 113, 0.15);
  backdrop-filter: blur(8px);
  border: 1px solid rgba(46, 204, 113, 0.2);
`;

const DesktopWordsListContainer = styled.div`
  display: flex;
  flex-direction: column;
  gap: 0.25rem;
  flex: 1;
  overflow-y: auto;
  max-height: 500px;

  &::-webkit-scrollbar {
    width: 8px;
  }

  &::-webkit-scrollbar-track {
    background: rgba(255, 255, 255, 0.05);
  }

  &::-webkit-scrollbar-thumb {
    background: rgba(255, 255, 255, 0.2);
    border-radius: 4px;
  }
`;

const DesktopWordItemWrapper = styled.div<{ isNew: boolean }>`
  display: flex;
  align-items: center;
  padding: 8px 12px;
  border-radius: 8px;
  animation: ${(props: { isNew: boolean }) =>
    props.isNew ? "highlight 1.5s ease-out" : "none"};

  @keyframes highlight {
    0% {
      background-color: rgba(52, 152, 219, 0.2);
      transform: translateX(-5px);
    }
    50% {
      background-color: rgba(52, 152, 219, 0.1);
      transform: translateX(0);
    }
    100% {
      background-color: transparent;
    }
  }
`;

// Mobile-specific components
const MobileWordsContainer = styled(WordsContainerBase)`
  padding: 0.75rem;
  min-height: 280px;
  max-height: calc(100vh - 380px); // Dynamic max height based on viewport
`;

const MobileTargetWordCard = styled(Card)`
  padding: 1rem;
  margin-bottom: 1rem;
  text-align: center;
  background-color: rgba(46, 204, 113, 0.15);
  backdrop-filter: blur(8px);
  border: 1px solid rgba(46, 204, 113, 0.2);
`;

const MobileWordsListContainer = styled.div`
  display: flex;
  flex-direction: column;
  gap: 0.25rem;
  flex: 1;
  overflow-y: auto;

  &::-webkit-scrollbar {
    width: 4px;
  }

  &::-webkit-scrollbar-track {
    background: rgba(255, 255, 255, 0.05);
  }

  &::-webkit-scrollbar-thumb {
    background: rgba(255, 255, 255, 0.2);
    border-radius: 2px;
  }
`;

// Custom wrapper for Mobile Word Items that handles the styling
const MobileWordItemWrapper = styled.div<{ isNew: boolean }>`
  display: flex;
  align-items: center;
  padding: 6px 8px;
  border-radius: 6px;
  animation: ${(props: { isNew: boolean }) =>
    props.isNew ? "highlight 1.5s ease-out" : "none"};
`;

// Badge components - fixed with proper props
const DesktopBadge = styled.div<{ type: string }>`
  min-width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 16px;
  font-size: 14px;
  font-weight: bold;
  margin-right: 12px;
  color: white;

  background-color: ${(props) => {
    switch (props.type) {
      case "accent":
        return "rgba(46, 204, 113, 0.9)";
      case "default":
        return "rgba(52, 152, 219, 0.9)";
      case "low":
        return "rgba(149, 165, 166, 0.9)";
      default:
        return "rgba(52, 152, 219, 0.9)";
    }
  }};

  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
`;

const MobileBadge = styled.div<{ type: string }>`
  min-width: 32px;
  height: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  border-radius: 16px;
  font-size: 14px;
  font-weight: bold;
  margin-right: 8px;
  color: white;

  background-color: ${(props) => {
    switch (props.type) {
      case "accent":
        return "rgba(46, 204, 113, 0.9)";
      case "default":
        return "rgba(52, 152, 219, 0.9)";
      case "low":
        return "rgba(149, 165, 166, 0.9)";
      default:
        return "rgba(52, 152, 219, 0.9)";
    }
  }};

  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2);
`;

// Common components
const TargetWordText = styled.span`
  color: #2ecc71;
  font-weight: bold;
`;

const HintText = styled(Body1)`
  text-align: center;
  color: rgba(255, 255, 255, 0.6);
  padding: 2rem 0;
  font-style: italic;
`;

const WordText = styled(Body1)<{ layout: string }>`
  margin-left: ${(props) => (props.layout === "desktop" ? "12px" : "8px")};
  font-size: ${(props) => (props.layout === "desktop" ? "16px" : "14px")};
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
`;

export const WordsList: React.FC<WordsListProps> = ({
  words,
  gameOver,
  targetWord,
  layout,
}) => {
  const [lastAddedWordId, setLastAddedWordId] = useState<string | null>(null);

  useEffect(() => {
    if (words.length > 0) {
      const latestWord = words[words.length - 1];
      setLastAddedWordId(latestWord.id);

      const timer = setTimeout(() => {
        setLastAddedWordId(null);
      }, 1500);

      return () => clearTimeout(timer);
    }
  }, [words.length]);

  const renderWordsList = () => {
    if (words.length === 0) {
      return <HintText>Попробуйте угадать секретное слово!</HintText>;
    }

    return words.map((word) => {
      const rankValue = word.rank <= 0 ? "?" : String(word.rank);
      let badgeType = "";

      if (word.rank <= 5) badgeType = "accent";
      else if (word.rank <= 20) badgeType = "default";
      else badgeType = "low";

      if (layout === "desktop") {
        return (
          <DesktopWordItemWrapper
            key={word.id}
            isNew={word.id === lastAddedWordId}
          >
            <DesktopBadge type={badgeType}>{rankValue}</DesktopBadge>
            <WordText layout="desktop">{word.text}</WordText>
          </DesktopWordItemWrapper>
        );
      } else {
        return (
          <MobileWordItemWrapper
            key={word.id}
            isNew={word.id === lastAddedWordId}
          >
            <MobileBadge type={badgeType}>{rankValue}</MobileBadge>
            <WordText layout="mobile">{word.text}</WordText>
          </MobileWordItemWrapper>
        );
      }
    });
  };

  // Render based on layout
  if (layout === "desktop") {
    return (
      <DesktopWordsContainer>
        {gameOver && targetWord && (
          <DesktopTargetWordCard>
            <Headline3>
              Секретное слово: <TargetWordText>{targetWord}</TargetWordText>
            </Headline3>
          </DesktopTargetWordCard>
        )}

        <DesktopWordsListContainer>
          {renderWordsList()}
        </DesktopWordsListContainer>
      </DesktopWordsContainer>
    );
  } else {
    return (
      <MobileWordsContainer>
        {gameOver && targetWord && (
          <MobileTargetWordCard>
            <Headline3 style={{ fontSize: "20px" }}>
              Секретное слово: <TargetWordText>{targetWord}</TargetWordText>
            </Headline3>
          </MobileTargetWordCard>
        )}

        <MobileWordsListContainer>{renderWordsList()}</MobileWordsListContainer>
      </MobileWordsContainer>
    );
  }
};
