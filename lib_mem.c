#define LIB_MEM
#include <malloc.h>
#include "lib_mem.h"

static void	lib_mem_init_hook(void);
static void	*lib_mem_malloc_hook(size_t, const void *);

static void	*(*lib_mem_malloc_initial_hook)(size_t, const void *);

void (*__malloc_initialize_hook)(void) = lib_mem_init_hook;

static t_lib_mem_reference	lib_mem_references;
static int			lib_mem_function_depth = 0;

void lib_mem_increase_function_depth()
{
  lib_mem_function_depth++;
}

void lib_mem_decrease_function_depth()
{
  lib_mem_function_depth--;
}

static void lib_mem_init_hook(void)
{
  lib_mem_malloc_initial_hook = __malloc_hook;
  __malloc_hook = &lib_mem_malloc_hook;
}

static void *lib_mem_malloc_hook(size_t size, unused const void *parent)
{
  void	*result;
  t_lib_mem_reference	reference;

  __malloc_hook = lib_mem_malloc_initial_hook;
  result = malloc(size);
  if (!result)
    return NULL;
  reference = malloc(sizeof(*reference));
  if (!reference)
    {
      free(result);
      return NULL;
    }
  reference->dependencies = NULL;
  reference->parents = NULL;
  reference->address = result;
  reference->next = lib_mem_references;
  reference->allowed_depth = lib_mem_function_depth;
  lib_mem_references = reference;
  __malloc_hook = &lib_mem_malloc_hook;
  return result;
}

static t_lib_mem_reference	lib_mem_find_reference(void *address)
{
  t_lib_mem_reference	list = lib_mem_references;

  while (list)
    {
      if (list->address == address)
	return list;
      list = list->next;
    }
  return NULL;
}

static void lib_mem_remove_from_list(t_lib_mem_list *list, void *what)
{
  t_lib_mem_list	element = *list;
  t_lib_mem_list	prev = NULL;

  while (element)
    {
      t_lib_mem_list	next = element->next;

      if (element->target == what)
	{
	  if (prev == NULL)
	    *list = next;
	  else
	    prev->next = next;
	  free(element);
	}
      element = next;
    }
}

static void lib_mem_unlink_internal(t_lib_mem_reference from, void *to)
{
  t_lib_mem_reference to_ref = lib_mem_find_reference(to);
  
  if (to_ref)
    lib_mem_remove_from_list(&(to_ref->dependencies), from);
  lib_mem_remove_from_list(&(from->parents), to);
}

static bool lib_mem_remove_reference(t_lib_mem_reference over)
{
  t_lib_mem_reference prev = NULL;
  bool		      ret = true;

  if (over != lib_mem_references)
    while (prev == NULL || prev->next != over)
      {
	if (prev == NULL)
	  prev = lib_mem_references;
	else
	  prev = prev->next;
      }
  if (prev != NULL)
    prev->next = over->next;
  else
    lib_mem_references = over->next;
  while (over->dependencies != NULL)
    {
      t_lib_mem_list next = over->dependencies->next;

      lib_mem_unlink_internal(over->dependencies->target, over->address);
      over->dependencies = next;
      ret = false;
    }
  while (over->parents != NULL)
    {
      t_lib_mem_list next = over->parents->next;

      lib_mem_unlink_internal(over, over->parents->target);
      over->parents = next;
    }
  free(over->address);
  free(over);
  return ret;
}

void	lib_mem_check_references()
{
  t_lib_mem_reference	list;
  bool			check_done = false;

  while (!check_done)
    {
      check_done = true;
      list = lib_mem_references;
      while (list)
	{
	  t_lib_mem_reference next = list->next;

	  if (list->allowed_depth > lib_mem_function_depth
	      && list->parents == NULL 
	      && !list->is_returned)
	    check_done = lib_mem_remove_reference(list);
	  else if (list->is_returned)
	    list->is_returned = 0;
	  list = next;
	}
    }
}

void	lib_mem_remove_links(void *what)
{
  t_lib_mem_reference	list = lib_mem_references;
  t_lib_mem_reference	what_ref = lib_mem_find_reference(what);

  if (!what_ref)
    while (list)
      {
	if (list->parents) {
	  lib_mem_remove_from_list(&(list->parents), what);
	}
	list = list->next;
      }
}

void	lib_mem_set_return(void *what)
{
  t_lib_mem_reference	ref = lib_mem_find_reference(what);

  if (ref)
    {
      ref->is_returned = true;
      ref->allowed_depth = lib_mem_function_depth - 1;
    }
}

void	*lib_mem_link_value(void *what, void *with)
{
  t_lib_mem_reference	what_ref = lib_mem_find_reference(what);
  t_lib_mem_reference	with_ref = lib_mem_find_reference(with);

  __malloc_hook = lib_mem_malloc_initial_hook;
  if (what_ref && with_ref)
    {
      t_lib_mem_list	element = malloc(sizeof(*element));

      if (element)
	{
	  element->target = what_ref;
	  element->next = with_ref->dependencies;
	  with_ref->dependencies = element;
	}
    }
  if (what_ref)
    {
      t_lib_mem_list	element = malloc(sizeof(*element));
      
      if (element)
	{
	  element->target = with;
	  element->next = what_ref->parents;
	  what_ref->parents = element;
	}
    }
  __malloc_hook = &lib_mem_malloc_hook;
  return what;
}

void	lib_mem_unlink(void *what, void *with)
{
  t_lib_mem_reference	what_ref = lib_mem_find_reference(what);
  
  if (what_ref)
    lib_mem_unlink_internal(what_ref, with);
}

void	_free(void *what)
{
  t_lib_mem_reference	what_ref = lib_mem_find_reference(what);

  if (what_ref)
    lib_mem_remove_reference(what_ref);
  else
    free(what);
}

bool	lib_mem_is_managed_variable(void *what)
{
  return lib_mem_find_reference(what) != NULL;
}

