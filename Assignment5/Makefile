all: main.out animation.out

main.out: src/main.cpp renderer.out
	clang++ src/main.cpp -o main.out renderer.out
animation.out: src/animation.cpp renderer.out
	clang++ src/animation.cpp -o animation.out renderer.out

renderer.out: src/renderer.h
	clang++ src/renderer.h -o renderer.out