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

#ifndef SHPP_ABSTRACT_OBJ_H
#define SHPP_ABSTRACT_OBJ_H

#include <string>
#include <memory>
#include <unordered_map>
#include <tuple>
#include <list>

#include "run_time_error.h"
#include "interpreter/symbol-table.h"

namespace shpp {
namespace internal {

class Executor;

class Object {
 public:
  using Args = std::vector<std::shared_ptr<Object>>;
  using KWArgs = std::unordered_map<std::string, std::shared_ptr<Object>>;

  enum class ObjectType: unsigned int8_t {
    ROOT,
    NIL,
    INT,
    BOOL,
    REAL,
    STRING,
    SLICE,
    ARRAY,
    MAP,
    TUPLE,
    FUNC,
    SPEC_FUNC,
    CMD,
    REGEX,
    PATH,
    FILE,
    TYPE,
    ARRAY_ITER,
    MAP_ITER,
    RANGE_ITER,
    TUPLE_ITER,
    CMD_ITER,
    FILE_ITER,
    DECL_IFACE,
    DECL_TYPE,
    DECL_OBJ,
    MODULE,
    EXCEPT,
    CUSTON
  };

  virtual ~Object() {}

  inline ObjectType type() const {
    return type_;
  }

  virtual std::string Print() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no print interface")
                       %ObjType()->ObjectName());
  }

  virtual long int Len() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no len interface")
                       %ObjType()->ObjectName());
  }

  virtual std::size_t Hash() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no hash interface")
                       %ObjType()->ObjectName());
  }

  virtual bool operator==(const Object& /*obj*/) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no equal method")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> ObjBool() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no bool interface")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> ObjString() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no string interface")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> ObjInt() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no int interface")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> ObjReal() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no real interface")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> ObjCmd() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no cmd interface")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> ObjArray() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no array interface")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Call(Executor*, Args&&,
                                       KWArgs&& = KWArgs()) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no call interface")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> GetItem(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no get_item interface")
                       %ObjType()->ObjectName());
  }

  virtual void SetItem(std::shared_ptr<Object>,
      std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no set_item interface")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object>& GetItemRef(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no get_item interface")
                       %ObjType()->ObjectName());
  }

  // This method must receive the self object to apply the iterator
  virtual std::shared_ptr<Object> ObjIter(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no iter interface")
                       %ObjType()->ObjectName());
  }

  virtual void DelItem(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has del operation")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Add(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no + operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Sub(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no - operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Mult(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no * operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Div(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no / operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> DivMod(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no % operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> RightShift(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no >> operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> LeftShift(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no << operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Lesser(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no < operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Greater(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no > operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Copy() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no copy method")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Next() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no next method")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> HasNext() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no has_next method")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> LessEqual(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no >= operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> GreatEqual(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no <= operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Equal(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no == operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> In(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no in operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> NotEqual(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no == operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> BitAnd(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no & operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> BitOr(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no | operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> BitXor(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no ^ operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> BitNot() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no ~ operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> And(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no && operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Or(std::shared_ptr<Object>) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no && operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> UnaryAdd() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no unary + operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> UnarySub() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no unary - operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Not() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no not operator")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Begin() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no begin method")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> End() {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no end method")
                       %ObjType()->ObjectName());
  }

  virtual std::shared_ptr<Object> Attr(std::shared_ptr<Object>,
                                        const std::string&) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no attr method")
                       %ObjType()->ObjectName());
  }

  // this method must be used when arrow operation is on left side
  virtual std::shared_ptr<Object>& AttrAssign(std::shared_ptr<Object>,
                                        const std::string&) {
    throw RunTimeError(RunTimeError::ErrorCode::INCOMPATIBLE_TYPE,
                       boost::format("%1% has no attr assign method")
                       %ObjType()->ObjectName());
  }

  virtual std::string ObjectName() {
    return std::string("type");
  }

  virtual std::shared_ptr<Object> ObjType() const noexcept {
    return obj_type_.lock();
  }

  SymbolTableStack& symbol_table_stack() {
    return sym_table_;
  }

  virtual std::shared_ptr<Object> BaseType() noexcept {
    if (!base_) {
      return symbol_table_stack().LookupSys("object").SharedAccess();
    }

    return base_;
  }

  const std::vector<std::shared_ptr<Object>>& Interfaces() const noexcept {
    return ifaces_;
  }

 private:
  // enum type
  ObjectType type_;

  // type of object, it is other object
  std::weak_ptr<Object> obj_type_;

  SymbolTableStack sym_table_;

  std::shared_ptr<Object> base_;

  std::vector<std::shared_ptr<Object>> ifaces_;

 protected:
  Object(ObjectType type, std::shared_ptr<Object> obj_type,
         SymbolTableStack&& sym_table,
         std::shared_ptr<Object> base = std::shared_ptr<Object>(nullptr),
         std::vector<std::shared_ptr<Object>>&& ifaces =
            std::vector<std::shared_ptr<Object>>())
      : type_(type)
      , obj_type_(obj_type)
      , sym_table_(std::move(sym_table))
      , base_(base)
      , ifaces_(std::move(ifaces)) {}
};

typedef std::shared_ptr<Object> ObjectPtr;

}
}

#endif  // SHPP_ABSTRACT_OBJ_H
