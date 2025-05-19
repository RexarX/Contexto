import argparse
import re
from pathlib import Path
from natasha import Segmenter, MorphVocab, NewsEmbedding, NewsMorphTagger, Doc

def process_dictionary_lemmatization(input_file, output_file, min_word_length=2, verbose=False):
    """
    Process a dictionary file to extract lemmas using Natasha
    """
    # Initialize Natasha components
    segmenter = Segmenter()
    morph_vocab = MorphVocab()
    emb = NewsEmbedding()
    morph_tagger = NewsMorphTagger(emb)

    # Read the input file
    if verbose:
        print(f"Reading input file: {input_file}")
    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            lines = [line.strip() for line in f if line.strip()]
    except Exception as e:
        print(f"Error reading input file: {e}")
        return

    # Check if the first line contains the count
    word_count = 0
    start_idx = 0

    try:
        word_count = int(lines[0])
        start_idx = 1  # Skip the first line when processing
        if verbose:
            print(f"Detected word count in header: {word_count}")
    except ValueError:
        # First line is not a number, assume it's a word
        if verbose:
            print("No word count detected in header, processing all lines")

    # Process each word
    processed_words = []
    lemma_info = []
    skipped_words = 0

    if verbose:
        print("Processing words...")

    for i in range(start_idx, len(lines)):
        line = lines[i]

        # Check if the word already has a POS tag
        original = line
        clean_word = None
        existing_pos = None

        # Common formats: "word_POS" or "word_anytext_POS"
        pos_match = re.search(r'_([A-Z]+)$', line)
        if pos_match:
            existing_pos = pos_match.group(1)
            clean_word = line[:line.rfind('_')].lower()

            if verbose and i == start_idx:
                print(f"Detected words with POS tags. Example: '{line}' -> word='{clean_word}', pos='{existing_pos}'")
        else:
            # No POS tag found, just use the word as is
            clean_word = line.lower()

        # Skip if the word is too short
        if len(clean_word) < min_word_length:
            skipped_words += 1
            continue

        # Get the lemma using Natasha
        doc = Doc(clean_word)
        doc.segment(segmenter)
        doc.tag_morph(morph_tagger)

        if not doc.tokens:
            # Keep the original word if Natasha can't process it
            processed_words.append(original)
            lemma_info.append((original, clean_word))
            continue

        token = doc.tokens[0]
        token.lemmatize(morph_vocab)

        # Get the lemma (base form)
        lemma = token.lemma if token.lemma else clean_word

        # Store the original word
        processed_words.append(original)

        # Store lemma information
        lemma_info.append((original, lemma))

        # Progress reporting
        if verbose and (i+1) % 1000 == 0:
            print(f"Processed {i+1-start_idx} words...")

    # Write the output files
    if verbose:
        print(f"Writing {len(processed_words)} words to output file: {output_file}")

    try:
        # Write the main dictionary file (preserving original format)
        with open(output_file, 'w', encoding='utf-8') as f:
            # Write the count on the first line
            f.write(f"{len(processed_words)}\n")

            # Write each processed word on a new line
            for word in processed_words:
                f.write(f"{word}\n")

        # Write the lemma file with exactly one space between word and lemma
        lemma_output = output_file.parent / f"{output_file.stem}_lemmas{output_file.suffix}"
        with open(lemma_output, 'w', encoding='utf-8') as f:
            f.write(f"{len(lemma_info)}\n")

            for original, lemma in lemma_info:
                # Use exactly one space between word and lemma
                f.write(f"{original} {lemma}\n")

    except Exception as e:
        print(f"Error writing output file: {e}")
        return

    print(f"Conversion complete. {len(processed_words)} words processed, {skipped_words} words skipped.")
    print(f"Lemma information written to {lemma_output}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Extract lemmas from a dictionary file using Natasha NLP for Russian.')
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

    process_dictionary_lemmatization(input_path, output_path, args.min_length, args.verbose)
