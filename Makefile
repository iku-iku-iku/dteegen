.PHONY: all clean deploy

all:
	bash ./scripts/build_codegen.sh

clean:
	rm -rf build

deploy: all
	./build/codegen test_project
	tar -zcvf generated.tar.gz generated
	scp -P 12055 generated.tar.gz root@localhost:~/
