build:
	@g++ -o main.out src/main.c src/grammar.c src/ll1.c src/util.c

debug:
	@g++ -g -o main.out src/main.c src/grammar.c src/ll1.c src/util.c && gdb ./main.out

build-run: build
	@./main.out

run:
	@./main.out

valgrind: build
	@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./main.out 
