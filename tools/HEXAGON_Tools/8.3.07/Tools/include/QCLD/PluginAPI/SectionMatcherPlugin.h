//===- SectionMatcherPlugin.h --------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef SECTION_MATCHER_PLUGIN_H
#define SECTION_MATCHER_PLUGIN_H

#include "LinkerPlugin.h"

namespace QCLD {

class SectionMatcherPlugin : public Plugin {
public:
  /* Constructor */
  SectionMatcherPlugin(std::string Name)
      : Plugin(Plugin::Type::SectionMatcher, Name) {}

  static bool classof(const Plugin *P) {
    return P->getType() == QCLD::Plugin::Type::SectionMatcher;
  }

  static bool classof(const SectionMatcherPlugin *) { return true; }

  /* Chunks that the linker will call the client with */
  virtual void processSection(Section S) = 0;
};
}
#endif
