#include "Halide.h"

using namespace Halide;

class DmaPipeline : public Generator<DmaPipeline> {
public:
    Input<Buffer<uint8_t>> input_y{"input_y", 2};
    Input<Buffer<uint8_t>> input_uv{"input_uv", 3};
    Output<Buffer<uint8_t>> output_y{"output_y", 2};
    Output<Buffer<uint8_t>> output_uv{"output_uv", 3};

    void generate() {

        Var x{"x"}, y{"y"}, c{"c"};

        // We could use 'in' to generate the input copies, but we can't name the variables that way.
        Func input_y_copy("input_y_copy"), input_uv_copy("input_uv_copy");

        Func work_y("work_y");
        Func work_uv("work_uv");

        ////////////////////////////////////////
        // Algorithm
        ////////////////////////////////////////

        // DMA Read
        input_y_copy(x, y) = input_y(x, y);
        // Basic Processing 
        work_y(x, y) = input_y_copy(x, y) * 2;
        // DMA Write
        output_y(x, y) = work_y(x, y);
        // DMA Read for UV
        input_uv_copy(x, y, c) = input_uv(x, y, c);
        // Basic Processing
        work_uv(x, y, c) = input_uv_copy(x, y, c) * 2;
        // DMA Write for UV 
        output_uv(x, y, c) = work_uv(x, y, c);

        //////////////////////////////////////////////////////
        // Schedule
        // Note: Order of scheduling directives are important
        //////////////////////////////////////////////////////

        Var tx("tx"), ty("ty"), new_x("new_x"), yo("yo");

        // DMA write to the Output Buffer

        output_y.copy_to_device();
        output_uv.copy_to_device();

        output_y
            .compute_root();

        output_uv
            .compute_root()    // Pure Function, Allocated Full Sized and computed at root 
            .bound(c, 0, 2)    // UV Plane has only Two components
            .reorder(c, x, y); // Re-order so that the DMA transfer happen in natural Order

        // Adjusting the Buffer Stride and extent to support Interleaved UV data for 4:2:0 format 
        // For both input and output
        input_uv.dim(0).set_stride(2);
        input_uv.dim(2).set_stride(1).set_bounds(0, 2);
        output_uv.dim(0).set_stride(2);
        output_uv.dim(2).set_stride(1).set_bounds(0, 2);

        // Break the output into tiles.
        const int tile_width = 256;
        const int tile_height = 64;

        Expr strip_size_y = output_y.dim(1).extent() / 2;
        strip_size_y = (strip_size_y / 2) * 2;

        Expr strip_size_uv = output_uv.dim(1).extent() / 2;
        strip_size_uv = (strip_size_uv / 2) * 2;

        // Tiling Output with boundary condition as roundup 
        output_y
            .split(y, yo, y, strip_size_y)
            .tile(x, y, tx, ty, x, y, tile_width, tile_height, TailStrategy::RoundUp)
            .parallel(yo);
        output_uv
            .split(y, yo, y, strip_size_uv)
            .tile(x, y, tx, ty, x, y, tile_width, tile_height, TailStrategy::RoundUp)
            .parallel(yo);

        input_y_copy
            .compute_at(output_y, tx)          // Do the DMA Read at each tile in the horizontal order
            .store_in(MemoryType::LockedCache) // The  destination of DMA read is a locked cache
            .copy_to_host();                   // Schedule directie to hint that this is a DMA Read

        input_uv_copy
            .compute_at(output_uv, tx)         // Do the DMA Read at each tile in the horizontal order
            .copy_to_host()                    // Schedule directie to hint that this is a DMA Read
            .bound(c, 0, 2)                    // Limit the c dimension to 2(UV interleaved data) for optimization
            .store_in(MemoryType::LockedCache) // The  destination of DMA read is a locked  cache
            .reorder_storage(c, x, y)          // Re-order dimensions to make C innermost loop to access U and V sequentially   
            .reorder(c, x, y)
            .fuse(c, x, new_x)
            ;

        // Schedule the work in tiles (same for all DMA schedules).
        work_y.compute_at(output_y, tx)
              .store_in(MemoryType::LockedCache)  // After Processing, put the output in a locked Cache
              .vectorize(x, 128, TailStrategy::RoundUp)
              ;

        work_uv
            .compute_at(output_uv, tx)         // Do the Processing  at each tile in the horizontal order 
            .bound(c, 0, 2)                    // Limit the c dimension to 2(UV interleaved data) for optimization
            .store_in(MemoryType::LockedCache) // After Processing, put the Output in a locked cache
            .reorder_storage(c, x, y)          // Re-order dimensions to make C innermost loop to access U and V sequentially
            .reorder(c, x, y)
            .fuse(c, x, new_x)
            .vectorize(new_x, 128, TailStrategy::RoundUp)
            ;
    }

};

HALIDE_REGISTER_GENERATOR(DmaPipeline, dma_nv12_rw_halide)
