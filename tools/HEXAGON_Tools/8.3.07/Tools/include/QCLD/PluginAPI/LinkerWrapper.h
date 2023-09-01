//===- LinkerWrapper.h ----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LINKER_WRAPPER_H
#define LINKER_WRAPPER_H

#include "PluginADT.h"
#include <unordered_map>
#include <vector>
#include <system_error>

namespace mcld {
class Section;
class Plugin;
}

namespace QCLD {

// This particular piece is duplicated in all the header files. This is
// make sure the customer includes one header file. Heaer files that are
// included in specific CPP files will also export the definition.
#ifdef _WIN32
// MSVC produces a warning from exporting STL types. This is to prevent
// customers from mixing STL implementations. We dont have that use case
// as we ask the customers to use the same runtime as per what is built with the
// tools. Disable this warning.
#pragma warning(disable : 4251)
#ifndef DLL_A_EXPORT
#ifndef IN_DLL
#define DLL_A_EXPORT __declspec(dllexport)
#else
#define DLL_A_EXPORT __declspec(dllimport)
#endif // IN_DLL
#endif // DLL_A_EXPORT
#endif // WIN32

#ifndef _WIN32
#define DLL_A_EXPORT
#endif // WIN32

class DLL_A_EXPORT LinkerWrapper;

// This is a Thin Linker Wrapper. The plugin calls the functions that are part
// of the Linker wrapper only.
class DLL_A_EXPORT LinkerWrapper {
public:
  enum State {
    Unknown,
    Initializing,
    BeforeLayout,
    CreatingSections,
    AfterLayout
  };

  /// Construct LinkerWrapper object.
  explicit LinkerWrapper(mcld::Plugin *, void *);

  /// Any memory that needs to live for the lifetime of the linker should use
  /// function. This function allocates memory and returns an error if memory
  /// cannot be allocated.
  std::error_code allocateMemory(std::string PluginName, uint32_t Sz,
                                 uint32_t &AllocatedSize, uint8_t **Buf);

  //  Returns a vector of uses that are referred from the Chunk.
  std::vector<Use> getUses(Chunk &C);

  /// This function can be called by the SectionIterator, and
  /// returns a vector of uses that are referred from the Chunk.
  std::vector<Use> getUses(Section &S);

  /// This returns a linker symbol for inspection for a symbol that the user
  /// wants to lookup.
  Symbol getSymbol(std::string Sym) const;

  /// This allows the plugin to override a output section for Section.
  void setOutputSection(Section &S, std::string OutputSection);

  /// The plugin can query the linker script to see what output section does a
  /// Section match.
  OutputSection getOutputSection(Section &S) const;

  /// The plugin can query the linker script if it defines an OutputSection.
  OutputSection getOutputSection(std::string OutputSection);

  /// Finish assigning output sections.
  bool finishAssignOutputSections();

  /// The plugin can set the linker to fatally error, if it recognizes that a
  /// dependency or the plugin doesnot capture all the requirements of the
  /// functionality.
  void setLinkerFatalError();

  // Get the linker plugin associated with the wrapper.
  mcld::Plugin *getPlugin() const { return m_Plugin; }

  /// Match pattern against a string.
  bool matchPattern(const std::string &Pattern, const std::string &Name) const;

  // Destroy the Linker Wrapper.
  ~LinkerWrapper();

  // Get the state of the
  State getState() const;

  // Add a symbol to the Chunk.
  void addSymbolToChunk(Chunk &C, std::string Symbol, uint64_t Val);

  // Create a Padding Chunk.
  Chunk createPaddingChunk(uint32_t Alignment, size_t PaddingSize);

  // Replace contents of a chunk.
  enum DLL_A_EXPORT ReplaceStatus : uint8_t {
    InvalidHandle,
    InvalidSymbol,
    NoChunkForSymbol,
    ChunkIsBSS,
    SymbolIsSmall,
    Ok,
  };

  ReplaceStatus replaceSymbolContent(QCLD::Symbol S, const uint8_t *Buf,
                                     size_t Sz);

private:
  void *m_Handle;
  mcld::Plugin *m_Plugin;
};
}

#endif
