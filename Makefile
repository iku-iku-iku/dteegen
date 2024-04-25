.PHONY: all debug clean perf deploy generate test_project generate_cpp build_in_docker docker build_compile_deps run_docker build_target build_target_raw push_docker

all:
	./scripts/build_dteegen.sh release

debug:
	./scripts/build_dteegen.sh debug

clean:
	rm -rf build

perf:
	sudo perf record --call-graph dwarf ./build/dteegen ./test/test_seal

generate: all
	./build/dteegen test_project

generate_cpp: all
	./build/dteegen test_project_cpp

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
	docker build --network=host -t dteegen .
push_docker:
	docker tag dteegen registry.cn-hangzhou.aliyuncs.com/dteegen/dteegen:1.0.3
	docker push registry.cn-hangzhou.aliyuncs.com/dteegen/dteegen:1.0.3
run_docker:
	docker run -v $(shell pwd):/test --network=host -w /test -it dteegen


build_target: 
	@echo "Building $(TARGET)"
	make build_compile_deps
	sudo make docker
	sudo ./scripts/gen_target.sh $(TARGET)
	sudo ./scripts/build_in_docker.sh $(TARGET).generated

build_target_raw: 
	@echo "Building $(TARGET)"
	sudo ./scripts/gen_target.sh $(TARGET)
	sudo ./scripts/build_in_docker.sh $(TARGET).generated

deploy_to_oe:
	scp ./test/template_project_distributed_tee.generated/build/client root@192.168.1.152:~/client_test
	scp ./test/template_project_distributed_tee.generated/build/compute_node root@192.168.1.151:~/server_test

