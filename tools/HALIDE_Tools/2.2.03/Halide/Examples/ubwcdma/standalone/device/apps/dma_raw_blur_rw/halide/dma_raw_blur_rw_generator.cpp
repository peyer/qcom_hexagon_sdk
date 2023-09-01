#include "Halide.h"

using namespace Halide;
#define LINE_BUFFERING 1

// Define a 1D Gaussian blur (a [1 4 6 4 1] filter) of 5 elements.
Expr blur5(Expr x0, Expr x1, Expr x2, Expr x3, Expr x4) {
    // Widen to 16 bits, so we don't overflow while computing the stencil.
    x0 = cast<uint16_t>(x0);
    x1 = cast<uint16_t>(x1);
    x2 = cast<uint16_t>(x2);
    x3 = cast<uint16_t>(x3);
    x4 = cast<uint16_t>(x4);
    return cast<uint8_t>((x0 + 4*x1 + 6*x2 + 4*x3 + x4 + 8)/16);
}

class DmaPipeline : public Generator<DmaPipeline> {
public:
    Input<Buffer<uint8_t>> input{"input", 2};
    Output<Buffer<uint8_t>> output{"output", 2};
    GeneratorParam<bool> use_line_buffering{"use_line_buffering", false};

    void generate() {
        Var x{"x"}, y{"y"};
        // We need a wrapper for the output so we can schedule the
        // multiply update in tiles.
        Func copy_bounded("copy_bounded");
        Func blur_y("blur_y");
        Func input_copy("input_copy"), output_copy("output_copy");
        Func work("work");
        /////////////////////////////////
        // Algorithm
        ////////////////////////////////

        // DMA Read
        input_copy(x, y) = input(x, y);
        // Bounding the X dimension accesses to be with in frame boundary 
        Expr bounded_x = max(input.dim(0).min(), min(x, input.dim(0).extent()-1));
        // Bounding the Y dimension accesses to be with in frame boundary 
        Expr bounded_y = max(input.dim(1).min(), min(y, input.dim(1).extent()-1));
        copy_bounded(x,y) =  input_copy(bounded_x, bounded_y);
        // BLUR Filter Algorithm ( Y direction Blur)
        blur_y(x, y) = blur5(copy_bounded(x, y - 2),
                             copy_bounded(x, y - 1),
                             copy_bounded(x, y),
                             copy_bounded(x, y + 1),
                             copy_bounded(x, y + 2));

        // BLUR Filter Algorithm (X direction Blur)
        work(x, y) = blur5(blur_y(x - 2, y),
                             blur_y(x - 1, y),
                             blur_y(x, y),
                             blur_y(x + 1, y),
                             blur_y(x + 2, y));
        // DMA Write
        output(x, y) = work(x, y);

        /////////////////////////////////////////////////////
        // Schedule
        // Note: Order of scheduling directives are important
        ////////////////////////////////////////////////////

        output.copy_to_device();                 // DMA write to the Output Buffer(schedule hint)
        output
            .compute_root();                     // Pure Function, Allocated Full Sized and computed at root
        if (use_line_buffering) {
            // Line Buffering
            Var yo("yo"), yi("yi"), yii("yii");
           // Break the output into Line strips
           Expr strip_size = output.dim(1).extent() / 2;
           strip_size = (strip_size / 2) * 2;

            output
                .split(y, yo, y, strip_size)        // Split Outputs in to Strips of Two Lines
                .split(y, yi, yii, 64)
                .parallel(yo);                       // Execute Parallely on Two Lines

            input_copy
                .compute_at(output, yi)              // Do the DMA Read for each Line
                .store_in(MemoryType::LockedCache)   // The  destination of DMA read is a locked L2 cache
                .copy_to_host();                     // Schedule directie to hint that this is a DMA Read

            work.compute_at(output, yi)
                .store_in(MemoryType::LockedCache); // After Processing put the Output in a Locked cache

            blur_y
               .compute_at(output, yi)
               .store_in(MemoryType::LockedCache)
               .vectorize(x, 128, TailStrategy::RoundUp);

        } else {

            // Tiling Output with boundary condition as roundup
            Var tx("tx"), ty("ty");
            Var yo("yo"), yi("yi");
            // Break the output into tiles.
            const int tile_width = 128;
            const int tile_height = 64;
            Expr strip_size = output.dim(1).extent() / 2;
            strip_size = (strip_size / 2) * 2;
            output
                .split(y, yo, y, strip_size)
                .tile(x, y, tx, ty, x, y, tile_width, tile_height, TailStrategy::RoundUp)
                .parallel(yo);

             input_copy
                .compute_at(output, tx)     // Do the DMA Read at each tile in the horizontal order
                .store_in(MemoryType::LockedCache)  // The  destination of DMA read is a locked L2 cache
                .store_at(output, ty)
                .copy_to_host();             // Schedule directie to hint that this is a DMA Read

            // Schedule the work in tiles (same for all DMA schedules).
            work
               .compute_at(output, tx)                         //This directive ensure ping pong nature of the dma
               .store_in(MemoryType::LockedCache)  // After Processing, put the output in a locked Cache
               .store_at(output, ty);

             blur_y
               .compute_at(output, tx)
               .store_in(MemoryType::LockedCache)
               .store_at(output, ty)
               .vectorize(x, 128, TailStrategy::RoundUp);
        }

    }

};

HALIDE_REGISTER_GENERATOR(DmaPipeline, dma_raw_blur_rw_halide)
 
