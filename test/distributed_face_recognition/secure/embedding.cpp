#include "embedding.h"
#include "datareader.h"
// #include "mobilefacenet.bin.inc"
#include "mobilefacenet.id.h"
#include "mobilefacenet.mem.h"
// #include "mobilefacenet.param.inc"
#include "net.h"
#include "simpleocv.h"
#include <TEE-Capability/common.h>
// int __fprintf_chk(FILE *, int, const char *, ...);
// int __sprintf_chk(char *, int, size_t, const char *, ...);
#include <cstddef>
#include <cstdint>
#include <cstring> // 用于memset
#include <unordered_map>
#include <vector>

class BitmapMemoryPool : public ncnn::Allocator {
private:
  void *pool; // 内存池的起始地址
  size_t blockSize;
  size_t poolSize;
  size_t numBlocks;
  std::vector<uint8_t> bitmap; // 位图，用于追踪每个块的占用情况
  std::unordered_map<size_t, size_t> allocatedMap;

public:
  BitmapMemoryPool(void *memPool, size_t poolSize, size_t blockSize)
      : pool(memPool), poolSize(poolSize), blockSize(blockSize) {
    numBlocks = poolSize / blockSize;
    bitmap.resize((numBlocks + 7) / 8,
                  0); // 初始化位图，所有位都设置为0（空闲）
  }

  virtual ~BitmapMemoryPool() override {}

  virtual void *fastMalloc(size_t size) override {
    size_t numBlocksNeeded =
        (size + blockSize - 1) / blockSize; // 计算所需的块数
    size_t startBlock = findFreeBlocks(numBlocksNeeded); // 查找连续空闲块

    if (startBlock == size_t(-1)) {
      eapp_print("NO BLOCKS\n");
      *(char *)0 = 0;
      return nullptr; // 找不到足够的连续空闲块
    }

    // 标记这些块为已分配
    for (size_t i = 0; i < numBlocksNeeded; ++i) {
      setBlockUsed(startBlock + i);
    }

    allocatedMap.emplace(startBlock, numBlocksNeeded);
    // eapp_print("ALLOC: %d blocks at %d\n", numBlocksNeeded, startBlock);
    return static_cast<char *>(pool) + startBlock * blockSize;
  }

  virtual void fastFree(void *ptr) override {
    if (!ptr)
      return; // 空指针，无需操作

    size_t blockIndex =
        (static_cast<char *>(ptr) - static_cast<char *>(pool)) / blockSize;
    size_t numBlocksNeeded = allocatedMap[blockIndex];
    for (size_t i = 0; i < numBlocksNeeded; ++i)
      setBlockFree(i + blockIndex);
  }

private:
  // 查找连续的空闲块
  size_t findFreeBlocks(size_t numBlocksNeeded) {
    for (size_t i = 0; i <= numBlocks - numBlocksNeeded; ++i) {
      bool found = true;
      for (size_t j = 0; j < numBlocksNeeded; ++j) {
        if (isBlockUsed(i + j)) {
          found = false;
          i += j; // 优化搜索，跳过检查过的已占用的块
          break;
        }
      }
      if (found) {
        return i;
      }
    }
    return size_t(-1); // 没有找到
  }

  bool isBlockUsed(size_t blockIndex) {
    return bitmap[blockIndex / 8] & (1 << (blockIndex % 8));
  }

  void setBlockUsed(size_t blockIndex) {
    bitmap[blockIndex / 8] |= (1 << (blockIndex % 8));
  }

  void setBlockFree(size_t blockIndex) {
    bitmap[blockIndex / 8] &= ~(1 << (blockIndex % 8));
  }
};

bool check_nan(float f) {
  unsigned int *bits = (unsigned int *)&f;

  return (((*bits) & 0x7F800000) == 0x7F800000) &&
         (((*bits) & 0x007FFFFF) != 0);
}
void print_num(int num) { eapp_print("%d", num); }

void print_float(float num) {
  // check num is nan
  if (check_nan(num)) {
    eapp_print("nan\n");
    return;
  }
  print_num((int)(num * 10000));
}
float mat_sum(const ncnn::Mat &mat) {
  float sum = 0;
  for (int i = 0; i < mat.total(); i++) {
    if (mat[i] > 100) {
      sum += mat[i] / 100;
    } else
      sum += mat[i];
  }
  return sum;
}
int embedding(in_char img[IMG_SIZE], out_char res[EMBEDDING_SIZE]) {
  ncnn::Net net;

#ifdef __TEE
  const int POOL_SIZE = 1024 * 1024 * 50;
  auto pool = new BitmapMemoryPool(malloc(POOL_SIZE), POOL_SIZE, 1024 * 16);
  eapp_print("POOL CREATED\n");
  net.opt.use_vulkan_compute = false;
  net.opt.blob_allocator = pool;
  net.opt.workspace_allocator = pool;
#endif
  /* const unsigned char *mobilefacenet_param_ptr = mobilefacenet_param; */
  /* const unsigned char *mobilefacenet_bin_ptr = mobilefacenet_bin; */
  net.load_param(mobilefacenet_param_bin);
  eapp_print("LOADED PARAM\n");
  net.load_model(mobilefacenet_bin);
  eapp_print("LOADED MODEL\n");
  /* if
   * (net.load_param_bin(ncnn::DataReaderFromMemory(mobilefacenet_param_ptr)))
   */
  /*   exit(-1); */
  /* if (net.load_model(ncnn::DataReaderFromMemory(mobilefacenet_bin_ptr))) */
  /*   exit(-1); */

  ncnn::Mat input = ncnn::Mat::from_pixels((const unsigned char *)img,
                                           ncnn::Mat::PIXEL_RGB, WIDTH, HEIGHT);
  for (int q = 0; q < input.c; q++) {
    float *ptr = input.channel(q);
    for (int i = 0; i < input.w * input.h; i++) {
      ptr[i] = ptr[i] * (1.f);
    }
  }

  ncnn::Mat output;
retry: {
  ncnn::Extractor extractor = net.create_extractor();
  extractor.input(mobilefacenet_param_id::BLOB_data, input);

  eapp_print("BEGIN INVOKE\n");
  // extractor.extract(mobilefacenet_param_id::BLOB_fc1, output);
  // for (int i = 1; i <= mobilefacenet_param_id::BLOB_fc1; i++) {
  //   extractor.extract(i, output);
  //   eapp_print("-----------%d------------", i);

  //   print_float(mat_sum(output));
  // }
  extractor.extract(mobilefacenet_param_id::BLOB_fc1, output);
}

  ncnn::Mat out_flatterned = output.reshape(output.w * output.h * output.c);

  float *out = (float *)res;
  if (check_nan(out_flatterned[0]))
    goto retry;
  TEE_ASSERT(out_flatterned.w * out_flatterned.h * out_flatterned.c == EMB_LEN,
             "EMB_LEN: %d, OUT_FLATTERNED: %d\n", EMB_LEN,
             out_flatterned.w * out_flatterned.h * out_flatterned.c);
  for (int i = 0; i < EMB_LEN; i++) {
    out[i] = out_flatterned[i];
    if (i < 10) {
      eapp_print("THE EMB[%d] is %d", i, (int)(out[i] * 1000));
    }
  }
  eapp_print("DONE\n");

  /* return EMB_LEN * sizeof(float); */
  return seal_data_inplace((char *)res, EMBEDDING_SIZE,
                           EMB_LEN * sizeof(float));
}

float calculate_distance(in_char emb1[EMBEDDING_SIZE],
                         in_char emb2[EMBEDDING_SIZE]) {

  int emb_len1 = unseal_data_inplace(emb1, EMBEDDING_SIZE);
  int emb_len2 = unseal_data_inplace(emb2, EMBEDDING_SIZE);

  TEE_ASSERT(emb_len1 == emb_len2, "EMB1 LEN: %d, EMB2 LEN: %d\n", emb_len1,
             emb_len2);

  // emb_len1 /= sizeof(float);

  float *f1 = (float *)emb1;
  float *f2 = (float *)emb2;

  float sum = 0.f;
  for (int i = 0; i < EMB_LEN; i++) {
    float d = f1[i] - f2[i];
    sum += d * d;
  }

  // memcpy(dist, &sum, sizeof(float));
  return sum;
  // return 0;
}
