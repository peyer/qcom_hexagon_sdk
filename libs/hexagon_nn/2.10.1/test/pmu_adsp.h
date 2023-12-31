
const char *PMU_EVENT_NAMES[] = {
	"CYCLES",		//0x0
	"USERDEF1 (im2col)",
	"USERDEF2 (gemm)",
	"COMMITTED_PKT_ANY",
	"COMMITTED_PKT_BSB",
	"",
	"",
	"COMMITTED_PKT_B2B",
	"COMMITTED_PKT_SMT",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",		//0x10
	"ICACHE_DEMAND_MISS_CYCLES",
	"ICACHE_DEMAND_MISS",
	"DCACHE_DEMAND_MISS",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"ANY_IU_REPLAY",		//0x20
	"ANY_DU_REPLAY",
	"CU_EARLY_CANCEL",
	"",
	"",
	"1T_RUNNING_PKTS",
	"2T_RUNNING_PKTS",
	"3T_RUNNING_PKTS",
	"",
	"",
	"COMMITTED_INSTS",
	"COMMITTED_TC1_INSTS",
	"COMMITTED_PRIVATE_INSTS",
	"",
	"",
	"4T_RUNNING_PKTS",
	"COMMITTED_LOADS",		//0x30
	"COMMITTED_STORES",
	"COMMITTED_MEMOPS",
	"",
	"",
	"",
	"",
	"COMMITED_CONTROL_FLOW",
	"COMMITTED_PKT_CHANGED_FLOW",
	"COMMITTED_ENDLOOP",
	"",
	"1T_RUNNING_CYCLES",
	"2T_RUNNING_CYCLES",
	"3T_RUNNING_CYCLES",
	"4T_RUNNING_CYCLES",
	"",
	"AXI_READS",		//0x40
	"AXI_READ_32",
	"AXI_WRITE",
	"AXI_WRITE_32",
	"AHB_READ",
	"AHB_WRITE",
	"",
	"AXI_SLAVE_MULTI_BEAT",
	"AXI_SLAVE_SINGLE_BEAT",
	"AXI2_READ",
	"AXI2_READ_32",
	"AXI2_WRITE",
	"AXI2_WRITE_32",
	"AXI2_CONGESTION",
	"",
	"",
	"COMMITTED_FP_INSTS",		//0x50
	"BRANCH_MISPREDICT_DIRECTION",
	"BRANCH_MISPREDICT_TARGET",
	"",
	"",
	"",
	"",
	"",
	"JTLB_MISS",
	"",
	"COMMITTED_PKT_RETURN",
	"COMMITTED_PKT_INDIR_JUMP",
	"COMMITTED_BIMODAL_BRANCH",
	"BRANCH_QUEUE_FULL",
	"SMT_XU_CONFLICT",
	"",
	"",		//0x60
	"",
	"",
	"",
	"",
	"",
	"IU_LINE_FROM_LOOP_BUF",
	"",
	"IU_1_PKT_AVAIL",
	"",
	"IU_REQ_TO_L2_REPLAYED",
	"IU_PREFETCH_TO_L2",
	"ITLB_MISS",
	"IU_2_PKT_AVAIL",
	"IU_3_PKT_AVAIL",
	"IU_REQ_STALLED",
	"BIMODAL_UPDATE_DROPPED",	//0x70
	"IU_0_PKT_AVAIL",
	"IU_LINE_FALLTHROUGH",
	"IU_LEFTOVER_DROP",
	"L2FETCH_IU_ACCESS",
	"L2FETCH_IU_MISS",
	"L2_IU_ACCESS",
	"L2_IU_MISS",
	"L2_IU_PREFETCH_ACCESS",
	"L2_IU_PREFETCH_MISS",
	"",
	"",
	"L2_DU_READ_ACCESS",
	"L2_DU_READ_MISS",
	"L2FETCH_ACCESS",
	"L2FETCH_MISS",
	"L2_AXI_INTERLEAVE_DROP",		//0x80
	"L2_ACCESS",
	"L2_PIPE_CONFLICT",
	"L2_TAG_ARRAY_CONFLICT",
	"AXI_RD_CONGESTION",
	"AHB_CONGESTION",
	"",
	"TCM_DU_ACCESS",
	"TCM_DU_RD_ACCESS",
	"TCM_IU_ACCESS",
	"L2_CASTOUT",
	"L2_DU_STORE_ACCESS",
	"L2_DU_STORE_MISS",
	"L2_DU_PREFETCH_ACCESS",
	"L2_DU_PREFETCH_MISS",
	"L2_DU_RETURN_NOT_ACKED",
	"L2_DU_LOAD_2NDARY_MISS",		//0x90
	"L2FETCH_COMMAND",
	"L2FETCH_COMMAND_KIILLED",
	"L2FETCH_COMMAND_OVERWRITE",
	"",
	"",
	"",
	"L2_ACCESS_EVEN",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"ANY_DU_STALL",		//0xA0
	"DU_BANK_CONFLICT_REPLAY",
	"",
	"L2_FIFO_FULL_REPLAY",
	"DU_STOREBUF_FULL_REPLAY",
	"",
	"",
	"",
	"DU_FILL_REPLAY",
	"",
	"",
	"",
	"DU_READ_TO_L2",
	"DU_WRITE_TO_L2",
	"",
	"DCZERO_COMMITTED",
	"",		//0xB0
	"",
	"",
	"DTLB_MISS",
	"",
	"",
	"STOREBUF_HIT_REPLAY",
	"STOREBUF_FORCE_REPLAY",
	"",
	"SMT_BANK_CONFLICT",
	"SMT_PORT_CONFLICT",
	"",
	"",
	"PAGE_CROSS_REPLAY",
	"",
	"DU_DEMAND_2NDARY_MISS",
	"",		//0xC0
	"",
	"",
	"DCFETCH_COMMITTED",
	"DCFETCH_HIT",
	"DCFETCH_MISS",
	"",
	"",
	"DU_LOAD_UNCACHEABLE",
	"DU_DUAL_LOAD_UNCACHEABLE",
	"DU_STORE_UNCACHEABLE",
	"",
	"MISS_TO_PREFETCH",
	"",
	"AXI_LINE64_READ_REQ",
	"AXI_LINE64_WRITE_REQ",
	"AXI_WR_CONGESTION",		//0xD0
	"AHB_8_RD_REQ",
	"AXI_INCOMPLETE_WR_REQ",
	"L2FETCH_COMMAND_PAGE_TERMINATION",
	"REQ_STALL_WRITE_BUFFER_EXHAUSTION",
	"L2_DU_STORE_COALESCE",
	"",
	"",
	"",
	"",
	"",
	"",
	"REQ_STALL_EVICTION_BUFFER_EXHAUSTION",
	"AHB_MULTIBEAT_RD_REQ",
	"",
	"L2_DU_LOAD_2NDARY_MISS_ON_SW_PF",
	"",		//0xE0
	"",
	"",
	"",
	"",
	"",
	"ARCH_LOCK_PVIEW_CYCLES",
	"IU_NO_PKT_PVIEW_CYCLES",
	"IU_BRANCH_MISS_PVIEW_CYCLES",
	"DU_CACHE_MISS_PVIEW_CYCLES",
	"DU_BUSY_OTHER_PVIEW_CYCLES",
	"CU_BUSY_PVIEW_CYCLES",
	"SMT_DU_CONFLICT_PVIEW_CYCLES",
	"SMT_XU_CONFLICT_PVIEW_CYCLES",
	"",
	"COPROC_L2_STORE_TO_LOAD_MISS",
	"COPROC_PKT_THREAD",		//0xF0
	"COPROC_PKT_COUNT",
	"COPROC_POWER_THROTTLING_STALL_CYCLES",
	"COPROC_REGISTER_STALL_CYCLES",
	"COPROC_LOAD_STALL_CYCLES",
	"COPROC_STORE_STALL_CYCLES",
	"COPROC_BUSY_STALL_CYCLES",
	"COPROC_FREEZE_STALL_CYCLES",
	"COPROC_CORE_VFIFO_FULL_STALL",
	"COPROC_L2_STORE_ACCESS",
	"COPROC_L2_STORE_MISS",
	"COPROC_L2_LOAD_ACCESS",
	"COPROC_L2_LOAD_MISS",
	"COPROC_TCM_STORE_ACCESS",
	"COPROC_TCM_LOAD_ACCESS",
	"COPROC_L2_LOAD_2NDARY_MISS",
};
