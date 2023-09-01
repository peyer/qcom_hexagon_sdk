	.text
	.p2align 5
	.global hvx_add_constant_asm

hvx_add_constant_asm:
/*void hvx_add_constant_asm(void* dst,             //r0: tx
                            void* src,             //r1: rx
                            unsigned int linesize, //r2: linesize
                            unsigned int constant, //r3: constant
                            int shift);*/          //r4: shift_num       

r7 = combine(r3.L, r3.L);
v0 = vsplat(r7);
r2 = lsr(r2, r4);
loop0(1f, r2)
.falign
1:
{
  v1 = vmem(r1++#1);
}
{
//Vd.uh=vadd(Vu.uh,Vv.uh):sat
  v2.uh = vadd(v1.uh, v0.uh):sat
}
{
  vmem(r0++#1) = v2;
}:endloop0
jumpr r31;

