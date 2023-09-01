#include "Halide.h"

using namespace Halide;

class Conv3x3a16 : public Generator<Conv3x3a16> {
public:
    // Takes an 8 bit image; one channel.
    Input<Buffer<uint8_t>> input{"input", 2};
    Input<Buffer<int8_t>> mask{"mask", 2};
    // Outputs an 8 bit image; one channel.
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        bounded_input(x, y) = BoundaryConditions::repeat_edge(input)(x, y);
        Expr sum = cast<int16_t>(0);
        for (int j = -1; j <= 1; j++) {
            for (int i = -1; i <= 1; i++) {
                sum += cast<int16_t>(bounded_input(x+j, y+i)) * cast<int16_t>(mask(j+1, i+1));
            }
        }
        output(x, y) = cast<uint8_t>(clamp(sum >> 4, 0, 255));
    }

    void schedule() {
        Var xi{"xi"}, yi{"yi"};

        input.dim(0).set_min(0);
        input.dim(1).set_min(0);

        auto output_buffer = Func(output).output_buffer();
        output_buffer.dim(0).set_min(0);
        output_buffer.dim(1).set_min(0);

        if (1) {
            const int vector_size = 128;
            Expr input_stride = input.dim(1).stride();
            input.dim(1).set_stride((input_stride/vector_size) * vector_size);
            input.set_host_alignment(vector_size);

            Expr output_stride = output_buffer.dim(1).stride();
            output_buffer.dim(1).set_stride((output_stride/vector_size) * vector_size);
            output_buffer.set_host_alignment(vector_size);

            bounded_input
                .store_root()
                .compute_at(Func(output), y)
                .align_storage(x, 128)
                .vectorize(x, vector_size, TailStrategy::RoundUp);
            Func(output)
                .tile(x, y, xi, yi, vector_size, 4, TailStrategy::RoundUp)
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
    Func bounded_input{"input_bounded"};
};

HALIDE_REGISTER_GENERATOR(Conv3x3a16, conv3x3a16_halide);
