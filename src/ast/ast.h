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

#ifndef SHPP_AST_H
#define SHPP_AST_H

#include <string>
#include <memory>
#include <vector>
#include <list>
#include <iostream>
#include <functional>

#include "parser/token.h"
#include "parser/lexer.h"

namespace shpp {
namespace internal {

#define DECLARATION_NODE_LIST(V) \
  V(ClassDeclaration)            \
  V(InterfaceDeclaration)        \
  V(VariableDeclaration)         \
  V(FunctionDeclaration)         \
  V(FunctionParam)               \
  V(CmdDeclaration)              \
  V(AliasDeclaration)            \
  V(ClassDeclList)               \
  V(InterfaceDeclList)           \
  V(ClassBlock)                  \
  V(InterfaceBlock)              \

#define ITERATION_NODE_LIST(V) \
  V(DoWhileStatement)          \
  V(WhileStatement)            \
  V(ForStatement)              \
  V(ForInStatement)

#define BREAKABLE_NODE_LIST(V) \
  V(Block)                     \
  V(SwitchStatement)

#define STATEMENT_NODE_LIST(V)    \
  ITERATION_NODE_LIST(V)          \
  BREAKABLE_NODE_LIST(V)          \
  V(StatementList)                \
  V(AssignmentStatement)          \
  V(GlobalAssignmentStatement)    \
  V(ExpressionStatement)          \
  V(EmptyStatement)               \
  V(IfStatement)                  \
  V(ContinueStatement)            \
  V(BreakStatement)               \
  V(ReturnStatement)              \
  V(CaseStatement)                \
  V(DefaultStatement)             \
  V(TryCatchStatement)            \
  V(FinallyStatement)             \
  V(CatchStatement)               \
  V(ThrowStatement)               \
  V(DeferStatement)               \
  V(DelStatement)                 \
  V(ImportStatement)              \
  V(VarEnvStatement)              \
  V(DebuggerStatement)

#define LITERAL_NODE_LIST(V) \
  V(RegExpLiteral)           \
  V(ObjectLiteral)

#define PROPERTY_NODE_LIST(V) \
  V(Assignment)               \
  V(CountOperation)           \
  V(Property)                 \
  V(KeyValue)                 \
  V(Argument)

#define CALL_NODE_LIST(V) \
  V(Call)                 \
  V(CallNew)

#define PAKCAGE_NODE_LIST(V) \
  V(PackageScope)

#define EXPRESSION_NODE_LIST(V) \
  LITERAL_NODE_LIST(V)          \
  PROPERTY_NODE_LIST(V)         \
  CALL_NODE_LIST(V)             \
  V(FunctionLiteral)            \
  V(ClassLiteral)               \
  V(Attribute)                  \
  V(Conditional)                \
  V(VariableProxy)              \
  V(Literal)                    \
  V(NullExpression)             \
  V(Array)                      \
  V(ArrayInstantiation)         \
  V(CompIf)                     \
  V(CompFor)                    \
  V(ListComprehension)          \
  V(TupleInstantiation)         \
  V(DictionaryInstantiation)    \
  V(IfElseExpression)           \
  V(Identifier)                 \
  V(VarEnvId)                   \
  V(Yield)                      \
  V(CallRuntime)                \
  V(UnaryOperation)             \
  V(BinaryOperation)            \
  V(NotExpression)              \
  V(CompareOperation)           \
  V(AssignableValue)            \
  V(AssignableList)             \
  V(ArgumentsList)              \
  V(ExpressionList)             \
  V(FunctionCall)               \
  V(LetExpression)              \
  V(FunctionExpression)         \
  V(CmdExpression)              \
  V(Slice)                      \
  V(Glob)                       \
  V(ThisFunction)               \
  V(SuperPropertyReference)     \
  V(SuperCallReference)         \
  V(CaseClause)                 \
  V(EmptyParentheses)           \
  V(EllipsisExpression)         \
  V(DoExpression)

#define CMD_NODE_LIST(V)   \
  V(Cmd)                   \
  V(CmdPiece)              \
  V(SimpleCmd)             \
  V(CmdIoRedirect)         \
  V(FilePathCmd)           \
  V(CmdIoRedirectList)     \
  V(CmdPipeSequence)       \
  V(CmdAndOr)              \
  V(CmdFull)               \
  V(SubShell)              \
  V(CmdValueExpr)

#define AST_NODE_LIST(V)        \
  DECLARATION_NODE_LIST(V)      \
  STATEMENT_NODE_LIST(V)        \
  EXPRESSION_NODE_LIST(V)       \
  CMD_NODE_LIST(V)              \
  PAKCAGE_NODE_LIST(V)

class AstNodeFactory;
class AstVisitor;
class Expression;

#define DECLARE_TYPE_CLASS(type) class type;
  AST_NODE_LIST(DECLARE_TYPE_CLASS)
#undef DECLARE_TYPE_CLASS

// Position of ast node on source code
struct Position {
  unsigned int line;
  unsigned int col;
};

const char* AstNodeStr(size_t i);

class AstNode {
 public:
#define DECLARE_TYPE_ENUM(type) k##type,
  enum class NodeType : unsigned int8_t { AST_NODE_LIST(DECLARE_TYPE_ENUM) };
#undef DECLARE_TYPE_ENUM

  virtual ~AstNode() {}

  NodeType type() {
    return type_;
  }

  static bool IsExpression(NodeType k) {
    switch (k) {
#define DECLARE_TYPE_EXPR(type) \
      case NodeType::k##type:   \
        return true;            \
      break;

      EXPRESSION_NODE_LIST(DECLARE_TYPE_EXPR)

#undef DECLARE_TYPE_EXPR

      default:
        return false;
    }
  }

  static bool IsStatement(NodeType k) {
    switch (k) {
#define DECLARE_TYPE_STMT(type) \
      case NodeType::k##type:   \
        return true;            \
      break;

      STATEMENT_NODE_LIST(DECLARE_TYPE_STMT)

#undef DECLARE_TYPE_EXPR

      default:
        return false;
    }
  }

  virtual void Accept(AstVisitor* visitor) = 0;

  Position pos() const noexcept {
    return position_;
  }

 private:
  NodeType type_;
  Position position_;

 protected:
  AstNode(NodeType type, Position position):
      type_(type), position_(position) {}
};

class AstVisitor {
 public:

#define DECLARE_VIRTUAL_FUNC(type) \
  virtual void Visit##type(type* /*node*/) {}
  AST_NODE_LIST(DECLARE_VIRTUAL_FUNC)
#undef DECLARE_VIRTUAL_FUNC
};

class Statement: public AstNode {
 public:
  virtual ~Statement() {}

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  Statement(NodeType type, Position position): AstNode(type, position) {}
};

class Declaration: public Statement {
 public:
  virtual ~Declaration() {}

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  Declaration(NodeType type, Position position): Statement(type, position) {}
};

// Interface class to assignable
class AssignableInterface {};

class Expression: public Statement, public AssignableInterface {
 public:
  virtual ~Expression() {}

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  Expression(NodeType type, Position position): Statement(type, position) {}
};

class Cmd: public Statement {
 public:
  virtual ~Cmd() {}

  virtual void Accept(AstVisitor* visitor) = 0;

 protected:
  Cmd(NodeType type, Position position): Statement(type, position) {}
};

class StatementList: public AstNode {
 public:
  virtual ~StatementList() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitStatementList(this);
  }

  bool IsEmpty() const noexcept {
    return stmt_list_.empty();
  }

  std::vector<Statement*> children() noexcept {
    std::vector<Statement*> vec;

    for (auto&& p_stmt: stmt_list_) {
      vec.push_back(p_stmt.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    return stmt_list_.size();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<Statement>> stmt_list_;

  StatementList(std::vector<std::unique_ptr<Statement>> stmt_list,
                Position position)
      : AstNode(NodeType::kStatementList, position)
      , stmt_list_(std::move(stmt_list)) {}
};

class KeyValue: public AstNode {
 public:
  virtual ~KeyValue() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitKeyValue(this);
  }

  Expression* key() const noexcept {
    return key_.get();
  }

  AssignableValue* value() const noexcept {
    return value_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> key_;
  std::unique_ptr<AssignableValue> value_;

  KeyValue(std::unique_ptr<Expression> key,
           std::unique_ptr<AssignableValue> value,
           Position position)
      : AstNode(NodeType::kKeyValue, position)
      , key_(std::move(key))
      , value_(std::move(value)) {}
};

class DictionaryInstantiation: public Expression {
 public:
  virtual ~DictionaryInstantiation() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitDictionaryInstantiation(this);
  }

  std::vector<KeyValue*> children() noexcept {
    std::vector<KeyValue*> vec;

    for (auto&& e: key_value_list_) {
      vec.push_back(e.get());
    }

    return vec;
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<KeyValue>> key_value_list_;

  DictionaryInstantiation(
      std::vector<std::unique_ptr<KeyValue>>&& key_value_list,
      Position position)
      : Expression(NodeType::kDictionaryInstantiation, position)
      , key_value_list_(std::move(key_value_list)) {}
};

class ArrayInstantiation: public Expression {
 public:
  virtual ~ArrayInstantiation() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitArrayInstantiation(this);
  }

  AssignableList* assignable_list() {
    return elements_.get();
  }

  bool has_elements() const noexcept {
    if (elements_) {
      return true;
    }

    return false;
  }

  bool valid_elements() const noexcept {
    if (elements_) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<AssignableList> elements_;

  ArrayInstantiation(std::unique_ptr<AssignableList> elements,
                     Position position)
      : Expression(NodeType::kArrayInstantiation, position)
      , elements_(std::move(elements)) {}
};

class TupleInstantiation: public Expression {
 public:
  virtual ~TupleInstantiation() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitTupleInstantiation(this);
  }

  AssignableList* assignable_list() {
    return elements_.get();
  }

  bool has_elements() const noexcept {
    if (elements_) {
      return true;
    }

    return false;
  }

  bool valid_elements() const noexcept {
    if (elements_) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<AssignableList> elements_;

  TupleInstantiation(std::unique_ptr<AssignableList> elements,
                     Position position)
      : Expression(NodeType::kTupleInstantiation, position)
      , elements_(std::move(elements)) {}
};

class AssignableValue: public Expression {
 public:
  virtual ~AssignableValue() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitAssignableValue(this);
  }

  AstNode* value() const noexcept {
    return value_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<AstNode> value_;

 template<class T>
 AssignableValue(std::unique_ptr<T>&& value, Position position)
     : Expression(NodeType::kAssignableValue, position) {
   static_assert(std::is_base_of<AstNode,T>::value,
                 "Type is not derivated from AstNode");

   static_assert(std::is_base_of<AssignableInterface,T>::value,
                 "Type not implements AssignableInterface");

   value_ = std::unique_ptr<AstNode>(static_cast<AstNode*>(value.release()));
 }
};

class AssignableList: public AstNode, public AssignableInterface {
 public:
  virtual ~AssignableList() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitAssignableList(this);
  }

  std::vector<AssignableValue*> children() noexcept {
    std::vector<AssignableValue*> vec;

    for (auto&& node: nodes_) {
      vec.push_back(node.get());
    }

    return vec;
  }

  size_t size() const {
    return nodes_.size();
  }

  std::vector<std::unique_ptr<AssignableValue>>& nodes() {
    return nodes_;
  }

  bool IsEmpty() const noexcept {
    return nodes_.empty();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<AssignableValue>> nodes_;

  AssignableList(std::vector<std::unique_ptr<AssignableValue>>&& nodes,
                 Position position)
      : AstNode(NodeType::kAssignableList, position) {
    for (auto&& node: nodes) {
      nodes_.push_back(std::move(node));
    }
  }
};

class FunctionParam: public AstNode {
 public:
  virtual ~FunctionParam() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitFunctionParam(this);
  }

  bool variadic() const noexcept {
    return variadic_;
  }

  Identifier* id() const noexcept {
    return id_.get();
  }

  AssignableValue* value() const noexcept {
    return value_.get();
  }

  bool has_value() const noexcept {
    if (value_) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Identifier> id_;
  std::unique_ptr<AssignableValue> value_;
  bool variadic_;

  FunctionParam(std::unique_ptr<Identifier> id,
                std::unique_ptr<AssignableValue> value,
                bool variadic,
                Position position)
     : AstNode(NodeType::kFunctionParam, position)
     , id_(std::move(id))
     , value_(std::move(value))
     , variadic_(variadic) {}
};

class ReturnStatement: public Statement {
 public:
  virtual ~ReturnStatement() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitReturnStatement(this);
  }

  AssignableList* assign_list() const noexcept {
    return assign_list_.get();
  }

  bool is_void() const noexcept {
    if (assign_list_) {
      return false;
    }

    return true;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<AssignableList> assign_list_;

  ReturnStatement(std::unique_ptr<AssignableList> assign_list,
                  Position position)
      : Statement(NodeType::kReturnStatement, position)
      , assign_list_(std::move(assign_list)) {}
};

class ImportStatement: public Statement {
 public:
  using Import =
      boost::variant<std::vector<std::unique_ptr<Identifier>>,
      std::unique_ptr<Literal>>;

  virtual ~ImportStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitImportStatement(this);
  }

 const std::string& from() const noexcept {
   return from_;
 }

 template<typename T>
 const T& import() noexcept {
   return boost::get<T>(import_);
 }

 bool has_from() const noexcept {
   return has_from_;
 }

 bool is_import_path() const noexcept {
   int index = import_.which();

   if (index == 1) {
     if (boost::get<std::unique_ptr<Literal>>(import_)) {
       return true;
     }

     return false;
   }

   return false;
 }

 Identifier* as() const noexcept {
   return as_.get();
 }

 bool has_as() const noexcept {
   if (as_) {
     return true;
   }

   return false;
 }

 bool star() const {
   return star_;
 }

 private:
  friend class AstNodeFactory;

  std::string from_;
  Import import_;
  std::unique_ptr<Identifier> as_;
  bool has_from_;
  bool star_;

  ImportStatement(const std::string& from,  Import import, bool star,
      Position position)
      : Statement(NodeType::kImportStatement, position)
      , from_(std::move(from))
      , import_(std::move(import))
      , as_(std::unique_ptr<Identifier>(nullptr))
      , has_from_(true)
      , star_(star) {}

  ImportStatement(Import import, std::unique_ptr<Identifier> as,
                  Position position)
      : Statement(NodeType::kImportStatement, position)
      , import_(std::move(import))
      , as_(std::move(as))
      , has_from_(false)
      , star_(false) {}
};

class Block: public Statement {
 public:
  virtual ~Block() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitBlock(this);
  }

  StatementList* stmt_list() const noexcept {
    return stmt_list_.get();
  }

  bool is_empty() const noexcept {
    if (stmt_list_->num_children() == 0) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<StatementList> stmt_list_;

  Block(std::unique_ptr<StatementList> stmt_list, Position position)
      : Statement(NodeType::kBlock, position)
      , stmt_list_(std::move(stmt_list)) {}
};

class ExpressionList: public AstNode {
 public:
  virtual ~ExpressionList() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitExpressionList(this);
  }

  bool IsEmpty() const noexcept {
    return exps_.empty();
  }

  std::vector<Expression*> children() noexcept {
    std::vector<Expression*> vec;

    for (auto&& p_exp: exps_) {
      vec.push_back(p_exp.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    return exps_.size();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<Expression>> exps_;

  ExpressionList(std::vector<std::unique_ptr<Expression>> exps,
                 Position position)
      : AstNode(NodeType::kExpressionList, position)
      , exps_(std::move(exps)) {}
};

class EllipsisExpression: public Expression {
 public:
  virtual ~EllipsisExpression() {}

  virtual void Accept(AstVisitor* visitor) {
  visitor->VisitEllipsisExpression(this);
  }

  Expression* expr() const noexcept {
  return expr_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> expr_;

  EllipsisExpression(std::unique_ptr<Expression> expr, Position position)
    : Expression(NodeType::kEllipsisExpression, position)
    , expr_(std::move(expr)) {}
};

class CmdExpression: public Expression {
 public:
  virtual ~CmdExpression() {}

  virtual void Accept(AstVisitor* visitor) {
  visitor->VisitCmdExpression(this);
  }

  Cmd* cmd() const noexcept {
  return cmd_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Cmd> cmd_;

  CmdExpression(std::unique_ptr<Cmd> cmd, Position position)
    : Expression(NodeType::kCmdExpression, position)
    , cmd_(std::move(cmd)) {}
};

class SubShell: public Cmd {
 public:
  virtual ~SubShell() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitSubShell(this);
  }

 Block* block() const noexcept {
  return block_.get();
 }

 bool self_process() const {
   return self_process_;
 }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Block> block_;
  bool self_process_;

  SubShell(std::unique_ptr<Block> block, bool self_process, Position position)
      : Cmd(NodeType::kSubShell, position)
      , block_(std::move(block))
      , self_process_(self_process) {}
};

class CmdFull: public Cmd {
 public:
  virtual ~CmdFull() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitCmdFull(this);
  }

  bool background() const noexcept {
   return background_;
  }

  Cmd* cmd() const noexcept {
   return cmd_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Cmd> cmd_;
  bool background_;

  CmdFull(std::unique_ptr<Cmd> cmd, bool background,
          Position position)
     : Cmd(NodeType::kCmdFull, position)
     , cmd_(std::move(cmd))
     , background_(background) {}
};

class CmdAndOr: public Cmd {
 public:
  virtual ~CmdAndOr() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCmdAndOr(this);
  }

  TokenKind kind() const noexcept {
    return token_kind_;
  }

  Cmd* cmd_left() const noexcept {
    return cmd_left_.get();
  }

  Cmd* cmd_right() const noexcept {
    return cmd_right_.get();
  }

 private:
  friend class AstNodeFactory;

  TokenKind token_kind_;
  std::unique_ptr<Cmd> cmd_left_;
  std::unique_ptr<Cmd> cmd_right_;

  CmdAndOr(TokenKind token_kind, std::unique_ptr<Cmd> cmd_left,
           std::unique_ptr<Cmd> cmd_right, Position position)
      : Cmd(NodeType::kCmdAndOr, position)
      , token_kind_(token_kind)
      , cmd_left_(std::move(cmd_left))
      , cmd_right_(std::move(cmd_right)) {}
};

class CmdPipeSequence: public Cmd {
 public:
  ~CmdPipeSequence() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCmdPipeSequence(this);
  }

  std::vector<Cmd*> cmds() const noexcept {
    std::vector<Cmd*> vec;

    for (auto&& cmd: cmds_) {
      vec.push_back(cmd.get());
    }

    return vec;
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<Cmd>> cmds_;

  CmdPipeSequence(std::vector<std::unique_ptr<Cmd>>&& cmds, Position position)
      : Cmd(NodeType::kCmdPipeSequence, position)
      , cmds_(std::move(cmds)) {}
};

class CmdPiece: public AstNode {
 public:
  ~CmdPiece() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCmdPiece(this);
  }

  std::string cmd_str() {
    std::string str = Token::TokenValueToStr(token_.GetValue());

    return str;
  }

  const Token& token() const noexcept {
    return token_;
  }

  bool blank_after() {
    return token_.BlankAfter();
  }

 private:
  friend class AstNodeFactory;

  Token token_;

  CmdPiece(const Token& token, Position position)
      : AstNode(NodeType::kCmdPiece, position)
      , token_(std::move(token)) {}
};

// class to support CmdIoRedirect
class FilePathCmd: public Cmd {
 public:
  ~FilePathCmd() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitFilePathCmd(this);
  }

  std::vector<AstNode*> children() noexcept {
    std::vector<AstNode*> vec;

    for (auto&& piece: pieces_) {
      vec.push_back(piece.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    return pieces_.size();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<AstNode>> pieces_;

  FilePathCmd(std::vector<std::unique_ptr<AstNode>>&& pieces,
              Position position)
      : Cmd(NodeType::kFilePathCmd, position)
      , pieces_(std::move(pieces)) {}
};

class CmdIoRedirectList: public Cmd {
public:
 ~CmdIoRedirectList() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitCmdIoRedirectList(this);
  }

  Cmd* cmd() const noexcept {
   return cmd_.get();
  }

  std::vector<CmdIoRedirect*> children() noexcept {
    std::vector<CmdIoRedirect*> vec;

    for (auto& io: io_list_) {
      vec.push_back(io.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    return io_list_.size();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Cmd> cmd_;
  std::vector<std::unique_ptr<CmdIoRedirect>> io_list_;

  CmdIoRedirectList(std::unique_ptr<Cmd> cmd,
                    std::vector<std::unique_ptr<CmdIoRedirect>> io_list,
                    Position position)
     : Cmd(NodeType::kCmdIoRedirectList, position)
     , cmd_(std::move(cmd))
     , io_list_(std::move(io_list)) {}
};

class CmdIoRedirect: public Cmd {
 public:
  ~CmdIoRedirect() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCmdIoRedirect(this);
  }

  TokenKind kind() const noexcept {
    return token_kind_;
  }

  bool has_integer() const noexcept {
    if (integer_) {
      return true;
    }

    return false;
  }

  //Verify is the output io interface is all interfaces
  // that is: "&>", means stdout and stderr
  bool all() const noexcept {
    return all_;
  }

  // IO interface number: 2> or 1>
  Literal* integer() const noexcept {
    return integer_.get();
  }

  FilePathCmd* file_path_cmd() const noexcept {
    return fp_cmd_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Literal> integer_;
  std::unique_ptr<FilePathCmd> fp_cmd_;
  TokenKind token_kind_;
  bool all_;


  CmdIoRedirect(std::unique_ptr<Literal> integer,
                std::unique_ptr<FilePathCmd> fp_cmd, TokenKind token_kind,
                bool all, Position position)
      : Cmd(NodeType::kCmdIoRedirect, position)
      , integer_(std::move(integer))
      , fp_cmd_(std::move(fp_cmd))
      , token_kind_(token_kind)
      , all_(all) {}
};

class SimpleCmd: public Cmd {
 public:
  ~SimpleCmd() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitSimpleCmd(this);
  }

  std::vector<AstNode*> children() noexcept {
    std::vector<AstNode*> vec;

    for (auto&& piece: pieces_) {
      vec.push_back(piece.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    return pieces_.size();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<AstNode>> pieces_;

  SimpleCmd(std::vector<std::unique_ptr<AstNode>>&& pieces, Position position)
      : Cmd(NodeType::kSimpleCmd, position)
      , pieces_(std::move(pieces)) {}
};

class AliasDeclaration: public Declaration {
 public:
  ~AliasDeclaration() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitAliasDeclaration(this);
  }

  SimpleCmd* cmd() const noexcept {
    return cmd_.get();
  }

  Identifier* name() const noexcept {
    return name_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<SimpleCmd> cmd_;
  std::unique_ptr<Identifier> name_;

  AliasDeclaration(std::unique_ptr<SimpleCmd>&& cmd,
                   std::unique_ptr<Identifier>&& name, Position position)
      : Declaration(NodeType::kAliasDeclaration, position)
      , cmd_(std::move(cmd))
      , name_(std::move(name)) {}
};

class CmdValueExpr: public Cmd {
 public:
  ~CmdValueExpr() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCmdValueExpr(this);
  }

  Expression* expr() const noexcept {
    return expr_.get();
  }

  bool blank_after() const noexcept {
    return blank_after_;
  }

  bool is_iterator() const noexcept {
    return is_iterator_;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> expr_;
  bool blank_after_;
  bool is_iterator_;

  CmdValueExpr(std::unique_ptr<Expression> expr, bool blank_after,
      bool is_iterator, Position position)
      : Cmd(NodeType::kCmdValueExpr, position)
      , expr_(std::move(expr))
      , blank_after_(blank_after)
      , is_iterator_(is_iterator) {}
};

class ForInStatement: public Statement {
 public:
  virtual ~ForInStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitForInStatement(this);
  }

  ExpressionList* exp_list() const noexcept {
    return exp_list_.get();
  }

  ExpressionList* test_list() const noexcept {
    return test_list_.get();
  }

  Statement* block() const noexcept {
    return block_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<ExpressionList> exp_list_;
  std::unique_ptr<ExpressionList> test_list_;
  std::unique_ptr<Statement> block_;

  ForInStatement(std::unique_ptr<ExpressionList> exp_list,
                 std::unique_ptr<ExpressionList> test_list,
                 std::unique_ptr<Statement> block,
                 Position position)
      : Statement(NodeType::kForInStatement, position)
      , exp_list_(std::move(exp_list))
      , test_list_(std::move(test_list))
      , block_(std::move(block)) {}
};

class SwitchStatement: public Statement {
 public:
  virtual ~SwitchStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitSwitchStatement(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

  std::vector<CaseStatement*> case_list() noexcept {
    std::vector<CaseStatement*> vec;

    for (auto&& e: case_list_) {
      vec.push_back(e.get());
    }

    return vec;
  }

  DefaultStatement* default_stmt() const noexcept {
    return default_stmt_.get();
  }

  bool has_default() const noexcept {
    if (default_stmt_) {
      return true;
    }

    return false;
  }

  bool has_exp() const noexcept {
    if (exp_) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;
  std::vector<std::unique_ptr<CaseStatement>> case_list_;
  std::unique_ptr<DefaultStatement> default_stmt_;

  SwitchStatement(std::unique_ptr<Expression> exp,
                  std::vector<std::unique_ptr<CaseStatement>>&& case_list,
                  std::unique_ptr<DefaultStatement> default_stmt,
                  Position position)
      : Statement(NodeType::kSwitchStatement, position)
      , exp_(std::move(exp))
      , case_list_(std::move(case_list))
      , default_stmt_(std::move(default_stmt)) {}
};

class CaseStatement: public Statement {
 public:
  virtual ~CaseStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCaseStatement(this);
  }

  ExpressionList* exp_list() const noexcept {
    return exp_list_.get();
  }

  Block* block() const noexcept {
    return block_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<ExpressionList> exp_list_;
  std::unique_ptr<Block> block_;

  CaseStatement(std::unique_ptr<ExpressionList> exp_list,
                std::unique_ptr<Block> block,
                Position position)
      : Statement(NodeType::kCaseStatement, position)
      , exp_list_(std::move(exp_list))
      , block_(std::move(block)) {}
};

class DelStatement: public Statement {
 public:
  virtual ~DelStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitDelStatement(this);
  }

  ExpressionList* exp_list() noexcept {
    return exp_list_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<ExpressionList> exp_list_;

  DelStatement(std::unique_ptr<ExpressionList> exp_list, Position position)
      : Statement(NodeType::kDelStatement, position)
      , exp_list_(std::move(exp_list)) {}
};

class IfStatement: public Statement {
 public:
  virtual ~IfStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitIfStatement(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

  Statement* then_block() const noexcept {
    return then_block_.get();
  }

  Statement* else_block() const noexcept {
    return else_block_.get();
  }

  bool has_else() const noexcept {
    if (else_block_) { return true; }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;
  std::unique_ptr<Statement> then_block_;
  std::unique_ptr<Statement> else_block_;

  IfStatement(std::unique_ptr<Expression> exp,
              std::unique_ptr<Statement> then_block,
              std::unique_ptr<Statement> else_block,
              Position position)
      : Statement(NodeType::kIfStatement, position)
      , exp_(std::move(exp))
      , then_block_(std::move(then_block))
      , else_block_(std::move(else_block)) {}
};

class WhileStatement: public Statement {
 public:
  virtual ~WhileStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitWhileStatement(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

  Statement* block() const noexcept {
    return block_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;
  std::unique_ptr<Statement> block_;

  WhileStatement(std::unique_ptr<Expression> exp,
              std::unique_ptr<Statement> block,
              Position position)
      : Statement(NodeType::kWhileStatement, position)
      , exp_(std::move(exp))
      , block_(std::move(block)) {}
};


class GlobalAssignmentStatement: public Statement {
 public:
  virtual ~GlobalAssignmentStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitGlobalAssignmentStatement(this);
  }

  AssignmentStatement* assign() const noexcept {
    return assign_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<AssignmentStatement> assign_;

  GlobalAssignmentStatement(std::unique_ptr<AssignmentStatement> assign,
      Position position)
      : Statement(NodeType::kGlobalAssignmentStatement, position)
      , assign_(std::move(assign)) {}
};

class AssignmentStatement: public Statement {
 public:
  virtual ~AssignmentStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitAssignmentStatement(this);
  }

  TokenKind assign_kind() const noexcept {
    return assign_kind_;
  }

  ExpressionList* lexp_list() const noexcept {
    return lexp_.get();
  }

  AssignableList* rvalue_list() const noexcept {
    return rvalue_.get();
  }

  bool has_rvalue() const noexcept {
    if (rvalue_) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  TokenKind assign_kind_;
  std::unique_ptr<ExpressionList> lexp_;
  std::unique_ptr<AssignableList> rvalue_;

  AssignmentStatement(TokenKind assign_kind,
                      std::unique_ptr<ExpressionList> lexp,
                      std::unique_ptr<AssignableList> rvalue,
                      Position position)
      : Statement(NodeType::kAssignmentStatement, position)
      , assign_kind_(assign_kind)
      , lexp_(std::move(lexp))
      , rvalue_(std::move(rvalue)) {}
};

class LetExpression: public Expression {
 public:
  virtual ~LetExpression() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitLetExpression(this);
  }

  AssignmentStatement* assign() const noexcept {
    return assign_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<AssignmentStatement> assign_;

  LetExpression(std::unique_ptr<AssignmentStatement> assign, Position position)
      : Expression(NodeType::kLetExpression, position)
      , assign_(std::move(assign)) {}
};

class ExpressionStatement: public Statement {
 public:
  virtual ~ExpressionStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitExpressionStatement(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;

  ExpressionStatement(std::unique_ptr<Expression> exp, Position position)
      : Statement(NodeType::kExpressionStatement, position)
      , exp_(std::move(exp)) {}
};

class VarEnvStatement: public Statement {
 public:
  virtual ~VarEnvStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitVarEnvStatement(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

  Identifier* var() const noexcept {
    return var_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Identifier> var_;
  std::unique_ptr<Expression> exp_;

  VarEnvStatement(std::unique_ptr<Identifier> var,
      std::unique_ptr<Expression> exp, Position position)
      : Statement(NodeType::kVarEnvStatement, position)
      , var_(std::move(var))
      , exp_(std::move(exp)) {}
};


class BreakStatement: public Statement {
 public:
  virtual ~BreakStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitBreakStatement(this);
  }

 private:
  friend class AstNodeFactory;

  BreakStatement(Position position)
      : Statement(NodeType::kBreakStatement, position) {}
};

class ContinueStatement: public Statement {
 public:
  virtual ~ContinueStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitContinueStatement(this);
  }

 private:
  friend class AstNodeFactory;

  ContinueStatement(Position position)
      : Statement(NodeType::kContinueStatement, position) {}
};

class DefaultStatement: public Statement {
 public:
  virtual ~DefaultStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitDefaultStatement(this);
  }

  Block* block() const noexcept {
    return block_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Block> block_;

  DefaultStatement(std::unique_ptr<Block> block, Position position)
      : Statement(NodeType::kBreakStatement, position)
      , block_(std::move(block)) {}
};

class DeferStatement: public Statement {
 public:
  virtual ~DeferStatement() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitDeferStatement(this);
  }

  Statement* stmt() const noexcept {
    return stmt_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Statement> stmt_;

  DeferStatement(std::unique_ptr<Statement> stmt, Position position)
      : Statement(NodeType::kDeferStatement, position)
      , stmt_(std::move(stmt)) {}
};

class BinaryOperation: public Expression {
 public:
  virtual ~BinaryOperation() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitBinaryOperation(this);
  }

  TokenKind kind() const noexcept {
    return token_kind_;
  }

  Expression* left() const noexcept {
    return left_.get();
  }

  Expression* right() const noexcept {
    return right_.get();
  }

 private:
  friend class AstNodeFactory;

  TokenKind token_kind_;
  std::unique_ptr<Expression> left_;
  std::unique_ptr<Expression> right_;

  BinaryOperation(TokenKind token_kind, std::unique_ptr<Expression> left,
                  std::unique_ptr<Expression> right, Position position)
      : Expression(NodeType::kBinaryOperation, position)
      , token_kind_(token_kind)
      , left_(std::move(left))
      , right_(std::move(right)) {}
};

class Identifier: public Expression {
 public:
  virtual ~Identifier() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitIdentifier(this);
  }

  const std::string& name() const noexcept {
    return name_;
  }

  PackageScope* scope() const noexcept {
    return scope_.get();
  }

  bool has_scope() const noexcept {
    if (scope_) {
      return true;
    }

    return false;
  }

 private:
  friend class AstNodeFactory;

  std::string name_;
  std::unique_ptr<PackageScope> scope_;

  Identifier(const std::string& name, std::unique_ptr<PackageScope> scope,
             Position position)
    : Expression(NodeType::kIdentifier, position)
    , name_(name)
    , scope_(std::move(scope)) {}
};

class VarEnvId: public Expression {
 public:
  virtual ~VarEnvId() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitVarEnvId(this);
  }

  const std::string& name() const noexcept {
    return name_;
  }

 private:
  friend class AstNodeFactory;

  std::string name_;

  VarEnvId(const std::string& name, Position position)
    : Expression(NodeType::kVarEnvId, position)
    , name_(name) {}
};

class PackageScope: public AstNode {
 public:
  virtual ~PackageScope() {}

  virtual void Accept(AstVisitor* visitor) {
   visitor->VisitPackageScope(this);
  }

  Identifier* id() const noexcept {
   return id_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Identifier> id_;

  PackageScope(std::unique_ptr<Identifier> id, Position position)
    : AstNode(NodeType::kPackageScope, position)
    , id_(std::move(id)) {}
};

class NotExpression: public Expression {
 public:
  virtual ~NotExpression() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitNotExpression(this);
  }

  TokenKind kind() const noexcept {
    return token_kind_;
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

 private:
  friend class AstNodeFactory;

  TokenKind token_kind_;
  std::unique_ptr<Expression> exp_;

  NotExpression(TokenKind token_kind, std::unique_ptr<Expression> exp,
                Position position)
      : Expression(NodeType::kNotExpression, position)
      , token_kind_(token_kind)
      , exp_(std::move(exp)) {}
};

class UnaryOperation: public Expression {
 public:
  virtual ~UnaryOperation() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitUnaryOperation(this);
  }

  TokenKind kind() const noexcept {
    return token_kind_;
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

 private:
  friend class AstNodeFactory;

  TokenKind token_kind_;
  std::unique_ptr<Expression> exp_;

  UnaryOperation(TokenKind token_kind, std::unique_ptr<Expression> exp,
                 Position position)
      : Expression(NodeType::kUnaryOperation, position)
      , token_kind_(token_kind)
      , exp_(std::move(exp)) {}
};

class Slice: public Expression {
 public:
 virtual ~Slice() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitSlice(this);
  }

  Expression* start_exp() const noexcept {
    return start_exp_.get();
  }

  Expression* end_exp() const noexcept {
    return end_exp_.get();
  }

  bool has_start_exp() const noexcept {
    if (start_exp_) {
      return true;
    }

    return false;
  }

  bool has_end_exp() const noexcept {
    if (end_exp_) {
      return true;
    }

    return false;
  }

 private:
 friend class AstNodeFactory;

 std::unique_ptr<Expression> start_exp_;
 std::unique_ptr<Expression> end_exp_;

 Slice(std::unique_ptr<Expression> start_exp,
       std::unique_ptr<Expression> end_exp, Position position)
     : Expression(NodeType::kSlice, position)
     , start_exp_(std::move(start_exp))
     , end_exp_(std::move(end_exp)) {}
};

class CompIf: public Expression{
 public:
  virtual ~CompIf() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCompIf(this);
  }

  Expression* comp_exp() const noexcept {
    return comp_exp_.get();
  }

  std::unique_ptr<Expression> MoveCompExp() {
    return std::move(comp_exp_);
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> comp_exp_;

  CompIf(std::unique_ptr<Expression> comp_exp, Position position)
      : Expression(NodeType::kCompIf, position)
      , comp_exp_(std::move(comp_exp)) {}
};

class CompFor: public Expression {
 public:
  virtual ~CompFor() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitCompFor(this);
  }

  ExpressionList* exp_list() const noexcept {
    return exp_list_.get();
  }

  ExpressionList* test_list() const noexcept {
    return test_list_.get();
  }

  std::unique_ptr<ExpressionList> MoveExpList() {
    return std::move(exp_list_);
  }

  std::unique_ptr<ExpressionList> MoveTestList() {
    return std::move(test_list_);
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<ExpressionList> exp_list_;
  std::unique_ptr<ExpressionList> test_list_;

  CompFor(std::unique_ptr<ExpressionList> exp_list,
          std::unique_ptr<ExpressionList> test_list, Position position)
      : Expression(NodeType::kCompFor, position)
      , exp_list_(std::move(exp_list))
      , test_list_(std::move(test_list)) {}
};

class ListComprehension: public Expression {
 public:
  virtual ~ListComprehension() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitListComprehension(this);
  }

  Expression* res_exp() const noexcept {
    return res_exp_.get();
  }

  std::unique_ptr<AssignableValue> MoveResExp() {
    return std::move(res_exp_);
  }

  std::vector<Expression*> comp_list() noexcept {
    std::vector<Expression*> vec;

    for (auto&& e: comp_list_) {
      vec.push_back(e.get());
    }

    return vec;
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<AssignableValue> res_exp_;
  std::vector<std::unique_ptr<Expression>> comp_list_;

  ListComprehension(std::unique_ptr<AssignableValue> res_exp,
      std::vector<std::unique_ptr<Expression>> comp_list, Position position)
      : Expression(NodeType::kListComprehension, position)
      , res_exp_(std::move(res_exp))
      , comp_list_(std::move(comp_list)) {}
};

class Array: public Expression {
 public:
  virtual ~Array() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitArray(this);
  }

  Expression* index_exp() const noexcept {
    return index_exp_.get();
  }

  Expression* arr_exp() const noexcept {
    return arr_exp_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> index_exp_;
  std::unique_ptr<Expression> arr_exp_;

  Array(std::unique_ptr<Expression> arr_exp,
        std::unique_ptr<Expression> index_exp, Position position)
      : Expression(NodeType::kArray, position)
      , index_exp_(std::move(index_exp))
      , arr_exp_(std::move(arr_exp)) {}
};

class IfElseExpression: public Expression {
 public:
  virtual ~IfElseExpression() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitIfElseExpression(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

  Expression* then_exp() const noexcept {
    return then_exp_.get();
  }

  Expression* else_exp() const noexcept {
    return else_exp_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;
  std::unique_ptr<Expression> then_exp_;
  std::unique_ptr<Expression> else_exp_;

  IfElseExpression(std::unique_ptr<Expression> exp,
                   std::unique_ptr<Expression> then_exp,
                   std::unique_ptr<Expression> else_exp, Position position)
      : Expression(NodeType::kIfElseExpression, position)
      , exp_(std::move(exp))
      , then_exp_(std::move(then_exp))
      , else_exp_(std::move(else_exp)) {}
};

class Attribute: public Expression {
 public:
  virtual ~Attribute() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitAttribute(this);
  }

  Expression* exp() const noexcept {
    return exp_.get();
  }

  Identifier* id() const noexcept {
    return id_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> exp_;
  std::unique_ptr<Identifier> id_;

  Attribute(std::unique_ptr<Expression> exp, std::unique_ptr<Identifier> id,
            Position position)
      : Expression(NodeType::kAttribute, position)
      , exp_(std::move(exp))
      , id_(std::move(id)) {}
};

class Argument: public Expression {
 public:
  virtual ~Argument() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitArgument(this);
  }

  AssignableValue* arg() {
    return arg_.get();
  }

  const std::string& key() const noexcept {
    return key_;
  }

  bool has_key() const noexcept {
    return !key_.empty();
  }

 private:
  friend class AstNodeFactory;

  std::string key_;
  std::unique_ptr<AssignableValue> arg_;

  Argument(const std::string& key, std::unique_ptr<AssignableValue> arg,
           Position position)
      : Expression(NodeType::kArgument, position)
      , key_(key)
      , arg_(std::move(arg)) {}
};

class ArgumentsList: public AstNode, public AssignableInterface {
 public:
  virtual ~ArgumentsList() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitArgumentsList(this);
  }

  std::vector<Argument*> children() noexcept {
    std::vector<Argument*> vec;

    for (auto&& node: nodes_) {
      vec.push_back(node.get());
    }

    return vec;
  }

  bool IsEmpty() const noexcept {
    return nodes_.empty();
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<Argument>> nodes_;

  ArgumentsList(std::vector<std::unique_ptr<Argument>>&& nodes,
                Position position)
      : AstNode(NodeType::kArgumentsList, position)
      , nodes_(std::move(nodes)) {}
};

class FunctionCall: public Expression {
 public:
  virtual ~FunctionCall() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitFunctionCall(this);
  }

  Expression* func_exp() {
    return func_exp_.get();
  }

  bool IsRvalueListEmpty() const noexcept {
    return args_list_->IsEmpty();
  }

  ArgumentsList* args_list() {
    return args_list_.get();
  }

 private:
  friend class AstNodeFactory;

  std::unique_ptr<Expression> func_exp_;
  std::unique_ptr<ArgumentsList> args_list_;

  FunctionCall(std::unique_ptr<Expression> func_exp,
               std::unique_ptr<ArgumentsList> args_list, Position position)
      : Expression(NodeType::kFunctionCall, position)
      , func_exp_(std::move(func_exp))
      , args_list_(std::move(args_list)) {}
};

class NullExpression: public Expression {
 public:
  virtual ~NullExpression() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitNullExpression(this);
  }

 private:
  friend class AstNodeFactory;

  NullExpression(Position position)
      : Expression(NodeType::kNullExpression, position) {}
};

class Glob: public Expression {
 public:
  ~Glob() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitGlob(this);
  }

  std::vector<AstNode*> children() noexcept {
    std::vector<AstNode*> vec;

    for (auto&& piece: pieces_) {
      vec.push_back(piece.get());
    }

    return vec;
  }

  size_t num_children() const noexcept {
    return pieces_.size();
  }

  bool recursive() const noexcept {
    return recursive_;
  }

 private:
  friend class AstNodeFactory;

  std::vector<std::unique_ptr<AstNode>> pieces_;
  bool recursive_;

  Glob(std::vector<std::unique_ptr<AstNode>>&& pieces, bool recursive,
       Position position)
      : Expression(NodeType::kGlob, position)
      , pieces_(std::move(pieces))
      , recursive_(recursive) {}
};

class Literal: public Expression {
 public:
  enum Type {
    kString,
    kInteger,
    kReal,
    kBool
  };

  virtual ~Literal() {}

  virtual void Accept(AstVisitor* visitor) {
    visitor->VisitLiteral(this);
  }

  const Token::Value& value() const noexcept {
    return value_;
  }

  Type literal_type() const noexcept {
    return lit_type_;
  }

 private:
  friend class AstNodeFactory;

  Token::Value value_;
  Type lit_type_;

  Literal(const Token::Value& value, Type type, Position position)
    : Expression(NodeType::kLiteral, position)
    , value_(value)
    , lit_type_(type) {}
};

}
}

#include "ast-class-inl.h"
#include "ast-factory-inl.h"

#endif  // SHPP_AST_H
