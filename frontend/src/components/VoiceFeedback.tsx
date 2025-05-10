import React, { useEffect, useState } from "react";
import { IconDone, IconClose, IconMessage } from "@salutejs/plasma-icons";
import styled from "styled-components";

interface VoiceFeedbackProps {
  message: string;
  type: "feedback" | "error" | "success";
}

const StyledToast = styled.div<{ type: string }>`
  position: fixed;
  bottom: 30px;
  right: 30px;
  background-color: ${(props) =>
    props.type === "error"
      ? "#e74c3c"
      : props.type === "success"
        ? "#2ecc71"
        : "#3498db"};
  color: white;
  padding: 16px;
  border-radius: 8px;
  display: flex;
  align-items: center;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2);
  z-index: 1000;
  animation: fadeIn 0.3s ease-out;

  @keyframes fadeIn {
    from {
      opacity: 0;
      transform: translateY(20px);
    }
    to {
      opacity: 1;
      transform: translateY(0);
    }
  }
`;

const IconContainer = styled.div`
  margin-right: 12px;
  display: flex;
  align-items: center;
`;

export const VoiceFeedback: React.FC<VoiceFeedbackProps> = ({
  message,
  type,
}) => {
  const [visible, setVisible] = useState(true);

  useEffect(() => {
    const timer = setTimeout(() => {
      setVisible(false);
    }, 3000);

    return () => clearTimeout(timer);
  }, [message]);

  if (!visible) return null;

  const getIcon = () => {
    switch (type) {
      case "success":
        return <IconDone color="white" />;
      case "error":
        return <IconClose color="white" />;
      default:
        return <IconMessage color="white" />;
    }
  };

  return (
    <StyledToast type={type}>
      <IconContainer>{getIcon()}</IconContainer>
      {message}
    </StyledToast>
  );
};
