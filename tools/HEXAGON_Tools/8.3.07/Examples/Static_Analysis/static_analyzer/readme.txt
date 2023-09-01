The LLVM Clang static analyzer is a source code analysis tool that finds
potential bugs in C, C++ and Objective-C programs.
It analyzes the program for various kinds of bugs (e.g., security
threats, memory corruption, garbage values etc.). The static analyzer
outputs a diagnostic report in an html file showing the path
(i.e., control and data) to reach the potential bug.

Using the static analyzer is as simple as:
hexagon-clang++ --analyze -Xclang -analyzer-output -Xclang html -o <out-dir>

The flag '-Xclang' is used to pass options to the hexagon-clang front end (cc1).
Here, we are passing '-analyzer-output html' to cc1 to specify that
analysis reports should be generated in html format.
The html output is created in the directory <out-dir>.

If the directory is not present it is automatically created by the
compiler if bugs are found. Each report will be in the form of report*.html.
As part of our toolset release, we provide a tool that will parse all
the html reports in a directory, and generate a nice summary
(see post-process).

Typical diagnostic report on a sample program:
// @file: test.cpp
int main() {
  int* p = new int();
  return* p;
}
warning: Potential leak of memory pointed to by `p'

- The static analyzer may report some false positives.
  A few factors contribute towards the generation of false
  positives. For e.g., If we enable the checker used to analyze
  dead code then the static analyzer will flag code conditionally
  enabled for debugging purposes. For that reason we have enabled a
  selected set of checkers which have been tested to identify a very
  high percentage of actual bugs. If needed, additional checkers can
  be enabled by passing the relevant flag to the front end
  (see hexagon-clang -cc1 -analyzer-checker-help for a list of all the
  available checkers).

- Another factor that results in false positives is
  non-returning functions like assert functions. Although the
  static analyzer 'knows' about the standard library non-returning
  functions, if the program has its own implementation of asserts
  for example, it helps to mark them with __attribute__((__noreturn__)).
  Having this attribute greatly improves diagnostics and lessens the
  number of false positives. For example:

void my_abort(const char* msg) __attribute__((__noreturn__)) {
  printf("%s", msg);
  exit(1);
}

Compile and perform static analysis at the same time:

hexagon-clang++ --compile-and-analyze <dir> test.cpp

This will produce diagnostic (html) files in <dir>. When using a build
system, keeping the same directory name throughout the build will
produce all the html report files in <dir>. The filenames generated
for report uses hashing functions so it is safe to assume that the
report files will not be overwritten.
We recommend using this flag to run the static analyzer. There are
several advantages of this feature:
-	This will produce the analysis report of all the files in a single place.
-	The flag can be passed from the build system, which will help
  perform static analysis and compilation every time when the program is built.
-	Since the build systems are really good at tracking the files which
  have changed and compiling only the minimal set of required files,
  the overall turnaround time of static analysis will be very small
  and user can afford to run static analyzer with every build.

To generate a summary of all the static analysis reports produced by
the compiler, we have provided a script called 'post-process'.

-----------------------------------------------------------------------
Post-process:
-----------------------------------------------------------------------
To view a summarized report of html output files produced by the
--compile-and-analyze option, use the post-process script as follows.

post-process --report-dir <dir> --html-title MyReport

e.g., post-process --report-dir report-dir --html-title FirstReport

This script reads all the files from <dir> and produces a file
'index.html' in the same directory. The --html-title e.g., `MyReport'
in this case, is the title of index.html file. It is provided in the
'$INSTALL_PREFIX/Tools/bin' directory.
See post-process --help for more details.

-----------------------------------------------------------------------
Advanced usage of the static analyzer:
-----------------------------------------------------------------------
The hexagon-clang static analyzer has a number of checkers which analyze programs
for various kinds of potential bugs. By default only selected number of
checkers have been enabled in order to limit the compile time, and keep
a high true positive rate. Currently, the checkers are categorised into
	1. alpha
	2. core
	3. cplusplus (Only for analyzing C++ programs)
	4. debug
	5. osx (Only for analyzing objective-C/objective-C++ programs)
	6. security
	7. unix

Each category (or package) contains a number of checkers, for e.g.,
NullDereference checker is a 'core' checker, and NewDelete is a 'cplusplus' checker.
The nesting of checkers into packages and sub-packages makes it easier to enable/disable
a set of checkers when required.

So, in order to enable all the 'alpha' checkers use:
hexagon-clang++ --analyze -Xclang -analyzer-output -Xclang html -Xclang -analyzer-checker=alpha -o <out-dir>

In order to 'disable' all the 'alpha' checkers use:
hexagon-clang++ --analyze -Xclang -analyzer-output -Xclang html -Xclang -analyzer-disable-checker=alpha -o <out-dir>

To enable multiple checkers, the list of checkers can be separated by comma or written separately:
hexagon-clang++ --analyze -Xclang -analyzer-output -Xclang html -Xclang -analyzer-checker=alpha,core -o <out-dir>
hexagon-clang++ --analyze -Xclang -analyzer-output -Xclang html -Xclang -analyzer-checker=alpha -Xclang -analyzer-checker=core -o <out-dir>

To get a list of all checkers use the following command:
$ hexagon-clang -cc1 -analyzer-checker-help
Sample output:
...
alpha.core.CastSize             Check when casting a malloc'ed type T, whether the size is a multiple of the size of T
alpha.core.CastToStruct         Check for cast from non-struct pointer to struct pointer
alpha.core.FixedAddr            Check for assignment of a fixed address to a pointer
alpha.core.IdenticalExpr        Warn about unintended use of identical expressions in operators
...
core.DivideZero                 Check for division by zero
...

So the checker CastSize belongs to the 'alpha' category and 'alpha.core' sub-category.
Note that 'alpha.core' is different from 'core' because 'alpha.core' is a subcategory of 'alpha'.

-----------------------------------------------------------------------
cc1:
-----------------------------------------------------------------------
When we invoke the compiler e.g., hexagon-clang test.cpp, we are actually invoking
a driver program which in turn invokes a set of 'tools' to complete the
compilation process. hexagon-clang provides a flag to see the list of steps involved
in the compilation process. For e.g.,

hexagon-clang test.c -ccc-print-phases
0: input, "test.c", c
1: preprocessor, {0}, cpp-output
2: compiler, {1}, assembler
3: assembler, {2}, object
4: linker, {3}, image

On the other hand if we only want to generate object files:
hexagon-clang test.c -c -ccc-print-phases
0: input, "test.c", c
1: preprocessor, {0}, cpp-output
2: compiler, {1}, assembler
3: assembler, {2}, object

So, the driver program (hexagon-clang/hexagon-clang++) builds a 'pipeline' of 'jobs' to be
executed in order to complete the compilation process. For e.g.,

$ hexagon-clang test.c -###

// Preprocessor+Compiler:
"hexagon-clang" "-cc1" "-triple" "hexagon-unknown--elf" "-S"
.....
"-main-file-name" "test.c" "-o" "/tmp/test-f11bc1.s" "-x" "c" "test.c"

// Assembler:
"/prj/dsp/qdsp6/release/internal/branch-7.2/linux64/latest/Tools/bin/hexagon-llvm-mc" "-march=hexagon" "-filetype=obj"
"-mcpu=hexagonv60" "-o" "/tmp/test-761baf.o" "/tmp/test-f11bc1.s"

// Linker:
"/prj/dsp/qdsp6/release/internal/branch-7.2/linux64/latest/Tools/bin/hexagon-link" "-march=hexagon" "-mcpu=hexagonv60" "-o" "a.out"
.....
"/tmp/test.o" "--start-group" "-lstandalone" "-lc" "-lgcc" "--end-group"
.....

When we invoke the static analyzer by passing '--analyze' flag to the compiler,
the driver only invokes the 'compiler' i.e., -cc1 to perform the static analysis.
e.g.,
$ hexagon-clang --analyze test.c -ccc-print-phases
0: input, "test.c", c
1: preprocessor, {0}, cpp-output
2: analyzer, {1}, plist

and,
$ hexagon-clang --analyze test.c -###

"hexagon-clang" "-cc1" "-triple" "hexagon-unknown--elf" "-analyze" "-disable-free" "-main-file-name" "test.c" "-analyzer-store=region" "-analyzer-opt-analyze-nested-blocks" "-analyzer-eagerly-assume" "-analyzer-checker=core" "-analyzer-checker=unix" "-analyzer-checker=deadcode" "-analyzer-checker=unix" "-analyzer-checker=deadcode" "-analyzer-checker=security.insecureAPI.UncheckedReturn" "-analyzer-checker=security.insecureAPI.getpw" "-analyzer-checker=security.insecureAPI.gets" "-analyzer-checker=security.insecureAPI.mktemp" "-analyzer-checker=security.insecureAPI.mkstemp" "-analyzer-checker=security.insecureAPI.vfork" "-analyzer-checker=alpha.cplusplus.NewDeleteLeaks" "-analyzer-checker=alpha.core.FixedAddr" "-analyzer-checker=alpha.unix.SimpleStream" "-analyzer-checker=core.StackAddressEscape" "-analyzer-checker=security" "-analyzer-checker=unix.Malloc" "-analyzer-checker=core.builtin.NoReturnFunctions" "-analyzer-output" "plist"
.....
"-o" "test.plist" "-x" "c" "test.c"

This command is helpful because we can easily see the list of checkers
enabled by default.

In order to pass any flag to the internal tools i.e., the compiler, assembler or
the linker, hexagon-clang provides different options at the driver level. For e.g.,
To pass a flag directly to the 'compiler' i.e., cc1 we use the flag '-Xclang'
To pass a flag directly to the 'static analyzer', we use the flag '-Xanalyzer'
To pass a flag directly to the 'assembler', we use the flag '-Xassembler'
To pass a flag directly to the 'linker', we use the flag '-Xlinker'

To pass a flag separated with spaces we need to use '-Xflag' multiple times
as shown in the following example.

// To enable a checker in the static analyzer:
$ hexagon-clang++ --analyze -Xclang -analyzer-output -Xclang html -Xclang -analyzer-checker=alpha -o <out-dir>

Here, '-analyzer-output html' and '-analyzer-checker=alpha' are passed to the cc1,
which is then passed to the static analyzer.

Example:

The makefile will perform static analysis on test.cpp and create an html report in report-dir folder.