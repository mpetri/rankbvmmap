INCLUDES	:= -I ./include -I ./libcds/includes

all: clean tests bench

tests:
	make -C tests

bench:
	make -C bench

clean:
