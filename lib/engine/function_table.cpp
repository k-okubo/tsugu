
#include "function_table.h"

using namespace tsugu;

void FunctionTable::set(tsg_func_t* func, tsg_tyenv_t* env,
                        llvm::Function* instance) {
  table[func][env] = instance;
}

llvm::Function* FunctionTable::get(tsg_func_t* func, tsg_tyenv_t* env) {
  return table[func][env];
}
