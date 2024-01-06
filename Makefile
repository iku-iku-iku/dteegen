.PHONY: all clean deploy generate test_project generate_cpp

all:
	bash ./scripts/build_codegen.sh

clean:
	rm -rf build

generate: all
	./build/codegen test_project

generate_cpp: all
	./build/codegen test_project_cpp

deploy: 
	tar -zcvf generated.tar.gz generated
	scp -P 12055 generated.tar.gz root@localhost:~/

test_project: 
	bash ./scripts/build_test_project.sh
