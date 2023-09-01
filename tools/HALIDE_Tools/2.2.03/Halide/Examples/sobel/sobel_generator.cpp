#include "Halide.h"

using namespace Halide;

class Sobel : public Generator<Sobel> {
public:
    Input<Buffer<uint8_t>> input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
        Func input_16{"input_16"};
#ifdef BORDERS
        bounded_input(x, y) = BoundaryConditions::repeat_edge(input)(x, y);
        input_16(x, y) = cast<int16_t>(bounded_input(x, y));
#else
        input_16(x, y) = cast<int16_t>(input(x, y));
#endif
        sobel_x_avg(x,y) = input_16(x-1, y) + 2*input_16(x, y) + input_16(x+1, y);
        sobel_x(x, y) = absd(sobel_x_avg(x, y-1), sobel_x_avg(x, y+1));

        sobel_y_avg(x, y) = input_16(x-1, y) - input_16(x+1, y);
        sobel_y(x, y) = abs(sobel_y_avg(x, y-1) + 2*sobel_y_avg(x, y) + sobel_y_avg(x, y+1));

        // This sobel implementation is non-standard in that it doesn't take the square root
        // of the gradient.
        output(x, y) = cast<uint8_t>(clamp(sobel_x(x, y) + sobel_y(x, y), 0, 255));
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
              .tile(x, y, xi, yi, vector_size, 6, TailStrategy::RoundUp)
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
    Func sobel_x_avg{"sobel_x_avg"}, sobel_y_avg{"sobel_y_avg"};
    Func sobel_x{"sobel_x"}, sobel_y{"sobel_y"};
#ifdef BORDERS
    Func bounded_input{"bounded_input"};
#endif
};

HALIDE_REGISTER_GENERATOR(Sobel, sobel);
