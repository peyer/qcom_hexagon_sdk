//===- PluginConfig.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef PLUGIN_CONFIG_H
#define PLUGIN_CONFIG_H

#include <llvm/ADT/StringRef.h>
#include <llvm/ObjectYAML/YAML.h>
#include <mcld/PluginAPI/LinkerPlugin.h>

namespace mcld {
namespace LinkerPlugin {

struct GlobalPlugin {
  QCLD::Plugin::Type PluginType;
  std::string PluginName;
  std::string LibraryName;
  std::string Options;
};

struct OutputSectionPlugin {
  QCLD::Plugin::Type PluginType;
  std::string OutputSection;
  std::string PluginName;
  std::string LibraryName;
  std::string Options;
};

struct Config {
  std::vector<mcld::LinkerPlugin::GlobalPlugin> GlobalPlugins;
  std::vector<mcld::LinkerPlugin::OutputSectionPlugin> OutputSectionPlugins;
};
} // namespace Plugin
} // namespace mcld

LLVM_YAML_IS_SEQUENCE_VECTOR(mcld::LinkerPlugin::Config)
LLVM_YAML_IS_SEQUENCE_VECTOR(mcld::LinkerPlugin::GlobalPlugin)
LLVM_YAML_IS_SEQUENCE_VECTOR(mcld::LinkerPlugin::OutputSectionPlugin)

namespace llvm {
namespace yaml {
template <> struct MappingTraits<mcld::LinkerPlugin::Config> {
  static void mapping(IO &IO, mcld::LinkerPlugin::Config &Config);
};
template <> struct MappingTraits<mcld::LinkerPlugin::GlobalPlugin> {
  static void mapping(IO &IO, mcld::LinkerPlugin::GlobalPlugin &GlobalPlugin);
};
template <> struct MappingTraits<mcld::LinkerPlugin::OutputSectionPlugin> {
  static void
  mapping(IO &IO, mcld::LinkerPlugin::OutputSectionPlugin &OutputSectionPlugin);
};
} // namespace yaml
} // namespace llvm

#endif
