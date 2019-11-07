/*--------------------------------------- vi: set ft=c ts=2 sw=2 et: --*-c-*--*/
/**
 * @file frame.c
 *
 ** --------------------------------------------------------------------------*/

#include <tsugu/core/frame.h>

#include <tsugu/core/memory.h>
#include <tsugu/core/platform.h>

static tsg_member_t* tsg_member_create(tsg_frame_t* frame);
static void tsg_member_destroy(tsg_member_t* member);

tsg_frame_t* tsg_frame_create(tsg_frame_t* outer) {
  tsg_frame_t* frame = tsg_malloc_obj(tsg_frame_t);

  if (outer != NULL) {
    frame->depth = outer->depth + 1;
  } else {
    frame->depth = 0;
  }

  frame->outer = outer;
  tsg_assert(frame->depth >= 0);
  frame->size = 0;
  frame->head = NULL;
  frame->tail = NULL;

  return frame;
}

void tsg_frame_destroy(tsg_frame_t* frame) {
  if (frame == NULL) {
    return;
  }

  tsg_member_node_t* node = frame->head;
  while (node != NULL) {
    tsg_member_node_t* next = node->next;
    tsg_member_destroy(node->member);
    tsg_free(node);
    node = next;
  }

  tsg_free(frame);
}

tsg_member_t* tsg_frame_add_member(tsg_frame_t* frame) {
  tsg_member_node_t* node = tsg_malloc_obj(tsg_member_node_t);
  tsg_member_t* member = tsg_member_create(frame);

  node->member = member;
  node->next = NULL;

  if (frame->head == NULL) {
    frame->head = node;
    frame->tail = node;
  } else {
    frame->tail->next = node;
    frame->tail = node;
  }

  return member;
}

tsg_member_t* tsg_member_create(tsg_frame_t* frame) {
  tsg_assert(frame != NULL);

  tsg_member_t* member = tsg_malloc_obj(tsg_member_t);
  member->depth = frame->depth;
  member->index = frame->size;
  member->tyvar = NULL;

  frame->size += 1;

  return member;
}

void tsg_member_destroy(tsg_member_t* member) {
  if (member == NULL) {
    return;
  }

  tsg_tyvar_destroy(member->tyvar);
  tsg_free(member);
}
