#include "Halide.h"
#include "process.h"
using namespace Halide;

#define UPDATE(X, Y)    X.update(Y) \
                        .allow_race_conditions() \
                        .vectorize(r.x, vector_size)

#define PURE(X)         X.compute_at(output, Var::outermost()) \
                        .store_in(MemoryType::VTCM) \
                        .vectorize(x, vector_size)

// Chain of scatter-gather-vector_store-vector_load instructions for hvx
// target >= v65.
// Assumption: no collisions while scattering values.
// Requirements: hvx_v65 target feature.
class SGChain : public Halide::Generator<SGChain> {
public:
    Input<Buffer<DTYPE>> input{"input", 1};
    Input<Buffer<DTYPE>> lut1{"lut1", 1};
    Input<Buffer<DTYPE>> lut2{"lut2", 1};
    Output<Buffer<DTYPE>> output{"output", 1};

    void generate() {
        RDom r(0, W); // RDom r(input); ---> This generates pretty bad code.
        Expr lut1_pure = clamp(cast<int32_t>(lut1(x)), 0, W-1);
        Expr lut1_update = clamp(cast<int32_t>(lut1(r.x)), 0, W-1);
        Expr lut2_pure = clamp(cast<int32_t>(lut2(x)), 0, W-1);
        Expr lut2_update = clamp(cast<int32_t>(lut2(r.x)), 0, W-1);

        // Scatter-gather && Gather-Load
        input_vtcm(x) = input(x);
        buf0(x) = cast<DTYPE>(11);
        buf0(lut1_update) = input_vtcm(r.x); // ---> Scatter
        buf0(r.x) = input_vtcm(lut1_update); // ---> Gather
        buf_out0(x) = buf0(x); // ---> Load
        // Gather-gather && Gather-Load
        buf1(x) = input_vtcm(lut1_pure); // ---> Gather
        buf1(r.x) = input_vtcm(lut2_update); // ---> Gather
        buf_out1(x) = buf1(x); // ---> Load
        // Gather-scatter && Scatter-Load
        buf2(x) = input_vtcm(lut1_pure); // ---> Gather
        buf2(lut2_update) = input_vtcm(r.x) * 2; // ---> Scatter
        buf_out2(x) = buf2(x); // ---> Load
        // Scatter-scatter
        buf_out3(x) = cast<DTYPE>(61);
        buf_out3(lut2_update) = input_vtcm(r.x); // ---> Scatter
        buf_out3(lut1_update) = cast<DTYPE>(41); // ---> Scatter
        // Scatter-Store
        buf_out4(x) = cast<DTYPE>(11);
        buf_out4(lut2_update) = input_vtcm(r.x) * 3; // ---> Scatter
        buf_out4(r.x) = input_vtcm(r.x); // ---> Store
        // Gather-Store
        buf_out5(x) = input_vtcm(lut1_pure); // ---> Gather
        buf_out5(r.x) = cast<DTYPE>(97); // ---> Store

        output(x) = buf_out0(x) + buf_out1(x) + buf_out2(x) +
                    buf_out3(x) + buf_out4(x) + buf_out5(x);

        if (get_target().has_feature(Target::HVX_v65)) {
            const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;
            UPDATE(buf0, 0);
            UPDATE(buf0, 1);
            UPDATE(buf1, 0);
            UPDATE(buf2, 0);
            UPDATE(buf_out3, 0);
            UPDATE(buf_out3, 1);
            UPDATE(buf_out4, 0);
            UPDATE(buf_out4, 1);
            UPDATE(buf_out5, 0);
        }
    }

    void schedule() {
        input.dim(0).set_min(0);
        input.dim(0).set_extent(W);
        output.dim(0).set_min(0);
        output.dim(0).set_extent(W);

        if (get_target().features_any_of({Target::HVX_64, Target::HVX_128})) {
            const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;
            PURE(input_vtcm);
            PURE(buf0);
            PURE(buf1);
            PURE(buf2);
            PURE(buf_out0);
            PURE(buf_out1);
            PURE(buf_out2);
            PURE(buf_out3);
            PURE(buf_out4);
            PURE(buf_out5);
            output
                .hexagon()
                .vectorize(x, vector_size);
        }
    }
private:
    Func input_vtcm{"input_vtcm"}, buf0{"buf0"}, buf1{"buf1"}, buf2{"buf2"},
         buf3{"buf3"}, buf_out0{"buf_out0"}, buf_out1{"buf_out1"},
         buf_out2{"buf_out2"}, buf_out3{"buf_out3"}, buf_out4{"buf_out4"},
         buf_out5{"buf_out5"};
    Var x{"x"};
};

HALIDE_REGISTER_GENERATOR(SGChain, sg_chain);
