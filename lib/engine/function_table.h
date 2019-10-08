
#ifndef TSUGU_ENGINE_FUNCTION_TABLE_H
#define TSUGU_ENGINE_FUNCTION_TABLE_H

#include <tsugu/core/ast.h>
#include <tsugu/core/tyenv.h>
#include <llvm/IR/Function.h>
#include <unordered_map>

namespace tsugu {

class FunctionTable {
 public:
  FunctionTable() : table() {}
  virtual ~FunctionTable() {}

  void set(tsg_func_t* func, tsg_tyenv_t* env, llvm::Function* instance);
  llvm::Function* get(tsg_func_t* func, tsg_tyenv_t* env);

 private:
  typedef std::unordered_map<tsg_tyenv_t*, llvm::Function*> env_tbl_t;
  typedef std::unordered_map<tsg_func_t*, env_tbl_t> func_tbl_t;

  func_tbl_t table;
};

}  // namespace tsugu

#endif
