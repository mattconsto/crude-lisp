all:
	g++ -Wall --std=c++14 lisp.cpp -o lisp

clean:
	rm lisp.exe lisp.o lisp
