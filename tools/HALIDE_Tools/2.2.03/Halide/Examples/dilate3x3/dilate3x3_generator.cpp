#include "Halide.h"

using namespace Halide;

class Dilate3x3 : public Generator<Dilate3x3> {
public:
    // Takes an 8 bit image; one channel.
    Input<Buffer<uint8_t>> input{"input", 2};
    // Outputs an 8 bit image; one channel.
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
#ifdef BORDERS
        bounded_input(x, y) = BoundaryConditions::repeat_edge(input)(x, y);
        max_y(x, y) = max(bounded_input(x, y-1), bounded_input(x, y), bounded_input(x, y+1));
#else
        max_y(x, y) = max(input(x, y-1), input(x, y), input(x, y+1));
#endif
        output(x, y) = max(max_y(x-1, y), max_y(x, y), max_y(x+1, y));
    }

    void schedule() {
        Var xi{"xi"}, yi{"yi"};

        input.dim(0).set_min(0);
        input.dim(1).set_min(0);

        auto output_buffer = Func(output).output_buffer();
        output_buffer.dim(0).set_min(0);
        output_buffer.dim(1).set_min(0);

        if (get_target().features_any_of({Target::HVX_64, Target::HVX_128})) {
            const int vector_size = get_target().has_feature(Target::HVX_128) ? 128 : 64;
            Expr input_stride = input.dim(1).stride();
            input.dim(1).set_stride((input_stride/vector_size) * vector_size);
            input.set_host_alignment(vector_size);

            Expr output_stride = output_buffer.dim(1).stride();
            output_buffer.dim(1).set_stride((output_stride/vector_size) * vector_size);
            output_buffer.set_host_alignment(vector_size);
#ifdef BORDERS
            bounded_input.compute_root();
#endif
            Func(output)
              .tile(x, y, xi, yi, vector_size*2, 4, TailStrategy::RoundUp)
              .vectorize(xi)
              .unroll(yi);
        } else {
            const int vector_size = natural_vector_size<uint8_t>();
            Func(output)
                .vectorize(x, vector_size)
                .parallel(y, 16);
        }
    }
private:
    Var x{"x"}, y{"y"};
    Func max_y{"max_y"};
#ifdef BORDERS
    Func bounded_input{"bounded_input"};
#endif
};

HALIDE_REGISTER_GENERATOR(Dilate3x3, dilate3x3);
