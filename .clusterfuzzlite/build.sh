#!/bin/bash -eu

################################
# 1. 基本环境
################################
cd $SRC/nanomq

export CC=${CC:-clang}
export CXX=${CXX:-clang++}

# OSS-Fuzz 自动注入：
# -fsanitize=fuzzer,address,undefined
# -O1 -g
# 不要手动再加 sanitizer

################################
# 2. 构建 NanoMQ（仅需要库）
################################

rm -rf build
mkdir -p build
cd build

cmake .. \
  -DCMAKE_C_COMPILER=$CC \
  -DCMAKE_CXX_COMPILER=$CXX \
  -DBUILD_STATIC_LIB=ON \
  -DBUILD_CLIENT=OFF \
  -DENABLE_RULE_ENGINE=ON \
  -DENABLE_ACL=ON \
  -DENABLE_JWT=OFF \
  -DBUILD_NFTP=OFF \
  -DBUILD_NANOMQ_CLI=OFF \
  -DNANOMQ_TESTS=OFF

cmake --build . --target nanomq -- -j$(nproc)

################################
# 3. 构建 fuzz targets
################################

cd ..

FUZZ_DIR=fuzz
LIBS=(
  build/nanomq/libnanomq.a
  build/nng/libnng.a
  -lm
  -lpthread
  -ldl
  -lnsl
)

INCLUDES=(
  -Inanomq
  -Inanomq/include
  -Inng/include
  -Inng/src
  -Inng/src/core
  -Inng/src/supplemental
)

src="$FUZZ_DIR/fuzz_pub_handler.c"
target=$(basename "$src" .c)
echo "Building fuzz target: $target"

$CC $CFLAGS \
  $src \
  ${INCLUDES[@]} \
  -DSUPP_RULE_ENGINE -DACL_SUPP \
  -DNNG_PLATFORM_POSIX -DNNG_PLATFORM_LINUX \
  -o $OUT/$target \
  ${LIBS[@]} \
  $LIB_FUZZING_ENGINE \
  $LDFLAGS

################################
# 4. Seed corpus
################################

corpus_dir="$FUZZ_DIR/corpus/$target"
if [ -d "$corpus_dir" ]; then
    mkdir -p "$OUT/${target}_seed_corpus"
    cp "$corpus_dir"/* "$OUT/${target}_seed_corpus/" \
       2>/dev/null || true
fi

echo "NanoMQ fuzz build done"
