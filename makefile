OUT=main.out
SRC=main.cpp helpers.cpp config.cpp args.cpp
HEAD=helpers.h config.h args.h
ifdef __clang__
	CC=clang++
else ifdef clang
	CC=clang++ -stdlib=libc++
else
	CC=g++
endif

$(OUT): $(SRC) $(HEAD)
	$(CC) -std=c++11 -Wall -Wno-sign-compare -Ofast -o $(OUT) $(SRC)

run: $(OUT)
	./$(OUT)

install: $(OUT)
	install $(OUT) /usr/local/bin/dmake

clean:
	rm -rf *.out *.o $(OUT)
