#include "Halide.h"

using namespace Halide;

class DmaPipeline : public Generator<DmaPipeline> {
public:
    Input<Buffer<uint8_t>> input{"input", 3};
    Output<Buffer<uint8_t>> output{"output", 3};

    void generate() {
        Var x{"x"}, y{"y"}, c{"c"};

        // We need a wrapper for the output so we can schedule the
        // multiply update in tiles.
        Func copy("copy"), work("work");
         /////////////////////////////////
        // Algorithm
        ////////////////////////////////

        // DMA Read
        copy(x, y, c) = input(x, y, c);
        // Basic Processing
        work(x, y, c) = copy(x, y, c) * 2;
        //DMA Write
        output(x, y, c) = work(x, y, c);

        /////////////////////////////////////////////////////
        // Schedule
        // Note: Order of scheduling directives are important
        ////////////////////////////////////////////////////

        output.copy_to_device();   // DMA write to the Output Buffer(schedule hint)

        output
            .compute_root()       // Pure Function, Allocated Full Sized and computed at root
            .bound(c, 0, 4)
            .reorder(c, x, y);

         // tweak stride/extent to handle deinterleaving
        input.dim(0).set_stride(4);
        input.dim(2).set_stride(1).set_bounds(0, 4);
        output.dim(0).set_stride(4);
        output.dim(2).set_stride(1).set_bounds(0, 4);


        Var tx("tx"), ty("ty"), yo("yo"), new_x("new_x");
        Expr fac = output.dim(1).extent()/4;

        // Break the output into tiles.
        const int tile_width = 256;
        const int tile_height = 128;
        // Tiling Output with boundary condition as roundup
        output
            .split(y, yo, y, fac)
            .tile(x, y, tx, ty, x, y, tile_width, tile_height, TailStrategy::RoundUp);

        copy
            .compute_at(output, tx)      // Do the DMA Read at each tile in the horizontal order
            .copy_to_host()              // Schedule directie to hint that this is a DMA Read
            .bound(c, 0, 4)
            .store_in(MemoryType::LockedCache)  // The destination of DMA read is a locked L2 cache
            .reorder_storage(c, x, y)
            .reorder(c, x, y);
        
        work
            .compute_at(output, tx)
            .bound(c, 0, 4)
            .store_in(MemoryType::LockedCache)  // The  destination of output is locked L2 cache
            .reorder_storage(c, x, y)
            .reorder(c, x, y)
            .fuse(c, x, new_x)
            .vectorize(new_x, 128, TailStrategy::RoundUp);
    }

};

HALIDE_REGISTER_GENERATOR(DmaPipeline, dma_raw_rw_halide)
 
