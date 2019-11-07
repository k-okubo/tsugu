/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file frame.h
 *
 ** --------------------------------------------------------------------------*/

#ifndef TSUGU_CORE_FRAME_H
#define TSUGU_CORE_FRAME_H

#include <tsugu/core/tyenv.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tsg_frame_s tsg_frame_t;
typedef struct tsg_member_s tsg_member_t;
typedef struct tsg_member_node_s tsg_member_node_t;

struct tsg_frame_s {
  tsg_frame_t* outer;
  int32_t depth;
  int32_t size;
  tsg_member_node_t* head;
  tsg_member_node_t* tail;
};

struct tsg_member_s {
  int32_t depth;
  int32_t index;
  tsg_tyvar_t* tyvar;
};

struct tsg_member_node_s {
  tsg_member_t* member;
  tsg_member_node_t* next;
};

tsg_frame_t* tsg_frame_create(tsg_frame_t* outer);
void tsg_frame_destroy(tsg_frame_t* frame);

tsg_member_t* tsg_frame_add_member(tsg_frame_t* frame);

#ifdef __cplusplus
}
#endif

#endif
