#include <algorithm>
#include <cstring>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>

#include "OutputSectionIteratorPlugin.h"
#include "SectionIteratorPlugin.h"

using namespace QCLD;

// All plugins must be derived from one of the linker defined plugins.
// In this case it is the SectionIteratorPlugin defined in
// "SectionIteratorPlugin.h"
class DLL_A_EXPORT ChangeSectionOutputPlugin : public SectionIteratorPlugin {

public:
  // This constructor takes no arguments;
  // the base class will take the name we will call this plugin
  ChangeSectionOutputPlugin() : SectionIteratorPlugin("CHANGESECTIONS") {}

  // This init doesn't do anything, but we could add code here
  void Init(std::string Options) override {}

  // This function will be called whenever the linker processes a section.
  // We will add this section to our vector of sections for later use.
  void processSection(QCLD::Section S) override {
    // this example only processes the text_sw output section
    QCLD::OutputSection OutputSection = Linker->getOutputSection(S);

    // add these sections to the vector
    if (OutputSection.getName().find("text_sw") != std::string::npos) {
      m_Sections.push_back(S);
    }
  }

  // After the linker lays out the image, but before it creates the elf file,
  // it will call this run function.
  Status Run(bool Trace) override {
    std::cout << "\nThe following output sections are being changed.\n\n";

    // iterate through all the sections added to the vector
    for (auto &S : m_Sections)
      getOrChangeOutputSections(S, Trace);

    std::cout << "\nFinished processing cold input sections.\n\n";

    return Plugin::Status::SUCCESS;
  }

  // Called from run
  void getOrChangeOutputSections(QCLD::Section S, bool Trace) {
    // only modify input sections that contain .cold
    if (S.getName().find(".cold") != std::string::npos) {
      // get the output section name for this input section
      QCLD::OutputSection OutputSection = Linker->getOutputSection(S);

      // append .cold to the output section name and move it.
      // The new output section name must already be defined
      // in the linker script.
      Linker->setOutputSection(S, OutputSection.getName() + ".cold");

      // If the linker is invoked with --trace=plugin,
      // this string will be printed to stdout
      if (Trace) {
        std::cout << "Changing output section for " << S.getName()
                  << " in file " << S.getInputFile().getFileName()
                  << " from: " << OutputSection.getName()
                  << " to: " << OutputSection.getName() + ".cold"
                  << "\n";
      }
    }
  }

  void Destroy() override {}

  uint32_t GetLastError() override { return 0; }

  std::string GetLastErrorAsString() override { return "SUCCESS"; }

  std::string GetName() override { return "CHANGESECTIONS"; }

private:
  std::vector<QCLD::Section> m_Sections;
};

// All plugins must be derived from one of the linker defined plugins.
class DLL_A_EXPORT ChangeSectionOrderPlugin
    : public OutputSectionIteratorPlugin {

public:
  // This constructor takes one argument: Chunks() returns pieces of a section,
  // including the section name; the base class will take the name we will call
  // this plugin
  ChangeSectionOrderPlugin()
      : OutputSectionIteratorPlugin("ORDERBLOCKS"),
        ReorderOutputSection(nullptr) {}

  // Perform any initialization here
  void Init(std::string Options) override {}

  // This function will be called whenever the linker processes a OutputSection.
  void processOutputSection(OutputSection O) override {
    if (Linker->getState() != LinkerWrapper::CreatingSections)
      return;
    if (O.getName() != ".compress")
      return;
    ReorderOutputSection = O;
  }

  // After the linker lays out the image, but before it creates the elf file,
  // it will call this run function.
  Status Run(bool Trace) override {
    if (Linker->getState() != LinkerWrapper::CreatingSections)
      return Plugin::Status::SUCCESS;

    if (!ReorderOutputSection.getOutputSection())
      return Plugin::Status::SUCCESS;

    LinkerScriptRule DefaultRule =
        ReorderOutputSection.getLinkerScriptRules().back();

    std::cout << "*************************************************************"
                 "***************"
              << "\n";
    std::cout << "This plugin only operates on the section named: "
                 ".compress\n";
    std::cout << "*************************************************************"
                 "***************"
              << "\n";
    std::cout << "This is the orignal order of this section:\n";
    std::cout << "*************************************************************"
                 "***************"
              << "\n";

    std::vector<Chunk> AllChunks;
    for (auto &LSR : ReorderOutputSection.getLinkerScriptRules()) {
      std::vector<Chunk> CS = LSR.getChunks();
      for (auto &C : CS) {
        std::cout << C.getName() << "\n";
        AllChunks.push_back(C);
        LSR.removeChunk(C);
      }
    }

    // This loop will iterate through all of the sections we have added to our
    // vector. Each pair of sections is reordered alphabetically by section
    // name. We will do all of the sorting in this loop.
    std::sort(AllChunks.begin(), AllChunks.end(),
              [](const Chunk &A, const Chunk &B) {
                if (A.getName().find(".cold") != std::string::npos)
                  return 0;
                else
                  return 1;
              });

    DefaultRule.updateChunks(AllChunks);

    std::cout << "*************************************************************"
                 "***************"
              << "\n";
    std::cout << "This is the changed order of this section:\n";
    std::cout << "*************************************************************"
                 "***************"
              << "\n";
    for (auto &LSR : ReorderOutputSection.getLinkerScriptRules()) {
      for (auto &C : LSR.getChunks())
        std::cout << C.getName() << "\n";
    }
    std::cout << "*************************************************************"
                 "***************"
              << "\n";

    // We could track a private variable for return status, but in this example
    // we always return success
    return Plugin::Status::SUCCESS;
  }

  void Destroy() override {}

  uint32_t GetLastError() override { return 0; }

  std::string GetLastErrorAsString() override { return "SUCCESS"; }

  std::string GetName() override { return "ORDERBLOCKS"; }

private:
  OutputSection ReorderOutputSection;
};
std::unordered_map<std::string, Plugin *> Plugins;

extern "C" {
bool DLL_A_EXPORT RegisterAll() {
  static bool AlreadyInitialized = false;
  if (AlreadyInitialized)
    return true;
  Plugins["CHANGESECTIONS"] = new ChangeSectionOutputPlugin();
  Plugins["ORDERCHUNKS"] = new ChangeSectionOrderPlugin();
  AlreadyInitialized = true;
  return true;
}

Plugin DLL_A_EXPORT *getPlugin(const char *T) {
  return Plugins[std::string(T)];
}

void DLL_A_EXPORT Cleanup() {
  delete Plugins["CHANGESECTIONS"];
  delete Plugins["ORDERCHUNKS"];
  Plugins.clear();
}
}
