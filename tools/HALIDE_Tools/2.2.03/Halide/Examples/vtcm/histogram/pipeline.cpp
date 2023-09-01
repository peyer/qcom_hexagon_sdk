#include "Halide.h"
#include "process.h"
using namespace Halide;

// Histogram example using scatter-accumulates.
// Requirements: hvx_v65 target feature.
class Histogram : public Halide::Generator<Histogram> {
public:
    Input<Buffer<DTYPE>> input{"input", 2};
    Output<Buffer<HTYPE>> output{"output", 1};

    void generate() {
        RDom r(input);
        Expr bin = clamp(cast<int32_t>(input(r.x, r.y)), 0, HSIZE-1);
        hist(x) = cast<HTYPE>(0);
        hist(bin) += cast<HTYPE>(1);
        output(x) = hist(x);

        if (get_target().has_feature(Target::HVX_v65)) {
            const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;
            hist
                .compute_at(output, Var::outermost())
                .store_in(MemoryType::VTCM)
                .vectorize(x, vector_size/2);
            hist
                .update(0)
                .allow_race_conditions()
                .vectorize(r.x, vector_size/2);
        }
    }

    void schedule() {
        input.dim(0).set_min(0);
        input.dim(1).set_min(0);
        output.dim(0).set_min(0);

        input.dim(0).set_extent(W);
        input.dim(1).set_extent(H);
        output.dim(0).set_extent(HSIZE);

        if (get_target().features_any_of({Target::HVX_64, Target::HVX_128})) {
            const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;
            output
                .hexagon()
                .vectorize(x, vector_size/2);
        }
    }
private:
    Func hist{"hist"};
    Var x{"x"}, y{"y"}, yi{"yi"};
};

HALIDE_REGISTER_GENERATOR(Histogram, histogram);
