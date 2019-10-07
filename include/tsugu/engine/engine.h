
#ifndef TSUGU_ENGINE_ENGINE_H
#define TSUGU_ENGINE_ENGINE_H

#include <tsugu/core/ast.h>
#include <tsugu/core/tyenv.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t tsg_engine_run_ast(tsg_ast_t* ast, tsg_tyenv_t* tyenv);

#ifdef __cplusplus
}
#endif

#endif
