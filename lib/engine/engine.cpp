/*----------------------------------- vi: set ft=cpp ts=2 sw=2 et: --*-cpp-*--*/
/**
 * @file engine.cpp
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/engine/engine.h>

#include "compiler.h"

int32_t tsg_engine_run_ast(tsg_ast_t* ast, tsg_tyenv_t* tyenv) {
  tsugu::Compiler compiler;
  int32_t ret = compiler.run(ast, tyenv);
  return ret;
}
