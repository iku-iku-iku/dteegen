.PHONY: all clean deploy generate test_project generate_cpp build_in_docker docker build_compile_deps build_docker build_target

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

build_in_docker:
	bash ./scripts/build_in_docker.sh

build_compile_deps:
	bash ./scripts/build_compile_deps.sh
docker:
	bash ./scripts/update_docker_deps.sh
	docker build -t rv-secgear .
build_docker:
	docker build -t rv-secgear .

build_target: 
	@echo "Building $(TARGET)"
	make build_compile_deps
	sudo make docker
	sudo ./scripts/gen_target.sh $(TARGET)
	sudo ./scripts/build_in_docker.sh $(TARGET).generated

