//===- LinkerPlugin.h -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LINKER_PLUGIN_H
#define LINKER_PLUGIN_H

#include "LinkerWrapper.h"
#include "PluginADT.h"
#include <string>
#include <vector>

namespace QCLD {

// PluginType.
typedef const char *PluginType;

// Register Function.
typedef bool RegisterAllFuncT();

class LinkerWrapper;

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
#endif

#define MAJ_API_VERSION "2"
#define MIN_API_VERSION "0"

#ifdef _WIN32
#pragma comment(user, "Plugin built with tools 8.3.07")
#else
__asm__(".section .comment\n\t"
        ".string \"Plugin built with tools 8.3.07\"");
#endif

const char PLUGIN_VERSION[] = MAJ_API_VERSION "." MIN_API_VERSION;

class DLL_A_EXPORT Plugin {
public:
  // Default error codes to signify SUCCESS or FAILURE. 0 indicates success and
  // any other integer signifies failure.
  enum Status : uint8_t { SUCCESS = 0, ERROR = 1 };

  enum Type : uint8_t {
    ControlFileSize,
    ControlMemorySize,
    SectionIterator,
    SectionMatcher,
    OutputSectionIterator
  };

  /* Constructor */
  explicit Plugin(Type T, std::string Name)
      : m_Type(T), EC(SUCCESS), Linker(nullptr), PluginName(Name) {}

  /* Get the type of the plugin */
  Plugin::Type getType() const { return m_Type; }

  /* Initialize the plugin with options specified */
  virtual void Init(std::string Options) = 0;

  /* The actual algorithm that will be implemented */
  virtual Status Run(bool Verbose) = 0;

  /* Linker will call Destroy, and the client would be able to free up any data
   * structures that its not relevant */
  virtual void Destroy() = 0;

  /* Returns the last error, a value of 0 means there was no error */
  virtual uint32_t GetLastError() = 0;

  /* Returns the error as string */
  virtual std::string GetLastErrorAsString() = 0;

  /* Returns the name of the plugin */
  virtual std::string GetName() = 0;

  std::string getVersion() const { return QCLD::PLUGIN_VERSION; }

  /* Pointer to Linker Wrapper */
  void setLinkerWrapper(QCLD::LinkerWrapper *L) { Linker = L; }

  /* Destructor */
  virtual ~Plugin() {}

protected:
  /* Plugin Type */
  Type m_Type;

  /* Error code */
  uint32_t EC;

  /* Linker Wrapper */
  QCLD::LinkerWrapper *Linker;

  /* Plugin Name */
  std::string PluginName;
};

// Handler
typedef Plugin *PluginFuncT(PluginType P);

// Cleanup Handler
typedef void *PluginCleanupFuncT();
}
#endif
