INCLUDES	:= -I ./CppUnitLite -I ../include
COMMON		:= ./CppUnitLite/*.cpp test-main.cpp

all: clean rankbvTest run

rankbvTest:
	g++ -Wall -g -o rankbvTest $(INCLUDES) $(COMMON) ../src/rankbv.c rankbvTest.cpp

run:
	./rankbvTest

clean:
	rm -f ./rankbvTest
