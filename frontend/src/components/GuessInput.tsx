import React from "react";
import "../App.css";

interface GuessInputProps {
  onGuess: (word: string) => void;
  disabled?: boolean;
}

interface GuessInputState {
  guess: string;
}

export class GuessInput extends React.Component<GuessInputProps, GuessInputState> {
  constructor(props: GuessInputProps) {
    super(props);
    this.state = {
      guess: '',
    };
  }

  handleSubmit = (event: React.FormEvent<HTMLFormElement>): void => {
    event.preventDefault();
    if (this.state.guess.trim() && !this.props.disabled) {
      this.props.onGuess(this.state.guess);
      this.setState({ guess: '' });
    }
  };

  handleChange = (event: React.ChangeEvent<HTMLInputElement>): void => {
    this.setState({ guess: event.target.value });
  };

  render(): React.ReactNode {
    return (
      <form onSubmit={this.handleSubmit} className="guess-form">
        <input
          className="guess-input"
          type="text"
          placeholder="Введите слово"
          value={this.state.guess}
          onChange={this.handleChange}
          disabled={this.props.disabled}
          required
          autoFocus
        />
        <button 
          type="submit" 
          className="guess-button" 
          disabled={this.props.disabled}
        >
          Угадать
        </button>
      </form>
    );
  }
}