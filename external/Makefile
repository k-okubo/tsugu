EXTERNAL_SRC_DIR := src

LLVM_VER := 9.0.0
LLVM_SRC_URL := http://releases.llvm.org/$(LLVM_VER)/llvm-$(LLVM_VER).src.tar.xz
LLVM_TAR_FILE := $(EXTERNAL_SRC_DIR)/llvm-$(LLVM_VER).src.tar.xz
LLVM_SRC_DIR := $(EXTERNAL_SRC_DIR)/llvm-$(LLVM_VER).src
LLVM_INSTALL_DIR := llvm-$(LLVM_VER)
LLVM_INSTALL_PATH := $(abspath $(LLVM_INSTALL_DIR))

llvm: $(LLVM_INSTALL_DIR)

$(LLVM_INSTALL_DIR): $(LLVM_SRC_DIR)/build/CMakeCache.txt
	cd $(LLVM_SRC_DIR)/build \
		&& make -j`nproc` \
		&& make install

$(LLVM_SRC_DIR)/build/CMakeCache.txt: $(LLVM_SRC_DIR)
	cd $(LLVM_SRC_DIR) \
		&& mkdir -p build \
		&& cd build \
		&& cmake \
			-DCMAKE_BUILD_TYPE=Release \
			-DCMAKE_INSTALL_PREFIX=$(LLVM_INSTALL_PATH) \
			-DLLVM_BUILD_EXAMPLES=OFF \
			-DLLVM_INCLUDE_EXAMPLES=OFF \
			-DLLVM_BUILD_TESTS=OFF \
			-DLLVM_INCLUDE_TESTS=OFF \
			-DLLVM_BUILD_BENCHMARKS=OFF \
			-DLLVM_INCLUDE_BENCHMARKS=OFF \
			-DLLVM_BUILD_LLVM_DYLIB=ON \
			..

$(LLVM_SRC_DIR): $(LLVM_TAR_FILE)
	cd $(EXTERNAL_SRC_DIR) \
		&& tar Jxf $(notdir $<)

$(LLVM_TAR_FILE): | $(EXTERNAL_SRC_DIR)
	cd $(EXTERNAL_SRC_DIR) \
		&& curl -L -O $(LLVM_SRC_URL)

$(EXTERNAL_SRC_DIR):
	mkdir $@
