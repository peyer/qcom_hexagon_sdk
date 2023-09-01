//===- ControlFileSizePlugin.h --------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef CONTROL_FILE_SIZE_PLUGIN_H
#define CONTROL_FILE_SIZE_PLUGIN_H

#include "LinkerPlugin.h"

namespace QCLD {

class ControlFileSizePlugin : public Plugin {
public:
  /* Constructor */
  ControlFileSizePlugin(std::string Name)
      : Plugin(Plugin::ControlFileSize, Name) {}

  static bool classof(const Plugin *P) {
    return P->getType() == QCLD::Plugin::Type::ControlFileSize;
  }

  static bool classof(const ControlFileSizePlugin *) { return true; }

  /* Memory blocks that the linker will call the client with */
  virtual void AddBlocks(Block memBlock) = 0;

  /* Return memory blocks to the client */
  virtual std::vector<Block> GetBlocks() = 0;
};
}
#endif
