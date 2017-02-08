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

#ifndef SETI_FUNC_OBJ_H
#define SETI_FUNC_OBJ_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <list>

#include "run_time_error.h"
#include "obj-type.h"

namespace seti {
namespace internal {

class FuncObject: public Object {
 public:
  FuncObject(ObjectPtr obj_type, SymbolTableStack&& sym_table)
      : Object(ObjectType::FUNC, obj_type, std::move(sym_table)) {}

  virtual ~FuncObject() {}

  std::size_t Hash() const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("func object has no hash method"));
  }

  bool operator==(const Object& obj) const override {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("func object has no compare method"));
  }

  std::string Print() override {
    return std::string("[function]");
  }
};

class FuncWrapperObject: public FuncObject {
 public:
  FuncWrapperObject(ObjectPtr obj_type, ObjectPtr func, ObjectPtr self,
                    SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , func_(func)
      , self_(self) {}

  virtual ~FuncWrapperObject() {}

  ObjectPtr Call(Executor* parent, std::vector<ObjectPtr>&& params) override;

 private:
  ObjectPtr func_;
  ObjectPtr self_;
};

class FuncDeclObject: public FuncObject {
 public:
  FuncDeclObject(const std::string& id, std::shared_ptr<Block> start_node,
                 const SymbolTableStack& symbol_table,
                 std::vector<std::string>&& params,
                 std::vector<ObjectPtr>&& default_values,
                 bool variadic, bool lambda, ObjectPtr obj_type,
                 SymbolTableStack&& sym_table)
      : FuncObject(obj_type, std::move(sym_table))
      , id_(id)
      , start_node_(start_node)
      , symbol_table_()
      , params_(std::move(params))
      , default_values_(std::move(default_values))
      , variadic_(variadic)
      , lambda_(lambda) {
    symbol_table_.Push(symbol_table.MainTable(), true);

    if (lambda) {
      if (symbol_table.HasClassTable()) {
        auto stack = symbol_table.GetUntilClassTable();
        symbol_table_.Append(std::move(stack));
      } else {
        // if has function copy symbol table until the function
        // if not, copy all stack of symbol table
        auto stack = symbol_table.GetUntilFuncTable();
        symbol_table_.Append(std::move(stack));
      }
    }
  }

  ObjectPtr Call(Executor* parent, std::vector<ObjectPtr>&& params) override;

  void HandleArguments(std::vector<ObjectPtr>&& params);

 private:
  std::string id_;

  // the start_node_ is a pointer on ast, on interactive mode the
  // ast can be free and the func object be keep on symbol table
  // so shared_ptr avoid lost this pointer
  std::shared_ptr<Block> start_node_;
  SymbolTableStack symbol_table_;
  std::vector<std::string> params_;
  std::vector<ObjectPtr> default_values_;
  bool variadic_;
  bool lambda_;
};

}
}

#endif  // SETI_FUNC_OBJ_H