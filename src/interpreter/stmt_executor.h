#ifndef SETI_STMT_EXECUTOR_H
#define SETI_STMT_EXECUTOR_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <tuple>

#include "ast/ast.h"
#include "obj_type.h"
#include "executor.h"
#include "object-factory.h"

namespace setti {
namespace internal {

class StmtListExecutor: public Executor {
 public:
  StmtListExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack), stop_flag_(StopFlag::kGo) {}

  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;

 private:
  StopFlag stop_flag_;
};

class FuncDeclExecutor: public Executor {
 public:
  FuncDeclExecutor(Executor* parent, SymbolTableStack& symbol_table_stack,
                   bool method = false)
      : Executor(parent, symbol_table_stack)
      , obj_factory_(symbol_table_stack)
      , method_(method) {}

  void Exec(AstNode* node);

  ObjectPtr FuncObj(AstNode* node);

  void set_stop(StopFlag flag) override;

 private:
  ObjectFactory obj_factory_;
  bool method_;
};

class ClassDeclExecutor: public Executor {
 public:
  ClassDeclExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , obj_factory_(symbol_table_stack) {}

  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;

 private:
  ObjectFactory obj_factory_;
};

class StmtExecutor: public Executor {
 public:
  StmtExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute expression
  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;
};

class BlockExecutor: public Executor {
 public:
  // the last parameter on Executor constructor means this is NOT the
  // root executor
  BlockExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack, false) {}

  void Exec(AstNode* node) {
    Block* block_node = static_cast<Block*>(node);
    StmtListExecutor executor(this, symbol_table_stack());
    executor.Exec(block_node->stmt_list());
  }

  void set_stop(StopFlag flag) override {
    if (parent() == nullptr) {
      return;
    }

    parent()->set_stop(flag);
  }
};

class ReturnExecutor: public Executor {
 public:
  ReturnExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack)
      , obj_factory_(symbol_table_stack) {}

  // Entry point to execute expression
  void Exec(AstNode* node);

  void set_stop(StopFlag flag) override;

 private:
  ObjectFactory obj_factory_;
};

class IfElseExecutor: public Executor {
 public:
  IfElseExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute expression
  void Exec(IfStatement* node);

  void set_stop(StopFlag flag) override;
};

class WhileExecutor: public Executor {
 public:
  WhileExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute while
  void Exec(WhileStatement* node);

  void set_stop(StopFlag flag) override;

 protected:
  bool inside_loop() override {
    return true;
  }

  bool inside_switch() override {
    return false;
  }

 private:
  StopFlag stop_flag_;
};

class ForInExecutor: public Executor {
 public:
  ForInExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  // Entry point to execute for in
  void Exec(ForInStatement *node);

  void Assign(std::vector<std::reference_wrapper<ObjectPtr>>& vars,
              std::vector<ObjectPtr>& it_values);

  void set_stop(StopFlag flag) override;

 protected:
  bool inside_loop() override {
    return true;
  }

  bool inside_switch() override {
    return false;
  }

 private:
  StopFlag stop_flag_;
};

class BreakExecutor: public Executor {
 public:
  BreakExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  void Exec(BreakStatement *node);

  void set_stop(StopFlag flag) override;
};

class ContinueExecutor: public Executor {
 public:
  ContinueExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  void Exec(ContinueStatement *node);

  void set_stop(StopFlag flag) override;
};

}
}

#endif  // SETI_STMT_EXECUTOR_H


