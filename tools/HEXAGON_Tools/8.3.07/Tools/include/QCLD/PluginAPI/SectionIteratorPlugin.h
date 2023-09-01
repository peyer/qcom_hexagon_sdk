//===- SectionIteratorPlugin.h --------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef SECTION_ITERATOR_PLUGIN_H
#define SECTION_ITERATOR_PLUGIN_H

#include "LinkerPlugin.h"

namespace QCLD {

class SectionIteratorPlugin : public Plugin {
public:
  /* Constructor */
  SectionIteratorPlugin(std::string Name)
      : Plugin(Plugin::Type::SectionIterator, Name) {}

  static bool classof(const Plugin *P) {
    return P->getType() == QCLD::Plugin::Type::SectionIterator;
  }

  static bool classof(const SectionIteratorPlugin *) { return true; }

  /* Chunks that the linker will call the client with */
  virtual void processSection(Section S) = 0;
};
}
#endif
