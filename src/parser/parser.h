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

#ifndef SHPP_PARSER_H
#define SHPP_PARSER_H

#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <tuple>
#include <boost/format.hpp>

#include "token.h"
#include "msg.h"
#include "lexer.h"
#include "parser_result.h"
#include "ast/ast.h"

namespace shpp {
namespace internal {

class Parser {
 public:
  Parser() = delete;

  Parser(TokenStream&& ts)
      : ts_(std::move(ts))
      , factory_(std::bind(&Parser::Pos, this))
      , nerror_(0)
      , token_(ts_.CurrentToken())
      , token_error_(TokenKind::NUM_TOKENS, false, 0, 0) {}

  ParserResult<StatementList> AstGen() {
    return ParserStmtList();
  }

  ParserResult<Cmd> AstGenCmdExpr() {
    return ParserExpCmd();
  }

  inline unsigned int nerrors() const {
    return nerror_;
  }

  inline const Message& Msgs() const {
    return *msgs_.begin();
  }

  bool StmtIncomplete() {
    if ((nerror_ > 0) && (token_error_ == TokenKind::EOS)) {
      return true;
    }

    return false;
  }

 private:
  inline const Token& CurrentToken() const noexcept {
    return ts_.CurrentToken();
  }

  inline const Token& PeekAhead() const noexcept {
    return ts_.PeekAhead();
  }

  inline void Advance() noexcept {
    ts_.Advance();
    token_ = ts_.CurrentToken();
  }

  Token NextToken() noexcept {
    Token token = ts_.NextToken();
    token_ = ts_.CurrentToken();
    return token;
  }

  // Advance until find a valid token
  inline const Token& ValidToken() {
    while (CurrentToken() == TokenKind::NWL) {
      Advance();
    }

    return token_;
  }

  inline bool CheckComma() {
    if (token_.Is(TokenKind::COMMA)) {
      Advance();
      return true;
    } else {
      return false;
    }
  }

  inline std::string TokenValueStr() {
    return Token::TokenValueToStr(token_.GetValue());
  }

  inline bool IsCmdExprToken() {
    if (token_ == TokenKind::DOLLAR_LBRACE ||
        token_ == TokenKind::DOLLAR_AT_LBRACE) {
      return true;
    }

    return false;
  }

  void ErrorMsg(const boost::format& fmt_msg) {
    if (nerror_ == 0) {
      token_error_ = token_;
      Message msg(Message::Severity::ERR, fmt_msg, token_.Line(), token_.Col());
      msgs_.Push(std::move(msg));
    }
    nerror_++;
  }

  inline Position Pos() {
    Position pos = {token_.Line(), token_.Col()};
    return pos;
  }

  // Say if the current token is a token used to end a stmt
  bool IsEndOfStmt() {
    return token_.IsAny(TokenKind::NWL, TokenKind::SEMI_COLON,
                        TokenKind::RBRACE);
  }

  // Try match with any language statement or declaration
  bool MatchLangStmt() {
    const Token& tok(CurrentToken());
    const Token& tok_ahead(PeekAhead());

    // match id ('=' | '+=' | ...) ...
    if (Token::IsAssignToken(tok_ahead.GetKind())) {
      return true;
    }

    // match (..., %..., $(...
    if (tok.IsAny(TokenKind::LPAREN, TokenKind::MOD,
                  TokenKind::DOLLAR_LPAREN)) {
      return true;
    }

    switch (tok_ahead.GetKind()) {
      case TokenKind::COMMA:     // match id, ...
      case TokenKind::DOT:       // match id.
      case TokenKind::LPAREN:    // match id(...
      case TokenKind::LBRACKET:  // match id[...
      case TokenKind::SCOPE:     // match id::
        return true;
        break;

      default:
        return false;
    }
  }

  // Test if the token is an IO valid token
  inline bool IsIOToken(const Token& tok) {
    switch (tok.GetKind()) {
      case TokenKind::LESS_THAN:
      case TokenKind::GREATER_THAN:
      case TokenKind::SHL:
      case TokenKind::SAR:
      case TokenKind::SSHL:
      case TokenKind::SSAR:
      case TokenKind::GREAT_AND:
      case TokenKind::LESS_AND:
        return true;
        break;

      default:
        return false;
    }
  }

  // Test if the integer inside a command should be parsed as
  // only an integer token, or it is the io output channel
  inline bool CmdValidInt() {
    if ((token_.Is(TokenKind::INT_LITERAL)) &&
        (token_.BlankAfter() == false) &&
        (PeekAhead() == TokenKind::GREATER_THAN ||
        PeekAhead() == TokenKind::SAR ||
        PeekAhead() == TokenKind::SSAR ||
        PeekAhead() == TokenKind::GREAT_AND)) {
      return true;
    }

    return false;
  }

  // check if is redirecting all output descriptors, stdout and stderr
  // ex: test_cmd &> file.txt
  inline bool CmdValidAnd() {
    if ((token_.Is(TokenKind::BIT_AND)) &&
        (token_.BlankAfter() == false) &&
        (PeekAhead() == TokenKind::GREATER_THAN ||
        PeekAhead() == TokenKind::SAR ||
        PeekAhead() == TokenKind::SSAR ||
        PeekAhead() == TokenKind::GREAT_AND)) {
      return true;
    }

    return false;
  }

  // Check if is a stop sequence of tokens to command
  inline bool IsCmdStopPoint() {
    return Token::CmdValidToken(token_) || CmdValidInt();
  }

  inline bool TokenEndFullCmd() {
    bool r = token_.IsAny(TokenKind::NWL,
                          TokenKind::EOS,
                          TokenKind::RBRACE,
                          TokenKind::SEMI_COLON,
                          TokenKind::RPAREN);
    return r;
  }

  inline bool IsUnaryOp() {
    bool r = token_.IsAny(TokenKind::ADD, TokenKind::SUB, TokenKind::BIT_NOT,
                          TokenKind::EXCL_NOT);
    return r;
  }

  void SetTokenError(const Token& tk_err) {
    token_error_ = tk_err;
  }

  ParserResult<Expression> LiteralExp();
  ParserResult<Expression> ParserScopeIdentifier();
  std::vector<std::unique_ptr<Identifier>> ParserIdList();
  ParserResult<Expression> ParserPrimaryExp();
  ParserResult<Expression> ParserGlobExp(bool recursive = false);
  ParserResult<Expression> ParserPostExp();
  ParserResult<Expression> ParserUnaryExp();
  ParserResult<Expression> ParserTerm();
  ParserResult<Expression> ParserArithExp();
  ParserResult<Expression> ParserShiftExp();
  ParserResult<Expression> ParserBitAndExp();
  ParserResult<Expression> ParserBitXorExp();
  ParserResult<Expression> ParserBitOrExp();
  ParserResult<Expression> ParserComparisonExp();
  ParserResult<Expression> ParserNotExp();
  ParserResult<Expression> ParserAndExp();
  ParserResult<Expression> ParserOrExp();
  ParserResult<Expression> ParserLetExp();
  ParserResult<Expression> ParserIfElseExp();
  ParserResult<Expression> ParserEllipsisExp();
  ParserResult<Expression> ParserLambda();
  ParserResult<ExpressionList> ParserExpList();
  ParserResult<ExpressionList> ParserExpNoTestList();
  ParserResult<Statement> ParserStmt();
  ParserResult<Statement> ParserSimpleStmt(bool force_assignment = false);
  ParserResult<Statement> ParserGlobalAssignment();
  ParserResult<StatementList> ParserStmtList();
  ParserResult<Statement> ParserBlock();
  ParserResult<Statement> ParserVarEnvStmt();
  std::unique_ptr<Statement> ParserDeferableStmt();
  ParserResult<Statement> ParserDeferStmt();
  ParserResult<Statement> ParserBreakStmt();
  ParserResult<Statement> ParserContinueStmt();
  ParserResult<Statement> ParserReturnStmt();
  ParserResult<Statement> ParserIfStmt();
  ParserResult<Statement> ParserWhileStmt();
  ParserResult<Statement> ParserSwitchStmt();
  std::unique_ptr<CaseStatement> ParserCaseStmt();
  std::unique_ptr<DefaultStatement> ParserDefaultStmt();
  ParserResult<ExpressionList> ParserPostExpList();
  ParserResult<Statement> ParserForInStmt();
  ParserResult<Cmd> ParserExpCmd();
  ParserResult<Statement> ParserSubShell();
  ParserResult<Statement> ParserSimpleCmd();
  std::unique_ptr<CmdIoRedirect> ParserIoRedirectCmd();
  ParserResult<Statement> ParserIoRedirectCmdList();
  bool IsIoRedirect();
  ParserResult<Statement> ParserCmdPipe();
  ParserResult<Statement> ParserCmdAndOr();
  ParserResult<Statement> ParserCmdFull();
  ParserResult<AstNode> ParserFunctionDeclaration(bool lambda,
      bool abstract = false, bool fstatic = false);

  // parser every declartion accpted by class
  ParserResult<ClassBlock> ParserClassBlock();

  ParserResult<InterfaceBlock> ParserInterfaceBlock();

  ParserResult<Declaration> ParserInterfaceDecl();

  ParserResult<Declaration> ParserVariableDecl();

  std::unique_ptr<FinallyStatement> ParserFinally();
  std::vector<std::unique_ptr<CatchStatement>> ParserCatchList();
  ParserResult<Statement> ParserTryCatch();

  ParserResult<Statement> ParserThrow();

  // parser class declaration
  ParserResult<Declaration> ParserClassDecl(bool is_final, bool abstract);

  std::tuple<std::vector<std::unique_ptr<FunctionParam>>, bool>
  ParserParamsList();

  ParserResult<Declaration> ParserCmdDeclaration();

  ParserResult<Declaration> ParserAliasDeclaration();

  ParserResult<Statement> ParserStmtDecl();
  bool IsStmtDecl();

  ParserResult<Expression> ParserArgument();
  ParserResult<ArgumentsList> ParserArgumentsList();
  ParserResult<AssignableValue> ParserAssignable();

  ParserResult<AssignableList> ParserAssignableList(
      std::unique_ptr<AssignableValue> first_value =
          std::unique_ptr<AssignableValue>(nullptr));

  ParserResult<Expression> ParserArrayInstantiation();

  ParserResult<Expression> ParserTupleInstantiation();

  std::vector<std::unique_ptr<Expression>> ParserListComprehension();
  ParserResult<Expression> ParserCompIf();
  ParserResult<Expression> ParserCompFor();

  std::tuple<std::unique_ptr<KeyValue>, bool> ParserKeyValue();
  ParserResult<Expression> ParserDictionary();

  ParserResult<Expression> ParserSlice();

  ParserResult<Statement> ParserImportStmt();
  ParserResult<Statement> ParserImportIdListStmt();
  ParserResult<Statement> ParserImportPathStmt();
  ParserResult<Statement> ParserImportStarStmt();
  ParserResult<Statement> ParserDelStmt();

  TokenStream ts_;
  AstNodeFactory factory_;
  unsigned int nerror_;
  Token& token_;
  Token token_error_;
  Messages msgs_;
};

}
}

#endif  // SHPP_PARSER_H
