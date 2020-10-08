// Copyright 2016 Alex Silva Torres
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lexer.h"

#include <sstream>
#include <codecvt>

namespace shpp {
namespace internal {

void Lexer::SkipSingleLineComment() {
  Advance();
  while (c_ != kEndOfInput && c_ != '\n'){
    Advance();
  }
}

void Lexer::ErrorMsg(const boost::format& fmt_msg) {
  Message msg(Message::Severity::ERR, fmt_msg, line_, line_pos_);
  msgs_.Push(std::move(msg));
  nerror_++;
}

char Lexer::ScanAnsiEscapeCode() {
  std::string number = "";
  while (IsOctalChar(c_)) {
    number += c_;
    Advance();
  }

  Back();

  return static_cast<char>(std::stoi(number,nullptr,8));
}

std::string Lexer::ScanUnicodeEscapeCode() {
  Advance();

  std::string number = "";
  while (IsHexChar(c_)) {
    number += c_;
    Advance();
  }

  Back();

  unsigned int32_t hex_number = static_cast<unsigned int32_t>(std::stoi(number,nullptr,16));

  // initialize wstring with 1 wchar_t
  std::wstring str_hex(1, static_cast<wchar_t>(hex_number));

  //setup converter from wstring to string
  using convert_type = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_type, wchar_t> converter;

  return converter.to_bytes(str_hex);
}

std::string Lexer::ScanStringEscape() {
  Advance();

  char c = c_;

  // Handle special char on string
  switch (c) {
    case '\'':  // fall through
    case '"' :  // fall through
    case '\\': break;

    case '0' :
      if (IsOctalChar(PeekAhead())) {
        c = ScanAnsiEscapeCode();
      } else {
        c = '\0';
      }
      break;

    case 'u':
      if (IsHexChar(PeekAhead())) {
        return ScanUnicodeEscapeCode();
      } else {
        return "\\u";
      }
      break;

    case 'b' : c = '\b'; break;
    case 'f' : c = '\f'; break;
    case 'n' : c = '\n'; break;
    case 'r' : c = '\r'; break;
    case 't' : c = '\t'; break;
  }

  // construct a string with 1 char c
  return std::string(1, c);
}

char Lexer::ScanWordEscape() {
  start_pos_ = line_pos_;
  Advance();

  char c = c_;

  switch (c) {
    case ' ' : c = ' '; break;
    case 'b' : c = '\b'; break;
    case 'f' : c = '\f'; break;
    case 'n' : c = '\n'; break;
    case 'r' : c = '\r'; break;
    case 't' : c = '\t'; break;
  }

  return c;
}

Token Lexer::ScanString() {
  std::string str = "";
  start_pos_ = line_pos_;

  Advance();

  while(true) {
    if (c_ == kEndOfInput) {
      ErrorMsg(boost::format("string literal not terminated"));
      break;
    }

    if (c_ == '"') {
      Advance();
      break;
    }


    if (c_ == '\\') {
       str += ScanStringEscape();
       Advance();
       continue;
    }

    str += c_;
    Advance();
  }

  // Check blank char after string
  char check_blank = c_;
  return GetToken(TokenKind::STRING_LITERAL, str, check_blank);
}

Token Lexer::ScanIdentifier(bool varenv) {
  std::string id = "";
  start_pos_ = line_pos_;

  if (IsIdentifierStart(c_)) {
    id += c_;
    Advance();

    while (IsLetter(c_) || c_ == '_' || IsDigit(c_)) {
      id += c_;
      Advance();
    }

    // Check blank char after identifier
    char check_blank = c_;

    // varenv is like $PATH, on this case it dosn't look if it is a keyord
    if (varenv) {
      return GetToken(TokenKind::VARENVID, id, check_blank);
    }

    TokenKind token_kind;
    bool res;
    std::tie(token_kind, res) = Token::IsKeyWord(id);



    if (res) {
      return GetToken(token_kind, check_blank);
    } else {
      return GetToken(TokenKind::IDENTIFIER, id, check_blank);
    }
  }

  return GetToken(TokenKind::UNKNOWN);
}

Token Lexer::ScanNumber() {
  std::string str_num = "";
  size_t point_num = 0;
  start_pos_ = line_pos_;

  if (IsDigit(c_)) {
    str_num += c_;
    Advance();
    while (IsDigit(c_) || c_ == '.') {
      if (c_ == '.') {
        point_num++;

        if (point_num > 1) {
          return ScanWord(str_num);
        }
      }

      str_num += c_;
      Advance();
    }
  }

  // Check blank char after number
  char check_blank = c_;

  if (point_num == 0) {
    int v;
    std::istringstream ss(str_num);
    ss >> v;
    return GetToken(TokenKind::INT_LITERAL, v, check_blank);
  } else {
    float v;
    std::istringstream ss(str_num);
    ss >> v;
    return GetToken(TokenKind::REAL_LITERAL, v, check_blank);
  }
}

Token Lexer::ScanWord(const std::string& prestr) {
  std::string word = prestr;
  start_pos_ = line_pos_;

  while (IsSpecialChar(c_)) {
    if (c_ == '\\') {
      word += ScanWordEscape();
      Advance();
      continue;
    }

    word += c_;
    Advance();
  }

  // Check blank char after word
  char check_blank = c_;
  return GetToken(TokenKind::WORD, word, check_blank);
}

TokenStream Lexer::Scanner() {
  TokenStream ts;
  char check_blank = c_;

  while (true) {
    start_pos_ = line_pos_;
    bool whitespace = false;
    Token&& token = GetToken(TokenKind::UNKNOWN);
    switch (c_) {
      case '#':
        SkipSingleLineComment();
        whitespace = true;
        break;

      case ' ':
      case '\t':
        Advance();
        whitespace = true;
        break;

      case '\n':
        token = Select(TokenKind::NWL);
        break;

      case '"':
        token = ScanString();
        break;

      case '<':
        // < <= <& << <<= <<<
        Advance();
        check_blank = c_;
        if (c_ == '=') {
          token = Select(TokenKind::LESS_EQ);
        } else if (c_ == '&') {
          token = Select(TokenKind::LESS_AND);
        } else if (c_ == '<') {
          Advance();
          if (c_ == '=') {
            token = Select(TokenKind::ASSIGN_SHL);
          } else if (c_ == '<') {
            token = Select(TokenKind::SSHL);
          } else {
            token = GetToken(TokenKind::SHL);
          }
        } else {
          token = GetToken(TokenKind::LESS_THAN, check_blank);
        }
        break;

      case '>':
        // > >= >& >> >>= >>>
        Advance();
        check_blank = c_;
        if (c_ == '=') {
          token = Select(TokenKind::GREATER_EQ);
        } else if (c_ == '&') {
          token = Select(TokenKind::GREAT_AND);
        } else if (c_ == '>') {
          Advance();
          if (c_ == '=') {
            token = Select(TokenKind::ASSIGN_SAR);
          } else if (c_ == '>') {
            token = Select(TokenKind::SSAR);
          } else {
            token = GetToken(TokenKind::SAR);
          }
        } else {
          token = GetToken(TokenKind::GREATER_THAN, check_blank);
        }
        break;

      case '=':
        // = ==
        Advance();
        if (c_ == '=') {
          token = Select(TokenKind::EQUAL);
        } else {
          token = GetToken(TokenKind::ASSIGN);
        }
        break;

      case '!':
        // ! !=
        Advance();
        if (c_ == '=') {
          token = Select(TokenKind::NOT_EQUAL);
        } else {
          token = GetToken(TokenKind::EXCL_NOT);
        }
        break;

      case '+':
        // + +=
        Advance();
        if (c_ == '=') {
          token = Select(TokenKind::ASSIGN_ADD);
        } else {
          token = GetToken(TokenKind::ADD);
        }
        break;

      case '-':
        // - -= ->
        Advance();
        if (c_ == '=') {
          token = Select(TokenKind::ASSIGN_SUB);
        } else if (c_ == '>') {
          token = Select(TokenKind::ARROW);
        } else {
          token = GetToken(TokenKind::SUB);
        }
        break;

      case '*':
        // * *=
        Advance();
        if (c_ == '=') {
          token = Select(TokenKind::ASSIGN_MUL);
        } else {
          token = GetToken(TokenKind::MUL);
        }
        break;

      case '/':
        // / /=
        Advance();
        if (c_ == '=') {
          token = Select(TokenKind::ASSIGN_DIV);
        } else {
          token = GetToken(TokenKind::DIV);
        }
        break;

      case '%':
        // % %=
        Advance();
        if (c_ == '=') {
          token = Select(TokenKind::ASSIGN_MOD);
        } else if (c_ == '%') {
          token = Select(TokenKind::RGLOB);
        } else {
          token = GetToken(TokenKind::MOD);
        }
        break;

      case '&':
        // & &= &&
        Advance();
        if (c_ == '=') {
          token = Select(TokenKind::ASSIGN_BIT_AND);
        } else if (c_ == '&') {
          token = Select(TokenKind::AND);
        } else {
          token = GetToken(TokenKind::BIT_AND);
        }
        break;

      case '|':
        // | |= ||
        Advance();
        if (c_ == '=') {
          token = Select(TokenKind::ASSIGN_BIT_OR);
        } else if (c_ == '|') {
          token = Select(TokenKind::OR);
        } else {
          token = GetToken(TokenKind::BIT_OR);
        }
        break;

      case '^':
        // ^ ^=
        Advance();
        if (c_ == '=') {
          token = Select(TokenKind::ASSIGN_BIT_XOR);
        } else {
          token = GetToken(TokenKind::BIT_XOR);
        }
        break;

      case '~':
        token = Select(TokenKind::BIT_NOT);
        break;

      case ';':
        token = Select(TokenKind::SEMI_COLON);
        break;

      case ':':
        // : ::
        Advance();
        if (c_ == ':') {
          token = Select(TokenKind::SCOPE);
        } else {
          token = GetToken(TokenKind::COLON);
        }
        break;

      case ',':
        token = Select(TokenKind::COMMA);
        break;

      case '$': {
        // $ $( ${
        Advance();
        std::string  pre_word = "$";
        if (c_ == '(') {
          token = Select(TokenKind::DOLLAR_LPAREN);
        } else if (c_ == '{') {
          token = Select(TokenKind::DOLLAR_LBRACE);
        } else if (c_ == '@') {
          pre_word += "@";
          Advance();
          if (c_ == '{') {
            token = Select(TokenKind::DOLLAR_AT_LBRACE);
          } else {
            token = ScanWord(pre_word);
          }
        } else if (IsIdentifierStart(c_)) {
          token = ScanIdentifier(true);
        } else {
          token = GetToken(TokenKind::DOLLAR);
        }
      } break;

      case '(':
        token = Select(TokenKind::LPAREN);
        break;

      case ')':
        token = Select(TokenKind::RPAREN);
        break;

      case '{':
        token = Select(TokenKind::LBRACE);
        break;

      case '}':
        token = Select(TokenKind::RBRACE);
        break;

      case '[':
        token = Select(TokenKind::LBRACKET);
        break;

      case ']':
        token = Select(TokenKind::RBRACKET);
        break;

      case '.': {
        // . ...
        std::string  pre_word = ".";
        Advance();
        if (c_ == '.') {
          pre_word += '.';
          Advance();
          if (c_ == '.') {
            token = Select(TokenKind::ELLIPSIS);
          } else {
            token = ScanWord(pre_word);
          }
        } else {
          token = GetToken(TokenKind::DOT);
        }
      } break;

      case '@':
        token = Select(TokenKind::AT_SIGN);
        break;

      default:
        if (c_ == kEndOfInput) {
          // End of the file, break the loop
          token = Select(TokenKind::EOS);
          ts.PushToken(std::move(token));
          return ts;
        } else if (IsIdentifierStart(c_)) {
          token = ScanIdentifier();
        } else if (IsDigit(c_)) {
          token = ScanNumber();
        } else if (c_ == '\\') {
          // Allows insert newline without insert a token
          if (PeekAhead() == '\n'){
            // advance char '\'
            Advance();
            // advace char '\n'
            Advance();
            whitespace = true;
          } else {
            std::string c;
            c = ScanWordEscape();
            token = Select(TokenKind::WORD, c);
          }
        } else {
          token = ScanWord();
        }
    }

    if (!whitespace) {
      ts.PushToken(std::move(token));
    }
  }

  return ts;
}

}
}
