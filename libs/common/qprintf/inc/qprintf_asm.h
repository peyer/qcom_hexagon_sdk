//---------------------------------------------------------------------------
/// @brief
///   Define macro available from assembly to display registers along with a
///   message and filename[linenumber] and function available from C to display
///   all registers.
///
/// @detailed
///    TBD.
///
/// @param ACTION
///   Action to perform. Allowed actions: print_<format> or cust<n> with
///   format=int,hex,short,byte,ubyte,float and n=0-3.
///
/// @param REG
///   Register on which to take action. Allowed registers: R<n>, V<n>,
///   R_all, V_all, with n=0-31.
///
/// @param MSG
///   Message to display.
///
/// @return
///   none.
//---------------------------------------------------------------------------



#define qprintf(MSG,...) qprintf_macro #__FILE__, #__LINE__, MSG, #__VA_ARGS__

.set STACK_SIZE, 24
.macro	qprintf_macro FILE_NAME LINE_NUMBER MSG ARGS
.data
1:
.string "\MSG\()\0"
2:
.string "\ARGS\()\0"
3:
.string "\LINE_NUMBER\()\0"
4:
.string "\FILE_NAME\()\0"
.text
  {
   allocframe(#STACK_SIZE)                 // sp[STACK_SIZE]=r31:30  (sp refering to sp after stack allocation)
   memd(r29 + #(-STACK_SIZE-8)) = r29:28   // sp[0]=r29:28 
   r28 = ADD(PC,##1b@PCREL)
   } {
   memw(r29 + #8) = r28                    // sp[8]=&msg 
   r28 = ADD(PC,##2b@PCREL)
   } {
   memw(r29 + #12) = r28                   // sp[12]=&args 
   r28 = #\LINE_NUMBER\()
   } {
   memw(r29 + #16) = r28                   // sp[12]=line_number 
   r28 = ADD(PC,##4b@PCREL)
   } {
   memw(r29 + #20) = r28                   // sp[16]=&file_name
   call qprintf_asm
   } {
   r28 = memw(r29 + #0)
   deallocframe 
   }
.endm
