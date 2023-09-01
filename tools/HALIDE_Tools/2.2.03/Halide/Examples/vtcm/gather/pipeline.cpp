#include "Halide.h"
#include "process.h"
using namespace Halide;

// Table lookups using gather instructions.
// Requirements: hvx_v65 target feature.
class Gather : public Halide::Generator<Gather> {
public:
    Input<Buffer<DTYPE>> input{"input", 2};
    Input<Buffer<DTYPE>> lut{"lut", 2};
    Output<Buffer<DTYPE>> output{"output", 2};

    void generate() {
        Expr xCoord = clamp(cast<int32_t>(input(x, 0)), 0, lut.width()-1);
        Expr yCoord = clamp(cast<int32_t>(input(x, 1)), 0, lut.height()-1);
        lut_vtcm(x, y) = lut(x, y);
        output_vtcm(x, y) = lut_vtcm(xCoord, yCoord);
        output(x, y) = output_vtcm(x, y);
    }

    void schedule() {
        input.dim(0).set_min(0);
        input.dim(1).set_min(0);
        lut.dim(0).set_min(0);
        lut.dim(1).set_min(0);
        output.dim(0).set_min(0);
        output.dim(1).set_min(0);

        input.dim(0).set_extent(W);
        input.dim(1).set_extent(H);
        lut.dim(0).set_extent(W);
        lut.dim(1).set_extent(H);
        output.dim(1).set_extent(H);
        output.dim(0).set_extent(W);

        if (get_target().features_any_of({Target::HVX_64, Target::HVX_128})) {
            const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;
            lut_vtcm
                .compute_at(output, Var::outermost())
                .vectorize(x, vector_size/2);

            output_vtcm
                .compute_at(output, y)
                .vectorize(x, vector_size/2);

            output
                .hexagon()
                .split(y, y, yi, H/2)
                .parallel(y)
                .vectorize(x, vector_size/2);

            if (get_target().has_feature(Target::HVX_v65)) {
                lut_vtcm.store_in(MemoryType::VTCM);
                output_vtcm.store_in(MemoryType::VTCM);
            }
        }
    }
private:
    Func lut_vtcm{"lut_vtcm"}, output_vtcm{"output_vtcm"};
    Var x{"x"}, y{"y"}, yi{"yi"};
};

HALIDE_REGISTER_GENERATOR(Gather, gather);
