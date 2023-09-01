#*********************************************************************************#
#
#	Examples/Profiling/pmustats_spreadsheet Example Readme File
#
#*********************************************************************************#
The Hexagon simulator collects PMU statistics while an application is running.
When the application terminates, the simulator writes the collected statistics
to a PMU statistics file.

PMU statistics files are text files which contain the same execution information
that the Hexagon Performance Monitor Unit generates to support on-target
performance tracking:

  Instruction scheduling details
  Bus access events
  Cache access events

The statistics file name is determined by the --pmu_statsfile hexagon-sim option.
The default name is pmu_statsfile.txt.  Appendix A of the Hexagon Simulator Guide
defines the symbols that are used in PMU statistics files to represent Hexagon
processor execution events.

The pmustats.xlsm file is a Windows Office 2013(TM) spreadsheet which is a Hexagon
V60 Performance Calculator.  The spreadsheet is capable of loading the
pmu_statsfile output from the hexagon-sim using the LoadPmuStatsFile macro button.

The pmu_statsfile data will be displayed on the left three columns and Metric
parameters with statistics will be on the right 8 columns.  Pie graphs show
Core Cycle View, Thread Concurrency, Average Cluster View of CPP, Average
ThreadView CPP, L2Cache Miss Composition, Instruction Mix and HVX Cycles.

Further description on the PMU events can be found by clicking on the
"(link to PMU descriptions)" link on column B1.