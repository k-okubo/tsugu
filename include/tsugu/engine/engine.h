
#ifndef TSUGU_ENGINE_ENGINE_H
#define TSUGU_ENGINE_ENGINE_H

#include <tsugu/core/ast.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t tsg_engine_run_ast(tsg_ast_t* ast);

#ifdef __cplusplus
}
#endif

#endif
