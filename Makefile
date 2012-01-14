INCLUDES	:= -I ./include -I ./libcds/includes

all: clean
	make -C tests

clean:
