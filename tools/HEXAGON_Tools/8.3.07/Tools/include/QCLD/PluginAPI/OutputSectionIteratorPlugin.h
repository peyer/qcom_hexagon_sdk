//===- OutputSectionIteratorPlugin.h --------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef OUTPUT_SECTION_ITERATOR_PLUGIN_H
#define OUTPUT_SECTION_ITERATOR_PLUGIN_H

#include "LinkerPlugin.h"

namespace QCLD {

class OutputSectionIteratorPlugin : public Plugin {
public:
  /* Constructor */
  OutputSectionIteratorPlugin(std::string Name)
      : Plugin(Plugin::Type::OutputSectionIterator, Name) {}

  static bool classof(const Plugin *P) {
    return P->getType() == QCLD::Plugin::Type::OutputSectionIterator;
  }

  static bool classof(const OutputSectionIteratorPlugin *) { return true; }

  virtual void processOutputSection(OutputSection O) = 0;
};
} // namespace QCLD
#endif
