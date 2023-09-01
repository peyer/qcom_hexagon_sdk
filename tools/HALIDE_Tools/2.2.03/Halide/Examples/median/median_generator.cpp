#include "Halide.h"

using namespace Halide;

class Median : public Generator<Median> {
public:
    // Takes an 8 bit image; one channel.
    Input<Buffer<uint8_t>> input{"input", 2};
    // Outputs an 8 bit image; one channel.
    Output<Buffer<uint8_t>> output{"output", 2};

    void generate() {
#ifdef BORDERS
        bounded_input(x, y) = BoundaryConditions::repeat_edge(input)(x, y);
        // FIXME: PDB: Would ideally like to use variadic versions of min/max, but using
        // variadic versions of min/max causes a performance regression. It may be because
        // lazy slice vectors isn't doing what it needs to for variadic versions of min/max.
        max_y(x,y) = max3(bounded_input(x ,y-1), bounded_input(x, y), bounded_input(x, y+1));
        min_y(x,y) = min3(bounded_input(x, y-1), bounded_input(x, y), bounded_input(x, y+1));
        mid_y(x,y) = mid3(bounded_input(x, y-1), bounded_input(x, y), bounded_input(x, y+1));
#else
        max_y(x,y) = max3(input(x ,y-1), input(x, y), input(x, y+1));
        min_y(x,y) = min3(input(x, y-1), input(x, y), input(x, y+1));
        mid_y(x,y) = mid3(input(x, y-1), input(x, y), input(x, y+1));
#endif

        minmax_x(x,y) = min3(max_y(x-1, y), max_y(x, y), max_y(x+1, y));
        maxmin_x(x,y) = max3(min_y(x-1, y), min_y(x, y), min_y(x+1, y));
        midmid_x(x,y) = mid3(mid_y(x-1, y), mid_y(x, y), mid_y(x+1, y));

        output(x,y) = mid3(minmax_x(x, y), maxmin_x(x, y), midmid_x(x, y));
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
            // This maximizes the benefit of the loop carried reuse pass in LLVM
            Func(output).vectorize(x, vector_size, TailStrategy::RoundUp);

        } else {
            const int vector_size = natural_vector_size<uint8_t>();
            Func(output)
                .vectorize(x, vector_size)
                .parallel(y, 16);
        }
    }
private:
  Var x{"x"}, y{"y"};
  Func max_y{"max_y"}, min_y{"min_y"}, mid_y{"mid_y"};
  Func minmax_x{"minmax_x"}, maxmin_x{"maxmin_x"}, midmid_x{"midmid_x"};
#ifdef BORDERS
  Func bounded_input{"bounded_input"};
#endif
  static Expr mid3(Expr a, Expr b, Expr c) {
    return max(min(max(a, b), c), min(a, b));
  }
  static Expr max3(Expr a, Expr b, Expr c) {
    return max(max(a, b), c);
  }
  static Expr min3(Expr a, Expr b, Expr c) {
    return min(min(a, b), c);
  }

};

HALIDE_REGISTER_GENERATOR(Median, median);
