import argparse
import re
from pathlib import Path
import pymorphy2

def process_dictionary(input_file, output_file, min_word_length=2, verbose=False):
    """
    Process a dictionary file and convert it to the required format.

    Args:
        input_file: Path to the input dictionary file
        output_file: Path to the output file
        min_word_length: Minimum length of words to include
        verbose: Print processing details
    """
    # Initialize the morphological analyzer
    morph = pymorphy2.MorphAnalyzer()

    # POS tag mapping from pymorphy2 to the format needed
    pos_mapping = {
        'NOUN': 'NOUN',
        'VERB': 'VERB',
        'INFN': 'VERB',  # Infinitive verbs -> VERB
        'ADJF': 'ADJ',   # Full adjectives
        'ADJS': 'ADJ',   # Short adjectives
        'ADVB': 'ADV',   # Adverbs
        'NPRO': 'PRON',  # Pronouns
        'PREP': 'ADP',   # Prepositions
        'CONJ': 'CONJ',  # Conjunctions
        'PRCL': 'PART',  # Particles
        'INTJ': 'INTJ',  # Interjections
        'NUMR': 'NUM',   # Numerals
    }

    # Read the input file
    print(f"Reading input file: {input_file}")
    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            lines = [line.strip() for line in f if line.strip()]
    except Exception as e:
        print(f"Error reading input file: {e}")
        return

    # Process each word
    processed_words = []
    skipped_words = 0

    print("Processing words...")
    for i, word in enumerate(lines):
        # Remove any punctuation and extra spaces
        clean_word = re.sub(r'[^\w\s]', '', word).strip()

        # Skip if the word is too short
        if len(clean_word) < min_word_length:
            skipped_words += 1
            continue

        # Get the most likely POS tag
        parsed = morph.parse(clean_word)
        if not parsed:
            skipped_words += 1
            continue

        # Get the most likely parse
        best_parse = parsed[0]
        pos = best_parse.tag.POS

        # Map POS to our format
        mapped_pos = pos_mapping.get(pos, '')

        if mapped_pos:
            processed_words.append(f"{clean_word}_{mapped_pos}")
        else:
            # If no mapping, just use the word without POS tag
            processed_words.append(clean_word)

        # Progress reporting
        if verbose and (i+1) % 1000 == 0:
            print(f"Processed {i+1} words...")

    # Write the output file
    print(f"Writing {len(processed_words)} words to output file: {output_file}")
    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            # Write the count on the first line
            f.write(f"{len(processed_words)}\n")

            # Write each processed word on a new line
            for word in processed_words:
                f.write(f"{word}\n")
    except Exception as e:
        print(f"Error writing output file: {e}")
        return

    print(f"Conversion complete. {len(processed_words)} words processed, {skipped_words} words skipped.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Convert a dictionary file to the required format.')
    parser.add_argument('input_file', type=str, help='Path to the input dictionary file')
    parser.add_argument('--output', '-o', type=str, help='Path to the output file')
    parser.add_argument('--min-length', '-m', type=int, default=2,
                        help='Minimum word length to include (default: 2)')
    parser.add_argument('--verbose', '-v', action='store_true', help='Print processing details')

    args = parser.parse_args()

    input_path = Path(args.input_file)

    # If output file not specified, create one with a similar name
    if args.output:
        output_path = Path(args.output)
    else:
        output_path = input_path.with_name(f"{input_path.stem}_processed{input_path.suffix}")

    process_dictionary(input_path, output_path, args.min_length, args.verbose)
