#include "Halide.h"
#include "process.h"
using namespace Halide;

// The pipeline demonstrates the tricks to use vgathers with small LUT.
// Requirements: hvx_v65 target feature.
class Small_LUT : public Halide::Generator<Small_LUT> {
    public:
        Input<Buffer<DTYPE>> input{"input", 2};
        Input<Buffer<DTYPE>> lut{"lut", 1};
        Output<Buffer<DTYPE>> output{"output", 2};

        void generate() {
            Expr idx = clamp(cast<int32_t>(input(x, y)), 0, lut.width()-1);

            if (get_target().has_feature(Target::HVX_v65)) {
                const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;
                const int elems = vector_size/sizeof(DTYPE);
                // Splat the LUT.
                lut_vtcm(x) = lut(x/elems);
                // Spread accesses across multiple banks.
                output_vtcm(x, y) = lut_vtcm(elems*idx + x%elems);
            } else {
                lut_vtcm(x) = lut(x);
                output_vtcm(x, y) = lut_vtcm(idx);
            }
            output(x, y) = output_vtcm(x, y);
        }

        void schedule() {
            input.dim(0).set_min(0);
            input.dim(1).set_min(0);
            output.dim(0).set_min(0);
            output.dim(1).set_min(0);

            input.dim(0).set_extent(W);
            input.dim(1).set_extent(H);
            output.dim(0).set_extent(W);
            output.dim(1).set_extent(H);
            
            lut.dim(0).set_min(0);
            lut.dim(0).set_extent(LUT_SIZE);

            if (get_target().features_any_of({Target::HVX_64, Target::HVX_128})) {
                const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;

                output
                    .hexagon()
                    .split(y, y, yi, H/2)
                    .parallel(y)
                    .prefetch(input, yi, 2)
                    .vectorize(x, 2*vector_size);

                lut_vtcm
                    .compute_at(output, Var::outermost())
                    .vectorize(x, vector_size/2);

                output_vtcm
                    .compute_at(output, x)
                    .store_at(output, y)
                    .vectorize(x, 2*vector_size);

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

HALIDE_REGISTER_GENERATOR(Small_LUT, small_lut);
