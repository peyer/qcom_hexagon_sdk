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
        Func input_y_copy("input_y_copy"), input_uv_copy("input_uv_copy");

        Func work_y("work_y");
        Func work_uv("work_uv");

        input_y_copy(x, y) = input_y(x, y);
        work_y(x, y) = input_y_copy(x, y) * 2;
        output_y(x, y) = work_y(x, y);

        input_uv_copy(x, y, c) = input_uv(x, y, c);
        work_uv(x, y, c) = input_uv_copy(x, y, c) * 2;
        output_uv(x, y, c) = work_uv(x, y, c);

        Var tx("tx"), ty("ty");
        output_y.copy_to_device();
        output_uv.copy_to_device();

        output_y
            .compute_root();

        output_uv
            .compute_root()
            .bound(c, 0, 2)
            .reorder(c, x, y);

        // tweak stride/extent to handle UV deinterleaving
        input_uv.dim(0).set_stride(2);
        input_uv.dim(2).set_stride(1).set_bounds(0, 2);
        output_uv.dim(0).set_stride(2);
        output_uv.dim(2).set_stride(1).set_bounds(0, 2);

        // Break the output into tiles.
        const int tile_width = 256;
        const int tile_height = 64;

        output_y
            .tile(x, y, tx, ty, x, y, tile_width, tile_height, TailStrategy::RoundUp);
        output_uv
            .tile(x, y, tx, ty, x, y, tile_width, tile_height, TailStrategy::RoundUp);

        input_y_copy
            .compute_at(output_y, tx)
            .store_in(MemoryType::LockedCache)
            .copy_to_host();

        input_uv_copy
            .compute_at(output_uv, tx)
            .copy_to_host()
            .bound(c, 0, 2)
            .store_in(MemoryType::LockedCache)
            .reorder_storage(c, x, y);
        // Schedule the work in tiles (same for all DMA schedules).
        work_y.compute_at(output_y, tx)
              .store_in(MemoryType::LockedCache);

        work_uv
            .compute_at(output_uv, tx)
            .bound(c, 0, 2)
            .store_in(MemoryType::LockedCache)
            .reorder_storage(c, x, y);
    }

};

HALIDE_REGISTER_GENERATOR(DmaPipeline, dmatest)
