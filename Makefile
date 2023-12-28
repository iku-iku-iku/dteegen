.PHONY: all clean

all:
	bash ./scripts/build_codegen.sh

clean:
	rm -rf build
