/* v0.2 itree.h -- AVL-based interval trees
 * 
 * this has been hacked up to support interval trees, searching interval trees, and computing
 * useful information about members of interval trees
 * 
 * this hacked code no longer supports regular binary trees.
 *
 * it is not ready for prime time (documentation needs to be written), but it works fine, and
 * the obvious adjustments to the original AVL test code work as you'd expect them to.
 *
 * TODO: v1.0 should support locking so that it can be used in a multithreaded environment.
 *
 * this initial commit is Copyright (c) 2011 Steve Uurtamo
 *
 * the original AVL tree code is Copyright (c) 2005 Ian Piumarta
 *
 * his (Ian's) copyright notice follows; this modified version falls under the same license:
 *
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the 'Software'), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * provided that the above copyright notice(s) and this permission notice appear
 * in all copies of the Software and that both the above copyright notice(s) and
 * this permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS'.  USE ENTIRELY AT YOUR OWN RISK.
 *
 *
 */

/* This file defines an AVL balanced binary tree [Georgii M. Adelson-Velskii and
 * Evgenii M. Landis, 'An algorithm for the organization of information',
 * Doklady Akademii Nauk SSSR, 146:263-266, 1962 (Russian).  Also in Myron
 * J. Ricci (trans.), Soviet Math, 3:1259-1263, 1962 (English)].
 * 
 * An AVL tree is headed by pointers to the root node and to a function defining
 * the ordering relation between nodes.  Each node contains an arbitrary payload
 * plus three fields per tree entry: the depth of the subtree for which it forms
 * the root and two pointers to child nodes (singly-linked for minimum space, at
 * the expense of direct access to the parent node given a pointer to one of the
 * children).  The tree is rebalanced after every insertion or removal.  The
 * tree may be traversed in two directions: forward (in-order left-to-right) and
 * reverse (in-order, right-to-left).
 * 
 * Because of the recursive nature of many of the operations on trees it is
 * necessary to define a number of helper functions for each type of tree node.
 * The macro TREE_DEFINE(node_tag, entry_name) defines these functions with
 * unique names according to the node_tag.  This macro should be invoked,
 * thereby defining the necessary functions, once per node tag in the program.
 * 
 */

#ifndef __tree_h
#define __tree_h

#define TREE_DELTA_MAX	1

#define TREE_ENTRY(type)			\
  struct {					\
    struct type	*avl_left;			\
    struct type	*avl_right;			\
    struct type	*parent;			\
    int		 avl_height;			\
  }

#define TREE_HEAD(name, type)				\
  struct name {						\
    struct type *th_root;				\
    int  (*th_cmp)(struct type *lhs, struct type *rhs);	\
  }

#define TREE_INITIALIZER(cmp) { 0, cmp }

#define TREE_DELTA(self, field)								\
  (( (((self)->field.avl_left)  ? (self)->field.avl_left->field.avl_height  : 0))	\
   - (((self)->field.avl_right) ? (self)->field.avl_right->field.avl_height : 0))

/* Recursion prevents the following from being defined as macros. */

#define TREE_DEFINE(node, field)									\
                                                                                                        \
struct node *INT_TREE_BALANCE_##node##_##field(struct node *);	                                        \
                                                                                                        \
unsigned int BOOL_INT_INTERSECT_##node##_##field(struct node *, struct node *);                         \
                                                                                                        \
void INT_FIX_MAX_HIGH_##node##_##field(struct node *);                                                  \
                                                                                                        \
unsigned long long INT_MAX_INTERSECT_##node##_##field(struct node *, struct node *);	                \
                                                                                                        \
unsigned long long INT_ALL_INTERSECT_##node##_##field(struct node *, struct node *);	                \
                                                                                                        \
unsigned long long INT_MAX_CONTAINMENT_##node##_##field(struct node *, struct node *);	                \
                                                                                                        \
unsigned long long INT_ALL_CONTAINMENT_##node##_##field(struct node *, struct node *);	                \
                                                                                                        \
struct node *TREE_BALANCE_##node##_##field(struct node *);		                                \
                                                                                                        \
struct node *TREE_ROTL_##node##_##field(struct node *self)						\
  {													\
    struct node *r= self->field.avl_right;								\
    self->field.avl_right= r->field.avl_left;								\
    r->field.avl_left= TREE_BALANCE_##node##_##field(self);						\
    return TREE_BALANCE_##node##_##field(r);								\
  }													\
                                                                                                        \
struct node *TREE_ROTR_##node##_##field(struct node *self)						\
  {													\
    struct node *l= self->field.avl_left;								\
    self->field.avl_left= l->field.avl_right;								\
    l->field.avl_right= TREE_BALANCE_##node##_##field(self);						\
    return TREE_BALANCE_##node##_##field(l);								\
  }													\
                                                                                                        \
struct node *TREE_BALANCE_##node##_##field(struct node *self)						\
  {													\
    int delta= TREE_DELTA(self, field);									\
													\
    if (delta < -TREE_DELTA_MAX)									\
      {													\
	if (TREE_DELTA(self->field.avl_right, field) > 0)						\
	  self->field.avl_right= TREE_ROTR_##node##_##field(self->field.avl_right);			\
	return TREE_ROTL_##node##_##field(self);							\
      }													\
    else if (delta > TREE_DELTA_MAX)									\
      {													\
	if (TREE_DELTA(self->field.avl_left, field) < 0)						\
	  self->field.avl_left= TREE_ROTL_##node##_##field(self->field.avl_left);			\
	return TREE_ROTR_##node##_##field(self);							\
      }													\
    self->field.avl_height= 0;										\
    if (self->field.avl_left && (self->field.avl_left->field.avl_height > self->field.avl_height))	\
      self->field.avl_height= self->field.avl_left->field.avl_height;					\
    if (self->field.avl_right && (self->field.avl_right->field.avl_height > self->field.avl_height))	\
      self->field.avl_height= self->field.avl_right->field.avl_height;					\
    self->field.avl_height += 1;									\
    return self;											\
  }													\
                                                                                                        \
  struct node *TREE_INSERT_##node##_##field								\
    (struct node *self, struct node *elm, int (*compare)(struct node *lhs, struct node *rhs))		\
  {													\
    if (!self)                                                                                          \
      return elm;                                                                                       \
    if (compare(elm, self) < 0)                                                                         \
      self->field.avl_left= TREE_INSERT_##node##_##field(self->field.avl_left, elm, compare);           \
    else                                                                                                \
      self->field.avl_right= TREE_INSERT_##node##_##field(self->field.avl_right, elm, compare);         \
    return TREE_BALANCE_##node##_##field(self);                                                         \
  } 													\
                                                                                                        \
  struct node *TREE_FIND_##node##_##field								\
    (struct node *self, struct node *elm, int (*compare)(struct node *lhs, struct node *rhs))		\
  {													\
    if (!self)												\
      return 0;												\
    if (compare(elm, self) == 0)									\
      return self;											\
    if (compare(elm, self) < 0)										\
      return TREE_FIND_##node##_##field(self->field.avl_left, elm, compare);				\
    else												\
      return TREE_FIND_##node##_##field(self->field.avl_right, elm, compare);				\
  }													\
                                                                                                        \
  struct node *TREE_MOVE_RIGHT(struct node *self, struct node *rhs)					\
  {													\
    if (!self)												\
      return rhs;											\
    self->field.avl_right= TREE_MOVE_RIGHT(self->field.avl_right, rhs);					\
    return TREE_BALANCE_##node##_##field(self);								\
  }													\
                                                                                                        \
  struct node *TREE_REMOVE_##node##_##field								\
    (struct node *self, struct node *elm, int (*compare)(struct node *lhs, struct node *rhs))		\
  {													\
    if (!self) return 0;										\
													\
    if (compare(elm, self) == 0)									\
      {													\
	struct node *tmp= TREE_MOVE_RIGHT(self->field.avl_left, self->field.avl_right);			\
	self->field.avl_left= 0;									\
	self->field.avl_right= 0;									\
	return tmp;											\
      }													\
    if (compare(elm, self) < 0)										\
      self->field.avl_left= TREE_REMOVE_##node##_##field(self->field.avl_left, elm, compare);		\
    else												\
      self->field.avl_right= TREE_REMOVE_##node##_##field(self->field.avl_right, elm, compare);		\
    return TREE_BALANCE_##node##_##field(self);								\
  }													\
                                                                                                        \
void TREE_FORWARD_APPLY_ALL_##node##_##field				                                \
    (struct node *self, void (*function)(struct node *node, void *data), void *data)			\
  {													\
    if (self)												\
      {													\
	TREE_FORWARD_APPLY_ALL_##node##_##field(self->field.avl_left, function, data);			\
	function(self, data);										\
	TREE_FORWARD_APPLY_ALL_##node##_##field(self->field.avl_right, function, data);			\
      }													\
  }													\
                                                                                                        \
void TREE_REVERSE_APPLY_ALL_##node##_##field				                                \
    (struct node *self, void (*function)(struct node *node, void *data), void *data)			\
  {													\
    if (self)												\
      {													\
	TREE_REVERSE_APPLY_ALL_##node##_##field(self->field.avl_right, function, data);			\
	function(self, data);										\
	TREE_REVERSE_APPLY_ALL_##node##_##field(self->field.avl_left, function, data);			\
      }													\
  }                                                                                                     \
                                                                                                        \
struct node *INT_TREE_ROTL_##node##_##field(struct node *self)                                          \
  {                                                                                                     \
    struct node *r= self->field.avl_right;                                                              \
    self->field.avl_right= r->field.avl_left;                                                           \
       INT_FIX_MAX_HIGH_##node##_##field(self);                                                         \
                                                                                                        \
    r->field.avl_left= INT_TREE_BALANCE_##node##_##field(self);                                         \
       INT_FIX_MAX_HIGH_##node##_##field(r);                                                            \
    return INT_TREE_BALANCE_##node##_##field(r);                                                        \
  }                                                                                                     \
                                                                                                        \
struct node *INT_TREE_ROTR_##node##_##field(struct node *self)                                          \
  {                                                                                                     \
    struct node *l= self->field.avl_left;                                                               \
    self->field.avl_left= l->field.avl_right;                                                           \
       INT_FIX_MAX_HIGH_##node##_##field(self);                                                         \
    l->field.avl_right= INT_TREE_BALANCE_##node##_##field(self);                                        \
       INT_FIX_MAX_HIGH_##node##_##field(l);                                                            \
                                                                                                        \
    return INT_TREE_BALANCE_##node##_##field(l);                                                        \
  }                                                                                                     \
                                                                                                        \
struct node *INT_TREE_BALANCE_##node##_##field(struct node *self)                                       \
  {								                                        \
    int delta= TREE_DELTA(self, field);				                                        \
                                                                                                        \
    if (delta < -TREE_DELTA_MAX)				                                        \
      {								                                        \
	if (TREE_DELTA(self->field.avl_right, field) > 0) {                                             \
	  self->field.avl_right= INT_TREE_ROTR_##node##_##field(self->field.avl_right);                 \
	    INT_FIX_MAX_HIGH_##node##_##field(self->field.avl_right);                                   \
        }									                        \
	    INT_FIX_MAX_HIGH_##node##_##field(self);                                                    \
	  return INT_TREE_ROTL_##node##_##field(self);				                        \
      }                                                                                                 \
    else if (delta > TREE_DELTA_MAX)						                        \
      {										                        \
	if (TREE_DELTA(self->field.avl_left, field) < 0) {			                        \
	  self->field.avl_left= INT_TREE_ROTL_##node##_##field(self->field.avl_left);                   \
	    INT_FIX_MAX_HIGH_##node##_##field(self->field.avl_left);                                    \
	}                                                                                               \
	INT_FIX_MAX_HIGH_##node##_##field(self);			                                \
	return INT_TREE_ROTR_##node##_##field(self);				                        \
      }										                        \
    self->field.avl_height= 0;							                        \
    if (self->field.avl_left && (self->field.avl_left->field.avl_height > self->field.avl_height))      \
      self->field.avl_height= self->field.avl_left->field.avl_height;				        \
    if (self->field.avl_right && (self->field.avl_right->field.avl_height > self->field.avl_height))    \
      self->field.avl_height= self->field.avl_right->field.avl_height;				        \
    self->field.avl_height += 1;								        \
                                                                                                        \
    return self;										        \
  }                                                                                                     \
                                                                                                        \
void INT_FIX_MAX_HIGH_##node##_##field(struct node *self)                                               \
  {                                                                                                     \
                                                                                                        \
    long long int max_high=0;                                                                           \
    long long int max_self_high=0;                                                                      \
    long long int max_LC_high=0;                                                                        \
    long long int max_RC_high=0;                                                                        \
                                                                                                        \
    struct node *l;                                                                                     \
    struct node *r;                                                                                     \
    struct node *p;                                                                                     \
    struct node *t;                                                                                     \
                                                                                                        \
                                                                                                        \
    if (self) {                                                                                         \
                                                                                                        \
    /* fixup max_high */                                                                                \
                                                                                                        \
                                                                                                        \
    /*    printf("1\n"); */						\
    max_self_high = self->high;                                                                         \
    /*    printf("3\n"); */						\
    if (self->field.avl_left) {                                                                         \
       max_LC_high   = self->field.avl_left->max_high;				                        \
    }                                                                                                   \
    if (self->field.avl_right) {					                                \
       max_RC_high   = self->field.avl_right->max_high;                                                 \
    }									                                \
    if (max_self_high > max_high) {max_high = max_self_high;}                                           \
    if (max_LC_high   > max_high) {max_high = max_LC_high;}                                             \
    if (max_RC_high   > max_high) {max_high = max_RC_high;}                                             \
                                                                                                        \
    /*    printf("2\n");	*/					\
    /* fixup max_high for self and parents, if max_high needs to be raised */                           \
                                                                                                        \
    if (self->max_high < max_high) {                                                                    \
      self->max_high = max_high;                                                                        \
    }                                                                                                   \
    if (self->field.parent) {                                                                           \
       p=self->field.parent;                                                                            \
                                                                                                        \
       while (p && (p->max_high < max_high)) {                                                          \
          p->max_high=max_high;                                                                         \
          t=p->field.parent;                                                                            \
          p=t;                                                                                          \
       }                                                                                                \
                                                                                                        \
    /* fixup max_high for parents, if max_high needs to be lowered */                                   \
                                                                                                        \
       p=self->field.parent;                                                                            \
                                                                                                        \
       while (p && (p->max_high > max_high)) {                                                          \
                                                                                                        \
         max_high      = 0;                                                                             \
         max_LC_high   = 0;                                                                             \
         max_self_high = self->max_high;                                                                \
                                                                                                        \
         if (p->field.avl_left) {                                                                       \
            max_LC_high   = p->field.avl_left->max_high;                                                \
         }                                                                                              \
                                                                                                        \
         if (max_self_high > max_high) {max_high = max_self_high;}                                      \
         if (max_LC_high   > max_high) {max_high = max_LC_high;}                                        \
                                                                                                        \
         if (p->max_high > max_high) {                                                                  \
	    p->max_high = max_high;                                                                     \
	    t=p->field.parent;                                                                          \
	    p=t;                                                                                        \
         }                                                                                              \
       }                                                                                                \
    }                                                                                                   \
    }                                                                                                   \
  }                                                                                                     \
                                                                                                        \
  struct node *INT_TREE_INSERT_##node##_##field								\
  (struct node *self, struct node *elm, int (*compare)(struct node *lhs, struct node *rhs))             \
  {                                                                                                     \
                                                                                                        \
    struct node *l;                                                                                     \
    struct node *r;                                                                                     \
                                                                                                        \
    if (!self) {								                        \
      elm->max_high = elm->high;						                        \
      return elm;                                                                                       \
    }											                \
    if (compare(elm, self) < 0) {					                                \
      l= INT_TREE_INSERT_##node##_##field(self->field.avl_left, elm, compare);		                \
      l->field.parent=self;                                                                             \
      self->field.avl_left=l;                                                                           \
         INT_FIX_MAX_HIGH_##node##_##field(self->field.avl_left);					\
                                                                                                        \
    }                                                                                                   \
    else {												\
      r= INT_TREE_INSERT_##node##_##field(self->field.avl_right, elm, compare);	                        \
      r->field.parent=self;                                                                             \
      self->field.avl_right=r;                                                                          \
         INT_FIX_MAX_HIGH_##node##_##field(self->field.avl_right);					\
    }                                                                                                   \
                                                                                                        \
       INT_FIX_MAX_HIGH_##node##_##field(self);								\
    return INT_TREE_BALANCE_##node##_##field(self);							\
  }                                                                                                     \
                                                                                                        \
                                                                                                        \
unsigned long long INT_MAX_INTERSECT_##node##_##field                                                   \
    (struct node *self, struct node *elm)                                                               \
  {                                                                                                     \
                                                                                                        \
  unsigned long long local_max, local_int, left_local, right_local;	                                \
                                                                                                        \
    local_max=0;                                                                                        \
    local_int=0;                                                                                        \
    left_local=0;                                                                                       \
    right_local=0;                                                                                      \
                                                                                                        \
    if (!self) {                                                                                        \
      return 0;                                                                                         \
    }                                                                                                   \
    if (self->max_high < elm->low) {                                                                    \
      return 0;                                                                                         \
    }                                                                                                   \
    if ((elm->low <= self->high) && (elm->high >= self->low)) {                                         \
                                                                                                        \
/* left inside, right outside */                                                                        \
                                                                                                        \
if ((elm->low >= self->low) && (elm->high >= self->high)) {                                             \
  local_int=(self->high - elm->low);                                                                    \
 }                                                                                                      \
                                                                                                        \
/* left inside, right inside */                                                                         \
                                                                                                        \
if ((elm->low >= self->low) && (elm->high <= self->high)) {                                             \
  local_int=(elm->high - elm->low);                                                                     \
}                                                                                                       \
                                                                                                        \
/* left outside, right inside */                                                                        \
                                                                                                        \
if ((elm->low <= self->low) && (elm->high <= self->high)) {                                             \
  local_int=(elm->high - self->low);                                                                    \
}                                                                                                       \
                                                                                                        \
/* left outside, right outside */                                                                       \
                                                                                                        \
if ((elm->low <= self->low) && (elm->high >= self->high)) {                                             \
    local_int=(self->high - self->low);					                                \
}                                                                                                       \
                                                                                                        \
 if ((self->field.avl_left) && (BOOL_INT_INTERSECT_##node##_##field(self->field.avl_left,elm))) {       \
    left_local=INT_MAX_INTERSECT_##node##_##field(self->field.avl_left,elm);                            \
 }                                                                                                      \
                                                                                                        \
 if ((self->field.avl_right) && (BOOL_INT_INTERSECT_##node##_##field(self->field.avl_right,elm))) {     \
    right_local=INT_MAX_INTERSECT_##node##_##field(self->field.avl_left,elm);                           \
 }                                                                                                      \
                                                                                                        \
 if (local_int   > local_max) {local_max = local_int;}                                                  \
 if (left_local  > local_max) {local_max = left_local;}                                                 \
 if (right_local > local_max) {local_max = right_local;}                                                \
                                                                                                        \
 /*    printf("E: %llu %llu S: %llu %llu\n",elm->low, elm->high, self->low, self->high); */             \
      return local_max;                                                                                 \
    }                                                                                                   \
                                                                                                        \
    if (self->low > elm->high) {                                                                        \
      return INT_MAX_INTERSECT_##node##_##field(self->field.avl_left, elm);                             \
    }                                                                                                   \
    else {                                                                                              \
      if (INT_MAX_INTERSECT_##node##_##field(self->field.avl_right, elm) == 0) {                        \
	return INT_MAX_INTERSECT_##node##_##field(self->field.avl_left, elm);                           \
      }                                                                                                 \
      else {                                                                                            \
        return INT_MAX_INTERSECT_##node##_##field(self->field.avl_right, elm);                          \
      }                                                                                                 \
    }                                                                                                   \
  }                                                                                                     \
unsigned long long INT_ALL_INTERSECT_##node##_##field                                                   \
    (struct node *self, struct node *elm)                                                               \
  {                                                                                                     \
                                                                                                        \
  unsigned long long local_max, local_int, left_local, right_local;	                                \
                                                                                                        \
    local_max=0;                                                                                        \
    local_int=0;                                                                                        \
    left_local=0;                                                                                       \
    right_local=0;                                                                                      \
                                                                                                        \
    if (!self) {                                                                                        \
      return 0;                                                                                         \
    }                                                                                                   \
    if (self->max_high < elm->low) {                                                                    \
      return 0;                                                                                         \
    }                                                                                                   \
    if ((elm->low <= self->high) && (elm->high >= self->low)) {                                         \
                                                                                                        \
/* left inside, right outside */                                                                        \
                                                                                                        \
if ((elm->low >= self->low) && (elm->high >= self->high)) {                                             \
  local_int=(self->high - elm->low);                                                                    \
  printf("I %llu\n",local_int);                                                                         \
 }                                                                                                      \
                                                                                                        \
/* left inside, right inside */                                                                         \
                                                                                                        \
if ((elm->low >= self->low) && (elm->high <= self->high)) {                                             \
  local_int=(elm->high - elm->low);                                                                     \
  printf("I %llu\n",local_int);                                                                         \
}                                                                                                       \
                                                                                                        \
/* left outside, right inside */                                                                        \
                                                                                                        \
if ((elm->low <= self->low) && (elm->high <= self->high)) {                                             \
  local_int=(elm->high - self->low);                                                                    \
  printf("I %llu\n",local_int);                                                                         \
}                                                                                                       \
                                                                                                        \
/* left outside, right outside */                                                                       \
                                                                                                        \
if ((elm->low <= self->low) && (elm->high >= self->high)) {                                             \
    local_int=(self->high - self->low);					                                \
    printf("I %llu\n",local_int);                                                                       \
}                                                                                                       \
                                                                                                        \
 if ((self->field.avl_left) && (BOOL_INT_INTERSECT_##node##_##field(self->field.avl_left,elm))) {       \
    left_local=INT_ALL_INTERSECT_##node##_##field(self->field.avl_left,elm);                            \
 }                                                                                                      \
                                                                                                        \
 if ((self->field.avl_right) && (BOOL_INT_INTERSECT_##node##_##field(self->field.avl_right,elm))) {     \
    right_local=INT_ALL_INTERSECT_##node##_##field(self->field.avl_left,elm);                           \
 }                                                                                                      \
                                                                                                        \
 if (local_int   > local_max) {local_max = local_int;}                                                  \
 if (left_local  > local_max) {local_max = left_local;}                                                 \
 if (right_local > local_max) {local_max = right_local;}                                                \
                                                                                                        \
 /*    printf("E: %llu %llu S: %llu %llu\n",elm->low, elm->high, self->low, self->high); */             \
      return local_max;                                                                                 \
    }                                                                                                   \
                                                                                                        \
    if (self->low > elm->high) {                                                                        \
      return INT_ALL_INTERSECT_##node##_##field(self->field.avl_left, elm);                             \
    }                                                                                                   \
    else {                                                                                              \
      if (INT_ALL_INTERSECT_##node##_##field(self->field.avl_right, elm) == 0) {                        \
	return INT_ALL_INTERSECT_##node##_##field(self->field.avl_left, elm);                           \
      }                                                                                                 \
      else {                                                                                            \
        return INT_ALL_INTERSECT_##node##_##field(self->field.avl_right, elm);                          \
      }                                                                                                 \
    }                                                                                                   \
  }                                                                                                     \
                                                                                                        \
 /* the query node is elm. the question is whether or not anything from */                              \
 /* self downward in the tree is completely contained by elm            */	                        \
                                                                                                        \
unsigned long long INT_MAX_CONTAINMENT_##node##_##field                                                 \
    (struct node *self, struct node *elm)                                                               \
  {                                                                                                     \
                                                                                                        \
    unsigned long long local_max, local_int, left_local, right_local, local_A, local_B;	                \
                                                                                                        \
    local_max=0;                                                                                        \
    local_int=0;                                                                                        \
    local_A=0;                                                                                          \
    local_B=0;                                                                                          \
    left_local=0;                                                                                       \
    right_local=0;                                                                                      \
                                                                                                        \
    if (!self) {                                                                                        \
      return 0;                                                                                         \
    }                                                                                                   \
    if (self->max_high < elm->low) {                                                                    \
      return 0;                                                                                         \
    }                                                                                                   \
    if ((elm->low <= self->high) && (elm->high >= self->low)) {                                         \
                                                                                                        \
/* left inside, right outside */                                                                        \
                                                                                                        \
if ((elm->low >= self->low) && (elm->high >= self->high)) {                                             \
 local_int=0;                                                                                           \
 }                                                                                                      \
                                                                                                        \
/* left inside, right inside */                                                                         \
                                                                                                        \
if ((elm->low >= self->low) && (elm->high <= self->high)) {                                             \
  local_int=0;                                                                                          \
}                                                                                                       \
                                                                                                        \
/* left outside, right inside */                                                                        \
                                                                                                        \
if ((elm->low <= self->low) && (elm->high <= self->high)) {                                             \
  local_int=0;                                                                                          \
}                                                                                                       \
                                                                                                        \
/* left outside, right outside */                                                                       \
                                                                                                        \
if ((elm->low <= self->low) && (elm->high >= self->high)) {                                             \
                                                                                                        \
local_A=(self->low)-(elm->low);						                                \
local_B=(elm->high)-(self->high);						                        \
if (local_A>local_B) {                                                                                  \
   local_int=local_B;                                                                                   \
}                                                                                                       \
 else {                                                                                                 \
   local_int=local_A;                                                                                   \
 }                                                                                                      \
}                                                                                                       \
                                                                                                        \
 if ((self->field.avl_left) && (BOOL_INT_INTERSECT_##node##_##field(self->field.avl_left,elm))) {       \
    left_local=INT_MAX_CONTAINMENT_##node##_##field(self->field.avl_left,elm);                          \
 }                                                                                                      \
                                                                                                        \
 if ((self->field.avl_right) && (BOOL_INT_INTERSECT_##node##_##field(self->field.avl_right,elm))) {     \
    right_local=INT_MAX_CONTAINMENT_##node##_##field(self->field.avl_left,elm);                         \
 }                                                                                                      \
                                                                                                        \
 if (local_int   > local_max) {local_max = local_int;}                                                  \
 if (left_local  > local_max) {local_max = left_local;}                                                 \
 if (right_local > local_max) {local_max = right_local;}                                                \
                                                                                                        \
 /*    printf("E: %llu %llu S: %llu %llu\n",elm->low, elm->high, self->low, self->high); */             \
      return local_max;                                                                                 \
    }                                                                                                   \
                                                                                                        \
    if (self->low > elm->high) {                                                                        \
      return INT_MAX_CONTAINMENT_##node##_##field(self->field.avl_left, elm);                           \
    }                                                                                                   \
    else {                                                                                              \
      if (INT_MAX_CONTAINMENT_##node##_##field(self->field.avl_right, elm) == 0) {                      \
	return INT_MAX_CONTAINMENT_##node##_##field(self->field.avl_left, elm);                         \
      }                                                                                                 \
      else {                                                                                            \
        return INT_MAX_CONTAINMENT_##node##_##field(self->field.avl_right, elm);                        \
      }                                                                                                 \
    }                                                                                                   \
  }                                                                                                     \
unsigned long long INT_ALL_CONTAINMENT_##node##_##field                                                 \
    (struct node *self, struct node *elm)                                                               \
  {                                                                                                     \
                                                                                                        \
    unsigned long long local_max, local_int, left_local, right_local, local_A, local_B;	                \
                                                                                                        \
    local_max=0;                                                                                        \
    local_int=0;                                                                                        \
    local_A=0;                                                                                          \
    local_B=0;                                                                                          \
    left_local=0;                                                                                       \
    right_local=0;                                                                                      \
                                                                                                        \
    if (!self) {                                                                                        \
      return 0;                                                                                         \
    }                                                                                                   \
    if (self->max_high < elm->low) {                                                                    \
      return 0;                                                                                         \
    }                                                                                                   \
    if ((elm->low <= self->high) && (elm->high >= self->low)) {                                         \
                                                                                                        \
/* left inside, right outside */                                                                        \
                                                                                                        \
if ((elm->low >= self->low) && (elm->high >= self->high)) {                                             \
 local_int=0;                                                                                           \
 }                                                                                                      \
                                                                                                        \
/* left inside, right inside */                                                                         \
                                                                                                        \
if ((elm->low >= self->low) && (elm->high <= self->high)) {                                             \
  local_int=0;                                                                                          \
}                                                                                                       \
                                                                                                        \
/* left outside, right inside */                                                                        \
                                                                                                        \
if ((elm->low <= self->low) && (elm->high <= self->high)) {                                             \
  local_int=0;                                                                                          \
}                                                                                                       \
                                                                                                        \
/* left outside, right outside */                                                                       \
                                                                                                        \
if ((elm->low <= self->low) && (elm->high >= self->high)) {                                             \
local_A=(self->low)-(elm->low);						                                \
local_B=(elm->high)-(self->high);						                        \
if (local_A>local_B) {                                                                                  \
   local_int=local_B;                                                                                   \
}                                                                                                       \
 else {                                                                                                 \
   local_int=local_A;                                                                                   \
 }                                                                                                      \
  printf("I %llu\n",local_int);							                        \
}                                                                                                       \
                                                                                                        \
 if ((self->field.avl_left) && (BOOL_INT_INTERSECT_##node##_##field(self->field.avl_left,elm))) {       \
    left_local=INT_ALL_CONTAINMENT_##node##_##field(self->field.avl_left,elm);                          \
 }                                                                                                      \
                                                                                                        \
 if ((self->field.avl_right) && (BOOL_INT_INTERSECT_##node##_##field(self->field.avl_right,elm))) {     \
    right_local=INT_ALL_CONTAINMENT_##node##_##field(self->field.avl_left,elm);                         \
 }                                                                                                      \
                                                                                                        \
 if (local_int   > local_max) {local_max = local_int;}                                                  \
 if (left_local  > local_max) {local_max = left_local;}                                                 \
 if (right_local > local_max) {local_max = right_local;}                                                \
                                                                                                        \
 /*    printf("E: %llu %llu S: %llu %llu\n",elm->low, elm->high, self->low, self->high); */             \
      return local_max;                                                                                 \
    }                                                                                                   \
                                                                                                        \
    if (self->low > elm->high) {                                                                        \
      return INT_ALL_CONTAINMENT_##node##_##field(self->field.avl_left, elm);                           \
    }                                                                                                   \
    else {                                                                                              \
      if (INT_ALL_CONTAINMENT_##node##_##field(self->field.avl_right, elm) == 0) {                      \
	return INT_ALL_CONTAINMENT_##node##_##field(self->field.avl_left, elm);                         \
      }                                                                                                 \
      else {                                                                                            \
        return INT_ALL_CONTAINMENT_##node##_##field(self->field.avl_right, elm);                        \
      }                                                                                                 \
    }                                                                                                   \
  }                                                                                                     \
                                                                                                        \
struct node *INT_INTERSECT_##node##_##field                                                             \
    (struct node *self, struct node *elm)                                                               \
  {                                                                                                     \
                                                                                                        \
    if (!self) {                                                                                        \
      return 0;                                                                                         \
    }                                                                                                   \
    if (self->max_high < elm->low) {                                                                    \
      return 0;                                                                                         \
    }                                                                                                   \
    if ((elm->low <= self->high) && (elm->high >= self->low)) {                                         \
      return self;                                                                                      \
    }                                                                                                   \
                                                                                                        \
    if (self->low > elm->high) {                                                                        \
      return INT_INTERSECT_##node##_##field(self->field.avl_left, elm);                                 \
    }                                                                                                   \
    else {                                                                                              \
      if (INT_INTERSECT_##node##_##field(self->field.avl_right, elm) == 0) {                            \
	return INT_INTERSECT_##node##_##field(self->field.avl_left, elm);                               \
      }                                                                                                 \
      else {                                                                                            \
        return INT_INTERSECT_##node##_##field(self->field.avl_right, elm);                              \
      }                                                                                                 \
    }                                                                                                   \
  }                                                                                                     \
                                                                                                        \
unsigned int BOOL_INT_INTERSECT_##node##_##field			                                \
    (struct node *self, struct node *elm)                                                               \
  {                                                                                                     \
                                                                                                        \
    if (!self) {                                                                                        \
      return 0;                                                                                         \
    }                                                                                                   \
    if (self->max_high < elm->low) {                                                                    \
      return 0;                                                                                         \
    }                                                                                                   \
    if ((elm->low <= self->high) && (elm->high >= self->low)) {                                         \
      return 1;								                                \
    }                                                                                                   \
                                                                                                        \
    if (self->low > elm->high) {                                                                        \
      return BOOL_INT_INTERSECT_##node##_##field(self->field.avl_left, elm);                            \
    }                                                                                                   \
    else {                                                                                              \
      if (BOOL_INT_INTERSECT_##node##_##field(self->field.avl_right, elm) == 0) {                       \
	return BOOL_INT_INTERSECT_##node##_##field(self->field.avl_left, elm);                          \
      }                                                                                                 \
      else {                                                                                            \
        return 1;                                                                                       \
      }                                                                                                 \
    }                                                                                                   \
  }                                                                                                     \
                                                                                                        \
struct node *INT_TREE_MOVE_RIGHT(struct node *self, struct node *rhs)	                                \
  {													\
    if (!self) {							                                \
      return rhs;											\
    }                                                                                                   \
    self->field.avl_right= INT_TREE_MOVE_RIGHT(self->field.avl_right, rhs);				\
       INT_FIX_MAX_HIGH_##node##_##field(self->field.avl_right);                                        \
       INT_FIX_MAX_HIGH_##node##_##field(self);                                                         \
    return INT_TREE_BALANCE_##node##_##field(self);							\
  }													\
                                                                                                        \
struct node *INT_TREE_REMOVE_##node##_##field				                                \
    (struct node *self, struct node *elm, int (*compare)(struct node *lhs, struct node *rhs))		\
  {													\
    /* printf("B\n"); */							                        \
    if (!self) {return 0;}				                                                \
													\
    if (compare(elm, self) == 0)									\
      {													\
    /* printf("C\n"); */						                                \
	struct node *tmp= INT_TREE_MOVE_RIGHT(self->field.avl_left, self->field.avl_right);		\
	self->field.avl_left= 0;									\
	self->field.avl_right= 0;									\
	/* printf("F\n"); */						                                \
	   INT_FIX_MAX_HIGH_##node##_##field(self);		                                        \
	/* printf("G\n");  */                                                                           \
	   INT_FIX_MAX_HIGH_##node##_##field(tmp);                                                      \
	/* printf("H\n"); */                                                                            \
                                                                                                        \
	return tmp;											\
      }													\
    if (compare(elm, self) < 0)	{					                                \
      /*     printf("D\n"); */						                                \
      self->field.avl_left= INT_TREE_REMOVE_##node##_##field(self->field.avl_left, elm, compare);	\
         INT_FIX_MAX_HIGH_##node##_##field(self->field.avl_left);                                       \
    }                                                                                                   \
    else {								                                \
      /*     printf("E\n"); */						                                \
      self->field.avl_right= INT_TREE_REMOVE_##node##_##field(self->field.avl_right, elm, compare);	\
         INT_FIX_MAX_HIGH_##node##_##field(self->field.avl_right);                                      \
    }                                                                                                   \
    return INT_TREE_BALANCE_##node##_##field(self);							\
  }													\

#define TREE_INSERT(head, node, field, elm)						                \
  ((head)->th_root= TREE_INSERT_##node##_##field((head)->th_root, (elm), (head)->th_cmp))

#define INT_TREE_INSERT(head, node, field, elm)						                \
  ((head)->th_root= INT_TREE_INSERT_##node##_##field((head)->th_root, (elm), (head)->th_cmp))

#define TREE_FIND(head, node, field, elm)				                                \
  (TREE_FIND_##node##_##field((head)->th_root, (elm), (head)->th_cmp))

#define INT_INTERSECT(head, node, field, elm)				                                \
  (INT_INTERSECT_##node##_##field((head)->th_root, (elm)))

#define BOOL_INT_INTERSECT(head, node, field, elm)				                        \
  (BOOL_INT_INTERSECT_##node##_##field((head)->th_root, (elm)))

#define INT_MAX__INTERSECT(head, node, field, elm)				                        \
  (INT_MAX_INTERSECT_##node##_##field((head)->th_root, (elm)))

#define INT_MAX__CONTAINMENT(head, node, field, elm)				                        \
  (INT_MAX_CONTAINMENT_##node##_##field((head)->th_root, (elm)))

#define TREE_REMOVE(head, node, field, elm)						                \
  ((head)->th_root= TREE_REMOVE_##node##_##field((head)->th_root, (elm), (head)->th_cmp))
#define INT_TREE_REMOVE(head, node, field, elm)						                \
  ((head)->th_root= INT_TREE_REMOVE_##node##_##field((head)->th_root, (elm), (head)->th_cmp))

#define TREE_DEPTH(head, field)			                                                        \
  ((head)->th_root->field.avl_height)

#define TREE_FORWARD_APPLY(head, node, field, function, data)	                                        \
  TREE_FORWARD_APPLY_ALL_##node##_##field((head)->th_root, function, data)

#define TREE_REVERSE_APPLY(head, node, field, function, data)	                                        \
  TREE_REVERSE_APPLY_ALL_##node##_##field((head)->th_root, function, data)

#define TREE_INIT(head, cmp) do {		                                                        \
    (head)->th_root= 0;				                                                        \
    (head)->th_cmp= (cmp);			                                                        \
  } while (0)

#endif /* __tree_h */
