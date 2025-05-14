#pragma once

#include <cassert>
#include <string>
#include <string_view>

namespace utils::utf8 {

static constexpr bool IsContinuationByte(char ch) noexcept {
  // UTF-8 bit pattern constants for continuation byte
  constexpr uint8_t UTF8_CONTINUATION_MASK = 0xC0;
  constexpr uint8_t UTF8_CONTINUATION_PATTERN = 0x80;

  return (static_cast<uint8_t>(ch) & UTF8_CONTINUATION_MASK) == UTF8_CONTINUATION_PATTERN;
}

static constexpr int CharLen(std::string_view str, size_t index) noexcept {
  // UTF-8 bit pattern constants
  constexpr uint8_t UTF8_ONE_BYTE_MASK = 0x80;
  constexpr uint8_t UTF8_TWO_BYTE_MASK = 0xE0;
  constexpr uint8_t UTF8_TWO_BYTE_PATTERN = 0xC0;
  constexpr uint8_t UTF8_THREE_BYTE_MASK = 0xF0;
  constexpr uint8_t UTF8_THREE_BYTE_PATTERN = 0xE0;
  constexpr uint8_t UTF8_FOUR_BYTE_MASK = 0xF8;
  constexpr uint8_t UTF8_FOUR_BYTE_PATTERN = 0xF0;

  if (index >= str.size()) return -1;

  const auto ch = static_cast<uint8_t>(str[index]);
  if (ch < UTF8_ONE_BYTE_MASK) return 1;
  if ((ch & UTF8_TWO_BYTE_MASK) == UTF8_TWO_BYTE_PATTERN) return 2;
  if ((ch & UTF8_FOUR_BYTE_PATTERN) == UTF8_THREE_BYTE_PATTERN) return 3;
  if ((ch & UTF8_FOUR_BYTE_MASK) == UTF8_FOUR_BYTE_PATTERN) return 4;

  return -1;  // Invalid UTF-8 sequence
}

static constexpr size_t CharCount(std::string_view str) noexcept {
  size_t count = 0;
  size_t index = 0;

  while (index < str.size()) {
    const int len = CharLen(str, index);
    if (len < 0) return count;  // Break on invalid UTF-8
    index += len;
    ++count;
  }

  return count;
}

static std::string ToLower(std::string_view str) {
  // Define named constants for UTF-8 Cyrillic character handling
  constexpr uint8_t CYRILLIC_A_CAPITAL_FIRST_BYTE = 0xD0;
  constexpr uint8_t CYRILLIC_A_CAPITAL_SECOND_BYTE = 0x90;
  constexpr uint8_t CYRILLIC_A_SMALL_FIRST_BYTE = 0xD0;
  constexpr uint8_t CYRILLIC_A_SMALL_SECOND_BYTE = 0xB0;

  constexpr uint8_t CYRILLIC_CAPITAL_FIRST_BYTE = 0xD0;
  constexpr uint8_t CYRILLIC_CAPITAL_A_TO_P_START = 0x90;  // А (A)
  constexpr uint8_t CYRILLIC_CAPITAL_A_TO_P_END = 0x9F;    // П (P)

  constexpr uint8_t CYRILLIC_CAPITAL_R_TO_YA_FIRST_BYTE = 0xD0;
  constexpr uint8_t CYRILLIC_CAPITAL_R_TO_YA_START = 0xA0;  // Р (R)
  constexpr uint8_t CYRILLIC_CAPITAL_R_TO_YA_END = 0xAF;    // Я (YA)

  constexpr uint8_t CYRILLIC_YO_CAPITAL_FIRST_BYTE = 0xD0;
  constexpr uint8_t CYRILLIC_YO_CAPITAL_SECOND_BYTE = 0x81;
  constexpr uint8_t CYRILLIC_YO_SMALL_FIRST_BYTE = 0xD1;
  constexpr uint8_t CYRILLIC_YO_SMALL_SECOND_BYTE = 0x91;

  std::string result;
  result.reserve(str.size());

  for (size_t i = 0; i < str.size();) {
    const int char_len = CharLen(str, i);

    // Invalid UTF-8 or end of string
    if (char_len <= 0 || i + char_len > str.size()) {
      result.push_back(str[i]);
      ++i;
      continue;
    }

    // ASCII single-byte characters (0x00-0x7F)
    if (char_len == 1) {
      char ch = str[i];
      if (ch >= 'A' && ch <= 'Z') {
        ch = static_cast<char>(ch - 'A' + 'a');  // Convert to lowercase with explicit cast
      }
      result.push_back(ch);
      ++i;
      continue;
    }

    // Handle 2-byte UTF-8 characters (most Cyrillic letters)
    if (char_len == 2) {
      const auto first = static_cast<uint8_t>(str[i]);
      const auto second = static_cast<uint8_t>(str[i + 1]);

      // Cyrillic uppercase letters А-П (A-P): D0 90-9F -> D0 B0-BF
      if (first == CYRILLIC_CAPITAL_FIRST_BYTE && second >= CYRILLIC_CAPITAL_A_TO_P_START &&
          second <= CYRILLIC_CAPITAL_A_TO_P_END) {
        result.push_back(static_cast<char>(CYRILLIC_A_SMALL_FIRST_BYTE));
        result.push_back(static_cast<char>(second + 0x20));  // Convert to lowercase
        i += 2;
        continue;
      }

      // Cyrillic uppercase letters Р-Я (R-YA): D0 A0-AF -> D1 80-8F
      if (first == CYRILLIC_CAPITAL_R_TO_YA_FIRST_BYTE && second >= CYRILLIC_CAPITAL_R_TO_YA_START &&
          second <= CYRILLIC_CAPITAL_R_TO_YA_END) {
        result.push_back(static_cast<char>(CYRILLIC_YO_SMALL_FIRST_BYTE));           // First byte changes to D1
        result.push_back(static_cast<char>(second - 0x20));  // Convert to lowercase
        i += 2;
        continue;
      }

      // Special case: Ё -> ё
      if (first == CYRILLIC_YO_CAPITAL_FIRST_BYTE && second == CYRILLIC_YO_CAPITAL_SECOND_BYTE) {
        result.push_back(static_cast<char>(CYRILLIC_YO_SMALL_FIRST_BYTE));
        result.push_back(static_cast<char>(CYRILLIC_YO_SMALL_SECOND_BYTE));
        i += 2;
        continue;
      }
    }

    // Any other character: copy as is
    for (int j = 0; j < char_len; ++j) {
      result.push_back(str[i + j]);
    }
    i += char_len;
  }

  return result;
}

static size_t CommonPrefixLength(std::string_view lhs, std::string_view rhs) {
  size_t common_chars = 0;
  size_t lhs_idx = 0;
  size_t rhs_idx = 0;

  // Compare character by character
  while (lhs_idx < lhs.size() && rhs_idx < rhs.size()) {
    const int lhs_len = CharLen(lhs, lhs_idx);
    const int rhs_len = CharLen(rhs, rhs_idx);

    // Invalid UTF-8 or different character sizes
    if (lhs_len <= 0 || rhs_len <= 0 || lhs_len != rhs_len) break;

    // Check if the characters match
    bool same = true;
    for (int k = 0; k < lhs_len; ++k) {
      if (lhs[lhs_idx + k] != rhs[rhs_idx + k]) {
        same = false;
        break;
      }
    }

    if (!same) break;

    // Move to the next character
    lhs_idx += lhs_len;
    rhs_idx += rhs_len;
    ++common_chars;
  }

  return common_chars;
}

}  // namespace utils::utf8
