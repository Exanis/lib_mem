NAME_STATIC=	build/static/libmem.a
NAME_SHARED=	build/shared/libmem.so


SRC=	lib_mem.c

OBJ=	$(SRC:.c=.o)

CC=	gcc

CFLAGS+=	-W -Wall -Werror -O2 -fno-optimize-sibling-calls -fno-omit-frame-pointer -I.

TESTING_SRC=	testing/testing.c
TESTING_OBJ=	$(TESTING_SRC:.c=.o)
TESTING_NAME_SHARED=	shared_test
TESTING_NAME_STATIC=	static_test
TESTING_LIB=	mem

$(NAME_STATIC):	$(OBJ)
	mkdir -p build/static
	ar rc $(NAME_STATIC) $(OBJ)
	ranlib $(NAME_STATIC)

$(NAME_SHARED):	LDFLAGS+= -shared
$(NAME_SHARED):	$(OBJ)
	mkdir -p build/shared
	$(CC) $(LDFLAGS) -o $(NAME_SHARED) $(OBJ)

all:	$(NAME_STATIC) $(NAME_SHARED)

static:	$(NAME_STATIC)

shared: $(NAME_SHARED)

testing:	CFLAGS += -ggdb
testing:	$(NAME_SHARED) $(NAME_STATIC) $(TESTING_OBJ)
	$(CC) $(TESTING_OBJ) -o $(TESTING_NAME_STATIC) -Lbuild/static -l$(TESTING_LIB) 
	@echo "====== STATIC TESTS ======"
	./$(TESTING_NAME_STATIC)
	$(CC) $(TESTING_OBJ) -o $(TESTING_NAME_SHARED) -Lbuild/shared -l$(TESTING_LIB)
	@echo "====== SHARED TESTS ======"
	env LD_LIBRARY_PATH=build/shared ./$(TESTING_NAME_SHARED)

clean:
	rm -rf $(OBJ) $(TESTING_OBJ)

fclean:	clean
	rm -rf $(NAME_STATIC) $(NAME_SHARED) $(TESTING_NAME_STATIC) $(TESTING_NAME_SHARED) build

re:	fclean all

.PHONY: all install clean fclean re static shared testing