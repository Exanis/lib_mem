#include <malloc.h>
#include "lib_mem.h"

#undef free

static void	(*lib_mem_testing_free_initial_hook)(void *, const void *);
static void	lib_mem_testing_free_hook(void *, unused const void *);

static void	initialize_testing_hook()
{
  lib_mem_testing_free_initial_hook = __free_hook;
  __free_hook = &lib_mem_testing_free_hook;
}

static void	*preserve_addr = NULL;
static bool	should_be_freed = false;
static bool	test_result = false;

static void	lib_mem_testing_free_hook(void *obj, unused const void *caller)
{
  if (obj == preserve_addr) {
    test_result = should_be_freed;
  }
  __free_hook = lib_mem_testing_free_initial_hook;
  free(obj);
  __free_hook = &lib_mem_testing_free_hook;
}

#define	test(what, description)	run_test(&what, #what, description)

static void	run_test(void (*func)(), char *test_name, char *test_description)
{
  fprintf(stderr, "[%s] %s:\t\t", test_name, test_description);
  test_result = false;
  func();
  if (test_result == false)
    fprintf(stderr, "\033[1;31mFAIL\033[0m\n");
  else
    fprintf(stderr, "Ok\n");
}

static void	managed(lib_mem_testing_malloc_1)()
{
  preserve_addr = malloc(1);
  should_be_freed = true;
}

static void	managed(lib_mem_testing_malloc_2_1)()
{
  // Note : this function does nothing.
}

static void	managed(lib_mem_testing_malloc_2)()
{
  preserve_addr = malloc(1);
  should_be_freed = false;
  lib_mem_testing_malloc_2_1();
  should_be_freed = true;
}

static void	*managed(lib_mem_testing_malloc_3_1)()
{
  preserve_addr = malloc(1);
  return (preserve_addr);
}

static void	managed(lib_mem_testing_malloc_3)()
{
  should_be_freed = false;
  lib_mem_testing_malloc_3_1();
  should_be_freed = true;
}

static void	managed(lib_mem_testing_malloc_4)()
{
  should_be_freed = false;
  lib_mem_testing_malloc_3_1();
  lib_mem_testing_malloc_2_1();
  should_be_freed = true;
}

static int	managed(lib_mem_testing_return_1_1)()
{
  return (1);
}

static void	managed(lib_mem_testing_return_1)()
{
  test_result = lib_mem_testing_return_1_1() == 1;
}

static int	managed(lib_mem_testing_return_2_1)()
{
  if (true)
    return (1);
  return (2);
}

static int	managed(lib_mem_testing_return_2_2)()
{
  if (false)
    return (1);
  return (2);
}

static void	managed(lib_mem_testing_return_2)()
{
  test_result = lib_mem_testing_return_2_1() == 1 && lib_mem_testing_return_2_2() == 2;
}

static void	managed(lib_mem_testing_auto_malloc_1)()
{
  int	*local_mem(val);

  preserve_addr = val;
  should_be_freed = true;
}

static int	*managed(lib_mem_testing_auto_malloc_2_1)()
{
  int	*mem(val);

  preserve_addr = val;
  return (val);
}

static void	managed(lib_mem_testing_auto_malloc_2)()
{
  should_be_freed = false;
  if (lib_mem_testing_auto_malloc_2_1() == 0)
    return ;
  should_be_freed = true;
}

static void	managed(lib_mem_testing_auto_malloc_3)()
{
  int	*local_memz(val);

  should_be_freed = *val == 0;
  preserve_addr = val;
}

static int	*managed(lib_mem_testing_auto_malloc_4_1)()
{
  int	*memz(val);

  preserve_addr = val;
  return (val);
}

static void	managed(lib_mem_testing_auto_malloc_4)()
{
  int	*val;

  should_be_freed = false;
  val = lib_mem_testing_auto_malloc_4_1();
  should_be_freed = *val == 0;
}

static void	managed(lib_mem_testing_preserve_1_1)(int *ptr)
{
  ptr = malloc(sizeof(*ptr));
  $mem_preserve(ptr);
  preserve_addr = ptr;
}

static void	managed(lib_mem_testing_preserve_1)()
{
  int	*ptr = NULL;

  should_be_freed = false;
  lib_mem_testing_preserve_1_1(ptr);
  should_be_freed = true;
}

static void	managed(lib_mem_testing_preserve_2)()
{
  int	*ptr = NULL;
  
  should_be_freed = false;
  lib_mem_testing_preserve_1_1(ptr);
  lib_mem_testing_malloc_2_1();
  should_be_freed = true;
}

static void	managed(lib_mem_testing_preserve_3_1)(int *param)
{
  $mem_preserve(param);
}

static void	managed(lib_mem_testing_preserve_3)()
{
  int	*ptr = NULL;

  should_be_freed = false;
  lib_mem_testing_preserve_1_1(ptr);
  lib_mem_testing_preserve_3_1(ptr);
  should_be_freed = true;
}

static void	managed(lib_mem_testing_link_1_1)(int *with)
{
  int	*local_mem(what);

  preserve_addr = what;
  $mem_link(what, with);
}

static void	managed(lib_mem_testing_link_1)()
{
  int	*local_mem(ptr);
  
  should_be_freed = false;
  lib_mem_testing_link_1_1(ptr);
  should_be_freed = true;
}

static void	managed(lib_mem_testing_link_2)()
{
  $mem_var int	my_int = 3;

  should_be_freed = false;
  lib_mem_testing_link_1_1(&my_int);
  should_be_freed = true;
}

static void	managed(lib_mem_testing_link_3)()
{
  int	my_int = 3;

  should_be_freed = false;
  lib_mem_testing_link_1_1(&my_int);
  $mem_unlink(preserve_addr, &my_int);
  should_be_freed = true;
}

static void	*managed(lib_mem_testing_link_4_1)()
{
  int	*mem(third);
  int	*mem(second);
  int	*mem(first);

  preserve_addr = $mem_link(third, second);
  $mem_link(second, first);
  return (first);
}

static void	managed(lib_mem_testing_link_4)()
{
  should_be_freed = false;
  lib_mem_testing_link_4_1();
  should_be_freed = true;
}

static void	managed(lib_mem_testing_flush)()
{
  int	*local_mem(ptr);

  should_be_freed = true;
  $mem_flush();
  should_be_freed = false;
}

int	main()
{
  initialize_testing_hook();
  test(lib_mem_testing_malloc_1, "Basic use case");
  test(lib_mem_testing_malloc_2, "Basic use case (with subfunction)");
  test(lib_mem_testing_malloc_3, "Basic use case, malloc in subfunction");
  test(lib_mem_testing_malloc_4, "Basic use case, malloc in subfunction, intermediate function");
  test(lib_mem_testing_return_1, "Return test, basic use without malloc");
  test(lib_mem_testing_return_2, "Return test, basic use without malloc in if without bracket");
  test(lib_mem_testing_auto_malloc_1, "Auto-malloc using local_mem");
  test(lib_mem_testing_auto_malloc_2, "Auto-malloc using mem");
  test(lib_mem_testing_auto_malloc_3, "Auto-malloc using local_memz");
  test(lib_mem_testing_auto_malloc_4, "Auto-malloc using memz");
  test(lib_mem_testing_preserve_1, "Out variable pointing to malloc");
  test(lib_mem_testing_preserve_2, "Out variable pointing to malloc (with subfunction)");
  test(lib_mem_testing_preserve_3, "Out variable in multiple functions");
  test(lib_mem_testing_link_1, "Linked variables");
  test(lib_mem_testing_link_2, "Linked variables, link to a non-allocated variable using $mem_var");
  test(lib_mem_testing_link_3, "Linked variables, manual unlink");
  test(lib_mem_testing_link_4, "Linked variables, multiple links");
  test(lib_mem_testing_flush, "Flush variables");
  return 0;
}
