# Lib MEM

Lib MEM is a basic garbage collector written for C. It use a _huge_ amount of gcc-specifics features and thus should not work with any other compiler. It also use Lib Tools, a header-only library in the same repository.

## How does it works ?

Lib MEM use counting references to check if memory should (or not) be freed. Each time you allocate (with malloc) memory, it will be traced and added to the garbage collector. Then, by the end of your function, all no longer needed variables will be removed.

## How to use it ?

First thing to do is to define your function as managed - Lib MEM will do nothing on non-managed functions, since they are not supposed to be on it responsability. To mark a function as managed, simply brace it's name with managed() :

```c
void	* managed(my_very_own_function)(int myparam)
{
	/* whatever you want here */
}
```

There is also a little limit, due to macros' limit in C : when you use return in a managed function, you are required to use parenthesis around the returned value, like this :

```c
void	*managed(my_very_own_function)(int myparam)
{
  void	*ret;
  
  ret = malloc(myparam);
  /* whatever you want here */
  return (ret); /* And not return ret; */
}
```

This will be enough for most of the basic usage, as "allocating memory" and "returning memory". As long as you don't do fancy stuff like out parameters or structures with your newly allocated memory inside, it will be enough.

Note that even if you can use non-managed function in managed ones, it's a (seriously) bad idea to use a managed function in a non-managed function. *TL;DR* : only use managed functions in your program.

Another note is that if you do use a prototype of your function - IE something like this :

```c
void	my_function(int toto);
```

and if this function is managed (and will use managed(my_function) when declared) you must use $proto around it, like this :

```c
void	$proto(my_function)(int toto);
```

### Fancy stuff 1: Allocated memory inside other memories

In order to be able to put some memory inside some other (allocated or not) memory, you must link those two. You can use $mem_link to link two elements, probably on attribution, like this :

```c
my_pretty_struct->field = $mem_link(my_element, my_pretty_struct);
```

my_element will then be linked with my_pretty_struct, meanings that as long as the two are linked, my_element will not be freed. Note that if my_pretty_struct is not allocated, you must either unlink it or manage it when not used (if you don't, you will have a memory leak).

There is two way to do so :
- Unlink it manually : when the link is no longer needed, either because you removed the variable from the structure (or assigned a new one) or because the structure is going to be remove, you can use $mem_unlink, like this :

```c
void	managed(my_pretty_function)(void *something)
{
  t_my_type	my_struct;

  my_struct.element = $mem_link(something, &my_struct);
  /* Do something */
  $mem_unlink(something, &my_struct);
  my_struct.element = NULL;
}
```

- Manage your non-allocated memory : if your variable is meant to hold managed memory, and you want to free yourself from checking the content, you can also mark the variable as a memory variable using the attribute $mem_var. Note that this is quite time-consuming, so you probably want to use it only when really needed :

```c
void	managed(my_pretty_function)(void *something)
{
  $mem_var t_my_type	my_struct;

  my_struct.element = $mem_link(something, &my_struct);
  /* Do something */
  /* Note that you no longer need $mem_unlink : my_struct is managed. */
  /* Note again that this does not apply if you manually change the value : you still need
     to unlink your variables if you reassign my_struct.element ! */
}
```

### Fancy stuff 2: Out variables

Sometimes, you want to pass a variable by reference and as such put a result (let's say, a pointer to allocated memory). This pointer will not be detected as such by Lib MEM. You will have to manually add a reference to this pointer, using $mem_preserve :

```c
int	managed(my_function)(void *out_value)
{
  out_value = malloc(32);
  $mem_preserve(out_value);
  return (42);
}
```

Note that there is no need to do anything other than $mem_preserve (except if you use your out_value in another out_value...) and that you need to use $mem_preserve AFTER assigning your out_value's value.

### Fancy stuff 3: Infinite loop

In some programs, you will use a infinite loop. This will prevent your program to be correctly checked for memory. To avoid memory leak, you can use $mem_flush to "emulate" the end of your function :

```c
void	managed(my_loop_function)()
{
  while (1)
  {
    /* Do some stuff */
    $mem_flush();
  }
}
```

Note that it's probably a very, very bad idea to use mem_flush in anything else than an infinite loop. Seriously, trust me on this one.

### Other tools

Lib MEM also define two helper macro to reduce the amount of code needed to declare a variable. Those two macros are mem() (for a returning function) and local_mem (for a non-returning function).

Basically, mem() assume that your function is returning a pointer, and will malloc your variable, then test it. If the malloc did fail, it will print and error and return null, elsewise it will let your function continue.

local_mem() is doing exactly the same, but for a non-returning function.

Both uses are quite easy :

```c
int	*managed(my_pretty_function)()
{
  int	*mem(my_var);

  return (my_var);
}
```

```c
void	managed(my_pretty_function)()
{
  int	*local_mem(my_var);

  /* Do something with my_var */
}
```

You can also use memz() and local_memz(), who behave exactly the same as mem() and local_mem(), with the only exception that they also memset() the allocated memory to zero.

Those macros are only meant as helper tool, and are not in any way needed to use Lib MEM.