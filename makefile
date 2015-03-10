OUT=main.out
SRC=main.cpp
HEAD=

$(OUT): $(SRC) $(HEAD)
	clang++ -Wall -std=c++11 -stdlib=libc++ -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)

clean:
	rm -rf *.out *.o
