OUT=main.out
SRC=main.cpp
HEAD=

$(OUT): $(SRC) $(HEAD)
	clang++ -Wall -std=c++11 -stdlib=libc++ -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)

install: $(OUT)
	install $(OUT) /usr/local/bin/dmake

clean:
	rm -rf *.out *.o $(OUT)
