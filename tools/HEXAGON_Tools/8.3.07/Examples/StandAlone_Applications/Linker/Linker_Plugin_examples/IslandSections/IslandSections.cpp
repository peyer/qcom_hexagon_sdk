#include <algorithm>
#include <cstring>
#include <iostream>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>

#include "SectionIteratorPlugin.h"

using namespace QCLD;

// All plugins must be derived from one of the linker defined plugins.
// In this case it is the SectionIteratorPlugin defined in
// "SectionIteratorPlugin.h"
class DLL_A_EXPORT IslandUsesPlugin : public SectionIteratorPlugin {

public:
  // This constructor takes no arguments;
  // the base class will take the name we will call this plugin
  IslandUsesPlugin() : SectionIteratorPlugin("FINDUSES") {}

  // This init doesn't do anything, but we could add code here
  void Init(std::string Options) override {}

  // This function will be called whenever the linker processes a section.
  // We will add this section to our vector of sections for later use.
  void processSection(QCLD::Section S) override { m_Sections.push_back(S); }

  // After the linker lays out the image, but before it creates the elf file,
  // it will call this run function.
  Status Run(bool Trace) override {
    std::cout << "\n";

    // This loop will iterate through all of the sections we have added to our
    // vector. We will do all of the checking in this loop
    for (auto &S : m_Sections)
      printSectionUses(S);

    std::cout << "\n";

    // We could track a private variable for return status, but in this example
    // we always return success
    return Plugin::Status::SUCCESS;
  }

  // This is where all of the work is done.
  // The run function will call this for each section in our vector
  void printSectionUses(QCLD::Section &S) {
    // Uses are relocations from a section
    // We will use a queue to hold all of the uses of this section
    std::queue<QCLD::Use> Uses;

    // Add all of the uses of this section to this queue
    for (auto &U : Linker->getUses(S))
      Uses.push(U);

    // define a set of uses
    std::set<Chunk, Chunk::Compare> SectionUses;

    // define a set of fragments
    std::set<mcld::Fragment *> FragmentSet;

    // Iterate through the queue of uses for this section
    while (!Uses.empty()) {
      // get the next use
      Use &U = Uses.front();
      Uses.pop();

      // Get the piece of input section that this use refers to
      Chunk ChunkForUse = U.getChunk();

      // get the pointer to the section
      mcld::Fragment *F = ChunkForUse.getFragment();

      // Skip Uses that do not have corresponding sections.
      // Examples are : Weak undefined symbols.
      if (!ChunkForUse.getFragment())
        continue;

      // add all of the valid uses to the SectionUses set
      SectionUses.insert(ChunkForUse);
    }

    // get the input section name for this section
    std::string thisInputSection = S.getName();

    QCLD::OutputSection OS = Linker->getOutputSection(S);

    // get the output section name for this section
    std::string thisOutputSection = OS.getName();

    // is the output section an island section?
    if (thisOutputSection.find("island") != std::string::npos) {
      // only process sections with a non zero size
      if (S.getSize()) {

        // if there are no uses for this section skip it
        if (SectionUses.size() > 0) {

          // track the last section that had an error
          std::string LastErrorSection;

          // check all the uses of this input section
          for (auto &C : SectionUses) {
            Section UsedSection = C.getSection();
            std::string UsesSectionName = C.getName();

            QCLD::OutputSection UsesOutputSection =
                Linker->getOutputSection(UsedSection);

            std::string UsesOutputName = UsesOutputSection.getName();

            if (UsesOutputName.find("island") == std::string::npos) {
              // only report once per section
              if (thisInputSection != LastErrorSection) {
                std::cout << "\nError: The input section " << thisInputSection
                          << " in output section " << thisOutputSection
                          << " is accessing " << UsesSectionName
                          << " in output section " << UsesOutputName;
                LastErrorSection = thisInputSection;

                std::cout << ".  The section use is " << UsesSectionName
                          << " from file " << UsedSection.getInputFile().getFileName();
                std::cout << "\n";
              }
            }
          }
        }
      }
    }
  }

  void Destroy() override {}

  uint32_t GetLastError() override { return 0; }

  std::string GetLastErrorAsString() override { return "SUCCESS"; }

  std::string GetName() override { return "FINDUSES"; }

private:
  std::vector<QCLD::Section> m_Sections;
};

Plugin *ThisPlugin = nullptr;

extern "C" {
bool DLL_A_EXPORT RegisterAll() {
  ThisPlugin = new IslandUsesPlugin();
  return true;
}
Plugin DLL_A_EXPORT *getPlugin(const char *T) { return ThisPlugin; }
void DLL_A_EXPORT Cleanup() {
  if (ThisPlugin)
    delete ThisPlugin;
}
}
