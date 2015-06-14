#pragma once

#include <string.h>
#include "lib_tools.h"

#ifdef	LIB_MEM
typedef struct s_lib_mem_reference	*t_lib_mem_reference;
typedef struct s_lib_mem_list		*t_lib_mem_list;

struct s_lib_mem_list
{
  void			*target;
  t_lib_mem_list	next;
};

struct s_lib_mem_reference
{
  void			*address;
  t_lib_mem_list	dependencies;
  t_lib_mem_list	parents;
  bool			is_returned;
  int			allowed_depth;
  t_lib_mem_reference	next;
};
#else
# define	free(value)	error("Warning: this program is using a garbage collector. You probably shouldn't use free for " #value ". If you really want to use it, use _free instead.")
#endif

#ifndef	LIB_MEM_STACK_COPY_SIZE
# define LIB_MEM_STACK_COPY_SIZE 100
#endif

#define	managed(function)	*lib_mem_type_hint_ ## function;	\
  static typeof(*lib_mem_type_hint_ ## function)	lib_mem_hide_ ## function (); \
  typeof(*lib_mem_type_hint_ ## function)	function ()		\
  {									\
    void	*args = __builtin_apply_args();				\
    void	*result;						\
									\
    lib_mem_increase_function_depth();					\
    result = __builtin_apply(						\
			     (void (*)()) &lib_mem_hide_ ## function,	\
			     args, LIB_MEM_STACK_COPY_SIZE);		\
    lib_mem_decrease_function_depth();					\
    lib_mem_check_references();						\
    __builtin_return(result);						\
  }									\
									\
  static typeof(*lib_mem_type_hint_ ## function)	lib_mem_hide_ ## function

#define return(value) {							\
    typeof(value) lib_mem_local_return_val = value;			\
    									\
    lib_mem_set_return((void *) ((long) lib_mem_local_return_val));	\
    return (lib_mem_local_return_val);					\
  }

#define	$proto(function)	function lib_mem_no_params
#define lib_mem_no_params(...)	()

#define	$mem_link(what, with)	lib_mem_link_value((what), (with))
#define	$mem_unlink(what, with)	lib_mem_unlink(what, with)
#define $mem_preserve(what)	lib_mem_set_return(what)
#define	$mem_flush	lib_mem_decrease_function_depth();	\
  lib_mem_check_references();					\
  lib_mem_decrease_function_depth
#define $mem_var		__attribute__ ((__cleanup__(lib_mem_remove_links)))

void	_free(void *);

void	lib_mem_check_references() __attribute__ ((destructor));
void	lib_mem_set_return(void *);
void	*lib_mem_link_value(void *, void *);
void	lib_mem_unlink(void *, void *);
void	lib_mem_remove_links(void *);

/* warning: you probably do not want to use the two next functions. */
void	lib_mem_increase_function_depth();
void	lib_mem_decrease_function_depth();

bool	lib_mem_is_managed_variable(void *) nnull;

#define	mem(variable)	lib_mem_holder_variable_ ## variable;		\
  typeof(lib_mem_holder_variable_ ## variable)	variable = malloc(sizeof(*lib_mem_holder_variable_ ## variable)); \
									\
  if (variable == NULL)							\
    {									\
      error("Cannot allocate memory.");					\
      return 0;								\
    }
#define local_mem(variable)	lib_mem_holder_variable_ ## variable;	\
  typeof(lib_mem_holder_variable_ ## variable)	variable = malloc(sizeof(*lib_mem_holder_variable_ ## variable)); \
									\
  if (variable == NULL)							\
    {									\
      error("Cannot allocate memory.");					\
      return ;								\
    }

#define memz(variable) mem(variable); memset(variable, 0, sizeof(*variable))
#define local_memz(variable) local_mem(variable); memset(variable, 0, sizeof(*variable))
