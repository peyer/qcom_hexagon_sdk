//===- ControlMemorySizePlugin.h
//--------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef CONTROL_MEMORY_SIZE_PLUGIN_H
#define CONTROL_MEMORY_SIZE_PLUGIN_H

#include "LinkerPlugin.h"

namespace QCLD {

class ControlMemorySizePlugin : public Plugin {
public:
  /* Constructor */
  ControlMemorySizePlugin(std::string Name)
      : Plugin(Plugin::ControlMemorySize, Name) {}

  static bool classof(const Plugin *P) {
    return P->getType() == QCLD::Plugin::Type::ControlMemorySize;
  }

  static bool classof(const ControlMemorySizePlugin *) { return true; }

  /* Memory blocks that the linker will call the client with */
  virtual void AddBlocks(Block memBlock) = 0;

  /* Return memory blocks to the client */
  virtual std::vector<Block> GetBlocks() = 0;
};
}
#endif
