#include "Halide.h"
#include "process.h"
using namespace Halide;

// Scatter values using scatter instruction.
// Assumption: no collisions while scattering values.
// Requirements: hvx_v65 target feature.
class Scatter : public Halide::Generator<Scatter> {
public:
    Input<Buffer<DTYPE>> x_idx{"x_idx", 1};
    Input<Buffer<DTYPE>> y_idx{"y_idx", 1};
    Output<Buffer<DTYPE>> output{"output", 2};

    void generate() {
        RDom r(0, W, 0, H);
        Expr xCoord = clamp(cast<int32_t>(x_idx(r.x)), 0, W-1);
        Expr yCoord = clamp(cast<int32_t>(y_idx(r.y)), 0, H-1);
        // Scatter values all over f
        output_vtcm(x, y) = cast<DTYPE>(19);
        output_vtcm(xCoord, yCoord) = x_idx(r.x) + cast<DTYPE>(r.x) +
                                      y_idx(r.y) + cast<DTYPE>(r.y);
        output(x, y) = output_vtcm(x, y);

        if (get_target().has_feature(Target::HVX_v65)) {
            const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;
            output_vtcm
                .update(0)
                .allow_race_conditions()
                .vectorize(r.x, vector_size/2);
        }
    }

    void schedule() {
        x_idx.dim(0).set_min(0);
        y_idx.dim(0).set_min(0);
        output.dim(0).set_min(0);
        output.dim(1).set_min(0);

        x_idx.dim(0).set_extent(W);
        y_idx.dim(0).set_extent(H);
        output.dim(0).set_extent(W);
        output.dim(1).set_extent(H);

        if (get_target().features_any_of({Target::HVX_64, Target::HVX_128})) {
            const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;

            output_vtcm
                .compute_at(output, Var::outermost())
                .vectorize(x, vector_size/2);

            output
                .hexagon()
                .split(y, y, yi, H/2)
                .parallel(y)
                .vectorize(x, vector_size/2);

            if (get_target().has_feature(Target::HVX_v65)) {
                output_vtcm.store_in(MemoryType::VTCM);
            }
        }
    }
private:
    Func output_vtcm{"output_vtcm"};
    Var x{"x"}, y{"y"}, yi{"yi"};
};

HALIDE_REGISTER_GENERATOR(Scatter, scatter);
