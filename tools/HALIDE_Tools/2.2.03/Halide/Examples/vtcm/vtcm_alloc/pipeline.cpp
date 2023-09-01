#include "Halide.h"
#include "process.h"
using namespace Halide;

// Pipeline to test single_page_flag option while allocating VTCM memory.
// Use HL_DEBUG_CODEGEN=1 to check single_page_flag for each buffer in VTCM.
// Requirements: hvx_v65 target feature.
class VTCM_Allocation : public Halide::Generator<VTCM_Allocation> {
    public:
        Input<Buffer<DTYPE>> input{"input", 2};
        Input<Buffer<DTYPE>> lut{"lut", 1};
        Output<Buffer<DTYPE>> output{"output", 2};

        void generate() {
            const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;
            const int elems = vector_size/sizeof(DTYPE);
            // Simple operations 
            part0_vtcm(x) = lut(x) + 1;
            part1_vtcm(x) = part0_vtcm(x) - 1;
            part2_vtcm(x, y) = input(x, y) % 32;
            part3_vtcm(x, y) = input(x, y) / 16;
            // Gather operation
            Expr idx = clamp(cast<int32_t>(input(x, y)), 0, lut.width()-1);
            part4_vtcm(x, y) = part1_vtcm(idx);
            
            output(x, y) = part0_vtcm(x) + part1_vtcm(x) + part2_vtcm(x, y) +
                           part3_vtcm(x, y) + part4_vtcm(x, y);
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
            
            input.dim(1).set_stride(W);
            output.dim(1).set_stride(W);

            lut.dim(0).set_min(0);
            lut.dim(0).set_extent(LUT_SIZE);

            if (get_target().features_any_of({Target::HVX_64, Target::HVX_128})) {
                const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;
                output
                    .hexagon()
                    .split(y, y, yi, 8)
                    .prefetch(input, y, 2)
                    .vectorize(x, vector_size);
                part0_vtcm
                    .compute_at(output, y)
                    .store_at(output, Var::outermost())
                    .vectorize(x, vector_size);
                part1_vtcm
                    .compute_at(output, y)
                    .store_at(output, Var::outermost())
                    .vectorize(x, vector_size);
                part2_vtcm
                    .compute_at(output, y)
                    .store_at(output, Var::outermost())
                    .vectorize(x, vector_size);
                part3_vtcm
                    .compute_at(output, y)
                    .store_at(output, Var::outermost())
                    .vectorize(x, vector_size);
                part4_vtcm
                    .compute_at(output, y)
                    .store_at(output, Var::outermost())
                    .vectorize(x, vector_size);
                if (get_target().has_feature(Target::HVX_v65)) {
                    part0_vtcm.store_in(MemoryType::VTCM);
                    part1_vtcm.store_in(MemoryType::VTCM);
                    part2_vtcm.store_in(MemoryType::VTCM);
                    part3_vtcm.store_in(MemoryType::VTCM);
                    part4_vtcm.store_in(MemoryType::VTCM);
                }
            }
        }
    private:
        Func part0_vtcm{"part0_vtcm"}, part1_vtcm{"part1_vtcm"}, part2_vtcm{"part2_vtcm"},
             part3_vtcm{"part3_vtcm"}, part4_vtcm{"part4_vtcm"};
        Var x{"x"}, y{"y"}, yi{"yi"}, xi{"xi"};
};

HALIDE_REGISTER_GENERATOR(VTCM_Allocation, vtcm_alloc);

