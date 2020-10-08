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

#ifndef SHPP_TOKEN_H
#define SHPP_TOKEN_H

#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <map>
#include <tuple>
#include <boost/variant.hpp>

namespace shpp {
namespace internal {

enum class TokenKind {
  UNKNOWN = 0,

#define TOKEN(X, Y) X,
#define LITERAL(X, Y) X,
#define KEYWORD(X, Y) KW_ ## X,
#define PUNCTUATOR_ASSIGN(X, Y) X,
#define PUNCTUATOR_COMP(X, Y) X,
#define PUNCTUATOR(X, Y) X,
#include "token.def"

  NUM_TOKENS
};

static const char* token_value_str[] = {
  "UNKNOWN", // UNKNOWN

#define TOKEN(X, Y) Y,
#define LITERAL(X, Y) Y,
#define KEYWORD(X, Y) Y,
#define PUNCTUATOR_ASSIGN(X, Y) Y,
#define PUNCTUATOR_COMP(X, Y) Y,
#define PUNCTUATOR(X, Y) Y,
#include "token.def"

  ""
};

static const char* token_name_str[] = {
  "UNKNOWN", // UNKNOWN

#define TOKEN(X, Y) #X,
#define LITERAL(X, Y) #X,
#define KEYWORD(X, Y) #X,
#define PUNCTUATOR_ASSIGN(X, Y) #X,
#define PUNCTUATOR_COMP(X, Y) #X,
#define PUNCTUATOR(X, Y) #X,
#include "token.def"

  ""
};


class Token {
 public:
  using Value = boost::variant<int, std::string, float, bool>;

  Token(TokenKind k, Value value, bool blank_after, unsigned int line, unsigned int col)
      : kind_(k)
      , value_(value)
      , blank_after_(blank_after)
      , line_(line)
      , col_(col) {}

  Token(TokenKind k, const char* value, bool blank_after, unsigned int line, unsigned int col)
      : kind_(k)
      , value_(std::string(value))
      , blank_after_(blank_after)
      , line_(line)
      , col_(col) {}

  Token(TokenKind k, bool blank_after, unsigned int line, unsigned int col)
      : kind_(k)
      , value_(std::string(token_value_str[static_cast<int>(k)]))
      , blank_after_(blank_after)
      , line_(line)
      , col_(col) {}

  Token(const Token& tok)
      : kind_(tok.kind_)
      , value_(tok.value_)
      , blank_after_(tok.blank_after_)
      , line_(tok.line_)
      , col_(tok.col_) {}

  Token(Token&& tok) noexcept
      : kind_(tok.kind_)
      , value_(std::move(tok.value_))
      , blank_after_(tok.blank_after_)
      , line_(tok.line_)
      , col_(tok.col_) {}

  Token& operator=(const Token& tok) {
    if (&tok == this)
      return *this;

    kind_ = tok.kind_;
    value_ = tok.value_;
    blank_after_ = tok.blank_after_;
    line_ = tok.line_;
    col_ = tok.col_;

    return *this;
  }

  Token& operator=(Token&& tok) noexcept {
    if (&tok == this)
      return *this;

    kind_ = tok.kind_;
    value_ = std::move(tok.value_);
    blank_after_ = tok.blank_after_;
    line_ = tok.line_;
    col_ = tok.col_;

    return *this;
  }

  bool operator==(TokenKind k) const noexcept { return Is(k); }
  bool operator!=(TokenKind k) const noexcept { return IsNot(k); }

  TokenKind GetKind() const noexcept { return kind_; }

  Value GetValue() const noexcept { return value_; }

  bool BlankAfter() const noexcept { return blank_after_; }

  unsigned int Line() const noexcept { return line_; }

  unsigned int Col() const noexcept { return col_; }

  bool Is(TokenKind k) const noexcept { return kind_ == k; }

  bool IsNot(TokenKind k) const noexcept { return kind_ != k; }

  bool IsAny(TokenKind k) const noexcept {
    return Is(k);
  }

  template <typename ...T>
  bool IsAny(TokenKind k1, TokenKind k2, T... kn) const noexcept {
    if (Is(k1))
      return true;
    return IsAny(k2, kn...);
  }

  template <typename ...T>
  bool IsNot(TokenKind k1, T... kn) const noexcept {
    return !IsAny(k1, kn...);
  }

  friend std::ostream& operator<<(std::ostream& stream, Token& token);
  friend std::ostream& operator<<(std::ostream& stream, const Token& token);

  // TODO(alex): this method must be optimized on future
  static std::tuple<TokenKind, bool> IsKeyWord(std::string str) {
    static std::map<std::string, TokenKind> map = {
#define KEYWORD(X, Y) {Y, TokenKind::KW_ ## X},
#include "token.def"
      {"", TokenKind::UNKNOWN}
    };

    auto it = map.find(str);
    if (it != map.end())
      return std::tuple<TokenKind, bool>(it->second, true);

    return std::tuple<TokenKind, bool>(TokenKind::UNKNOWN, false);
  }

  static bool IsAssignToken(TokenKind k) {
    switch (k) {
#define PUNCTUATOR_ASSIGN(X, Y) \
      case TokenKind::X:        \
      return true;
#include "token.def"
      break;

      default:
        return false;
    }
  }

  static bool IsComparisonToken(TokenKind k) {
    switch (k) {
#define PUNCTUATOR_COMP(X, Y) \
      case TokenKind::X:      \
      return true;
#include "token.def"
      break;

      default:
        return false;
    }
  }

  operator std::string() {
    return std::string(token_value_str[static_cast<int>(kind_)]);
  }

  static const char* name(TokenKind kind) {
    return token_value_str[static_cast<int>(kind)];
  }

  static std::string TokenValueToStr(Value value) {
    std::string str_value;
    std::stringstream stream;
    boost::apply_visitor(Token::Output{stream}, value);
    return stream.str();
  }

  static bool CmdValidToken(const Token& tok) {
    switch (tok.GetKind()) {
      case TokenKind::SHL:            // <<
      case TokenKind::SAR:            // >>
      case TokenKind::SSHL:           // <<<
      case TokenKind::SSAR:           // >>>
      case TokenKind::BIT_AND:        // &
      case TokenKind::BIT_OR:         // |
      case TokenKind::RPAREN:         // )
      case TokenKind::RBRACE:         // }
      case TokenKind::AND:            // &&
      case TokenKind::OR:             // ||
      case TokenKind::LESS_THAN:      // <
      case TokenKind::GREATER_THAN:   // >
      case TokenKind::GREAT_AND:      // >&
      case TokenKind::LESS_AND:       // <&
      case TokenKind::NWL:            // new line
      case TokenKind::SEMI_COLON:     // ;
      case TokenKind::EOS:            // eos
        return true;
        break;

      default:
        return false;
    }
  }

 private:
  struct Output : public boost::static_visitor<> {
    std::ostream& stream_;
    Output(std::ostream& stream): stream_(stream) {}
    template <typename T>
    void operator()(T t) const { stream_ << t; }
  };

  TokenKind kind_;
  Value value_;
  bool blank_after_;
  unsigned int line_;
  unsigned int col_;
};

inline std::ostream& operator<<(std::ostream& stream, Token& token) {
  std::string value = token_value_str[static_cast<int>(token.kind_)];
  stream  << "Name: " << token_name_str[static_cast<int>(token.kind_)]
          << ", BlankSpace: " << (token.blank_after_? "y": "n")
          << ", Type: " << (value == "\n"? "": value) << ", Value: ";

  boost::apply_visitor(Token::Output{stream}, token.value_);
  stream << "\n";

  return stream;
}

inline std::ostream& operator<<(std::ostream& stream, const Token& token) {
  std::string value = token_value_str[static_cast<int>(token.kind_)];
  stream  << "Name: " << token_name_str[static_cast<int>(token.kind_)]
          << ", BlankSpace: " << (token.blank_after_? "y": "n")
          << ", Type: " << (value == "\n"? "": value) << ", Value: ";

  boost::apply_visitor(Token::Output{stream}, token.value_);
  stream << "\n";

  return stream;
}

class TokenStream {
 public:
  TokenStream(): pos_(0) {}

  TokenStream(const TokenStream&) = delete;
  TokenStream& operator=(const TokenStream&) = delete;

  TokenStream(TokenStream&& tok_stream)
      : tok_vec_(std::move(tok_stream.tok_vec_))
      , pos_(tok_stream.pos_) {
    tok_stream.pos_ = 0;
  }

  inline TokenStream& operator=(TokenStream&& tok_stream) {
    if (this == &tok_stream)
      return *this;

    tok_vec_ = std::move(tok_stream.tok_vec_);
    pos_ = tok_stream.pos_;
    tok_stream.pos_ = 0;

    return *this;
  }

  inline void PushToken(Token&& tok) {
    tok_vec_.push_back(std::move(tok));
  }

  inline const Token& CurrentToken() const {
    return tok_vec_.at(pos_);
  }

  inline Token& CurrentToken() {
    return tok_vec_.at(pos_);
  }

  inline const Token& PeekAhead() const {
    if ((pos_ + 1) >= (tok_vec_.size() - 1))
      return tok_vec_.back();

    return tok_vec_.at(pos_ + 1);
  }

  inline Token& PeekAhead() {
    return tok_vec_.at(pos_ + 1);
  }

  inline Token& NextToken() {
    if ((pos_ + 1) >= (tok_vec_.size() - 1))
      return tok_vec_.back();

    Token& tk = tok_vec_.at(++pos_);
    return tk;
  }

  inline bool Advance() {
    if (pos_ == tok_vec_.size() - 1)
      return false;

    ++pos_;
    return true;
  }

  inline size_t Size() const noexcept {
    return tok_vec_.size();
  }

 private:
  std::vector<Token> tok_vec_;
  size_t pos_;
};

}
}

#endif  // SETTI_TOKEN_H
