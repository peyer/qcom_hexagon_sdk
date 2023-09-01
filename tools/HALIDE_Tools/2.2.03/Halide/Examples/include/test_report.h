#ifndef TEST_REPORT_H
#define TEST_REPORT_H
#include <string>
#include <vector>

using namespace std;
enum Result {
    Fail,
    Pass,
    Unknown,
};

const char *get_result_string(Result r) {
    switch(r) {
    case Result::Pass:
        return "Pass";
    case Result::Fail:
        return "Fail";
    case Result::Unknown:
        return "Unknown";
    default:
        return "Bad Result Value";
    }
}

enum Mode {
    Device_Offload,
    Simulator_Offload,
    Device_Standalone,
    Simulator_Standalone,
    Unknown_Mode
};

const char *get_mode_string(Mode m) {
    switch(m) {
    case Mode::Device_Offload:
        return "device-offload";
    case Mode::Simulator_Offload:
        return "simulator-offload";
    case Mode::Device_Standalone:
        return "device-standalone";
    case Mode::Simulator_Standalone:
        return "simulator-standalone";
    default:
        return "Unknown_Mode";
    }

}


class TestReport {
 private:
    string name;
    double perf;
    string units;
    Mode mode;
    Result result;

 public:
 TestReport(string n, double p, string u, Mode m, Result r = Result::Unknown) :
    name(n), perf(p), units(u), mode(m), result(r) {}
        
    void print() {
        fprintf(stdout, "Test_Info: {\n");
        fprintf(stdout, "\tName:%s\n", name.c_str());
        fprintf(stdout, "\tResult:%s\n", get_result_string(result));
        fprintf(stdout, "\tPerf:%f\n", perf);
        fprintf(stdout, "\tUnits:%s\n", units.c_str());
        fprintf(stdout, "\tMode:%s\n", get_mode_string(mode));
        fprintf(stdout, "}\n");
    }
    void add_mode(const Mode &m) {
        mode = m;
    }
};
#endif
