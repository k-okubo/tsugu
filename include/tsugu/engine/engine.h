/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file engine.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_ENGINE_ENGINE_H
#define TSUGU_ENGINE_ENGINE_H

#include <tsugu/core/ast.h>
#include <tsugu/core/tyenv.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t tsg_engine_run_ast(tsg_ast_t* ast);

#ifdef __cplusplus
}
#endif

#endif
