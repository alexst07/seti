#ifndef SETI_CMD_EXECUTOR_H
#define SETI_CMD_EXECUTOR_H

#include <tuple>

#include "executor.h"
#include "cmd-exec.h"

namespace setti {
namespace internal {

typedef std::tuple<int, std::string, std::string> CmdExprData;

class CmdExecutor: public Executor {
 public:
  CmdExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  CmdExprData ExecGetResult(CmdFull *node);

  CmdExprData ExecCmdGetResult(Cmd *node);

  CmdExprData ExecCmdBinOp(CmdAndOr* cmd);

  int ExecCmdBinOp(CmdAndOr* cmd, bool wait);

  int Exec(CmdFull *node);

  int ExecCmd(Cmd *node, bool wait);

  int ExecSimpleCmd(SimpleCmd *node, bool wait);

  CmdExprData ExecSimpleCmdWithResult(SimpleCmd *node);

  std::string CmdOutput() const;

 private:
  std::string cmd_output_;
};

class SimpleCmdExecutor: public Executor {
 public:
  SimpleCmdExecutor(Executor* parent, SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  std::vector<std::string> Exec(SimpleCmd *node);
};

class CmdIoRedirectListExecutor: public Executor {
 public:
  CmdIoRedirectListExecutor(Executor* parent,
                            SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  int Exec(CmdIoRedirectList *node, bool wait);

  CmdExprData Exec(CmdIoRedirectList *node);

  int GetInteger(Literal* integer);

  static std::string FileName(Executor *parent, FilePathCmd* file_path);

  void PrepareData(Job &job, CmdIoRedirectList *node);
};

class CmdPipeSequenceExecutor: public Executor {
 public:
  CmdPipeSequenceExecutor(Executor* parent,
                            SymbolTableStack& symbol_table_stack)
      : Executor(parent, symbol_table_stack) {}

  int Exec(CmdPipeSequence *node, bool wait);
  CmdExprData Exec(CmdPipeSequence *node);
  void PopulateCmd(Job& job, CmdPipeSequence *node);
  void InputFile(CmdIoRedirectList* file, Job &job);
  void OutputFile(CmdIoRedirectList* cmd_io, Job &job);
  void SelectInterface(CmdIoRedirect* io, Job& job, int fd);
  int GetInteger(Literal* integer);
  void AddCommand(Job& job, Cmd *cmd);
};

// functions to manipulate command
std::string ResolveCmdExpr(Executor *parent, CmdValueExpr *cmd_expr);

// functions to manipulate file
int CreateFile(std::string file_name);
int AppendFile(std::string file_name);
int ReadFile(std::string file_name);

}
}

#endif  // SETI_CMD_EXECUTOR_H

