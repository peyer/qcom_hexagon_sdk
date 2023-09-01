#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>

#include "ControlMemorySizePlugin.h"

// This example uses zlib on Linux
// If zlib is not installed set it to zero
#if _WIN64
#define HAVE_ZLIB 0
#else
#define HAVE_ZLIB 1
#include <zlib.h>
#endif

using namespace QCLD;

// All plugins must be derived from one of the linker defined plugins.
// In this case it is the ControlFileSizePlugin defined in
// "ControlMemorySizePlugin.h"
class DLL_A_EXPORT CopyBlocksPlugin : public ControlMemorySizePlugin {

public:
  // This constructor will take the plungin name
  CopyBlocksPlugin() : ControlMemorySizePlugin("COPYBLOCKS"), Blocks() {}

  // This init doesn't do anything, but we could add code here
  void Init(std::string Options) override { Blocks.clear(); }

  // This function is called whenever the linker processes
  // an ouput section referring to this plugin.
  // The block already has the relocations resolved.
  void AddBlocks(Block B) override {
    // Add the output section block to the block vector.
    std::cout << "\nCollecting Block " << B.Name.c_str() << "with a size of "
              << B.Size << " bytes.\n\n";

    Blocks.push_back(B);
  }

  Status Run(bool Verbose) override {
    // There is actually only one block in the vector in this example
    // It has already been added to the vector when this function is called.

    // create the copy block
    Block newBlock;

    // set the name of the section it will be "copied" to
    newBlock.Name = ".compressed";

    // get the original block length
    int nLenSrc = Blocks.front().Size;

    // declare a memory buffer for the new block
    uint8_t *Buf;

    // this is the actual amount of memory returned
    // by the allocateMemory function
    uint32_t AllocSz;

    // Get memory for the new block, it will be smaller
    // than the source block so just use the source value.
    // This memory will live for the lifetime of the linker.
    Linker->allocateMemory(GetName(), nLenSrc, AllocSz, &Buf);

    // check for allocate success or return error
    if (!Buf)
      return Plugin::Status::ERROR;

    // zero the buffer
    std::memset(Buf, 0, AllocSz);

    // use this buffer for the new block
    newBlock.Data = Buf;

#if HAVE_ZLIB

#define windowBits 15
#define GZIP_ENCODING 16

    int nErr = Z_OK;

    z_stream zInfo = {0};

    // configure zlib parameters
    zInfo.zalloc = Z_NULL;
    zInfo.zfree = Z_NULL;
    zInfo.opaque = Z_NULL;
    zInfo.total_in = zInfo.avail_in = nLenSrc;
    zInfo.total_out = zInfo.avail_out = nLenSrc;
    zInfo.next_in = (unsigned char *)Blocks.front().Data;
    zInfo.next_out = (unsigned char *)newBlock.Data;

    // compress the section
    nErr = deflateInit2(&zInfo, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                        windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY);
    if (nErr == Z_OK) {
      nErr = deflate(&zInfo, Z_FINISH); // zlib function
      if (nErr == Z_STREAM_END) {
        newBlock.Size = zInfo.total_out;
      }
    }

    // if zlib fails exit
    if (nErr < 0)
      return Plugin::Status::ERROR;

    deflateEnd(&zInfo); // zlib function

#else // HAVE_ZLIB

    // the new block will contain this smaller string
    std::string smallstring = "Opening paragraph of the book Moby Dick.";
    std::memcpy(Buf, smallstring.c_str(), smallstring.size());

    // set the new block size
    newBlock.Size = smallstring.size();

#endif

    // clear the original block
    Blocks.clear();

    // add the new block
    Blocks.push_back(newBlock);

    std::cout << "\nCopying Block" << newBlock.Name.c_str() << "with a size of "
              << newBlock.Size << " bytes.\n\n";

    return Plugin::Status::SUCCESS;
  }

  std::vector<Block> GetBlocks() override {
    // The linker will call this function
    // to get the new block(s)
    return Blocks;
  }

  void Destroy() override {}

  uint32_t GetLastError() override { return 0; }

  std::string GetLastErrorAsString() override { return "SUCCESS"; }

  std::string GetName() override { return "COPYBLOCKS"; }

private:
  std::vector<Block> Blocks;
};

Plugin *ThisPlugin = nullptr;

extern "C" {
bool DLL_A_EXPORT RegisterAll() {
  ThisPlugin = new CopyBlocksPlugin();
  return true;
}
Plugin DLL_A_EXPORT *getPlugin(const char *T) { return ThisPlugin; }
void DLL_A_EXPORT Cleanup() {
  if (ThisPlugin)
    delete ThisPlugin;
}
}
