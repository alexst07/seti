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

#ifndef SHPP_MSG_H
#define SHPP_MSG_H

#include <string>
#include <memory>
#include <vector>
#include <boost/format.hpp>

namespace shpp {
namespace internal {

class Message {
 public:
  enum class Severity {
    INFO,
    WARNING,
    ERR
  };

  Message() = delete;

  Message(Severity severity, const boost::format& msg, unsigned int line, unsigned int pos)
      : severity_(severity)
      , msg_(msg)
      , line_(line)
      , pos_(pos) {}

  Message(const Message& msg)
      : severity_(msg.severity_)
      , msg_(msg.msg_)
      , line_(msg.line_)
      , pos_(msg.pos_)
      , str_line_error_(msg.str_line_error_)
      , file_(msg.file_) {}

  Message& operator=(const Message& msg) {
    severity_ = msg.severity_;
    msg_ = msg.msg_;
    line_ = msg.line_;
    pos_ = msg.pos_;
    str_line_error_ = msg.str_line_error_;
    file_ = msg.file_;

    return *this;
  }

  std::string msg() {
    return msg_.str();
  }

  unsigned int line() {
    return line_;
  }

  unsigned int pos() {
    return pos_;
  }

  void file(const std::string& str_file) {
    file_ = str_file;
  }

  std::string file() const {
    return file_;
  }

  void line_error(const std::string& str_line) {
    str_line_error_ = str_line;
  }

  std::string line_error() const {
    return str_line_error_;
  }

  friend std::ostream& operator<<(std::ostream& stream, const Message& msg);
  friend std::ostream& operator<<(std::ostream& stream, Message& msg);

 private:
  Severity severity_;
  boost::format msg_;
  unsigned int line_;
  unsigned int pos_;
  std::string str_line_error_;
  std::string file_;
};

inline std::ostream& operator<<(std::ostream& stream, const Message& msg) {
  std::string severity;

  switch (msg.severity_) {
    case Message::Severity::INFO:
      severity = "info";
      break;

    case Message::Severity::WARNING:
      severity = "warning";
      break;

    case Message::Severity::ERR:
      severity = "error";
      break;
  }

  stream << msg.line_ << ":" << msg.pos_ << ": " << severity << ": " << msg.msg_ << '\n';

  return stream;
}

inline std::ostream& operator<<(std::ostream& stream, Message& msg) {
  std::string severity;

  switch (msg.severity_) {
    case Message::Severity::INFO:
      severity = "info";
      break;

    case Message::Severity::WARNING:
      severity = "warning";
      break;

    case Message::Severity::ERR:
      severity = "info";
      break;
  }

  stream << msg.line_ << ":" << msg.pos_ << ": " << severity << ": " << msg.msg_ << '\n';

  return stream;
}

class Messages {
 public:
  using iterator = std::vector<Message>::iterator;
  using const_iterator = std::vector<Message>::const_iterator;

  Messages() = default;

  Messages(Messages&& msg):msg_vec_(std::move(msg.msg_vec_)) {}

  Messages& operator=(Messages&& msg) {
    msg_vec_ = std::move(msg.msg_vec_);
    return *this;
  }

  Messages(const Messages& msg):msg_vec_(msg.msg_vec_) {}

  Messages& operator=(const Messages& msg) {
    msg_vec_ = msg.msg_vec_;
    return *this;
  }

  void Push(Message&& msg) {
    msg_vec_.push_back(std::move(msg));
  }

  iterator begin() {
    return msg_vec_.begin();
  }

  iterator end() {
    return msg_vec_.end();
  }

  const_iterator begin() const {
    return msg_vec_.begin();
  }

  const_iterator end() const {
    return msg_vec_.end();
  }

  size_t size() const {
    return msg_vec_.size();
  }

 private:
  std::vector<Message> msg_vec_;
};

}
}

#endif  // SETTI_MSG_H
