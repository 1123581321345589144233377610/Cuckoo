SHELL := /bin/bash

DPDK_DIR = dpdk
APP = test_hash

all: $(DPDK_DIR)/build $(APP)
	@echo "Build complete"

$(DPDK_DIR)/build:
	@echo "Cloning DPDK..."
	git clone --depth 1 https://github.com/DPDK/dpdk.git $(DPDK_DIR)
	@echo "Building DPDK..."
	cd $(DPDK_DIR) && meson setup build
	cd $(DPDK_DIR) && ninja -C build

$(APP): test/main.c
	@echo "Compiling test..."
	gcc -Wall -Wextra -Werror -I$(DPDK_DIR)/build/include $< -o $@ \
		-L$(DPDK_DIR)/build/lib \
		-lrte_hash -lrte_eal -lrte_kvargs -lrte_mempool -lrte_ring -lrte_malloc \
		-lnuma -ldl -pthread

run: $(APP)
	@echo "Running test..."
	sudo ./$(APP) --no-huge -l 0-3 --in-memory

clean:
	rm -rf $(DPDK_DIR) $(APP)

.PHONY: all run clean