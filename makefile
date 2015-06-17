OUT=main.out
SRC=main.cpp helpers.cpp config.cpp args.cpp
HEAD=helpers.h config.h args.h

$(OUT): $(SRC) $(HEAD)
	clang++ -Wall -std=c++11 -stdlib=libc++ -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)

install: $(OUT)
	install $(OUT) /usr/local/bin/dmake

clean:
	rm -rf *.out *.o $(OUT)
