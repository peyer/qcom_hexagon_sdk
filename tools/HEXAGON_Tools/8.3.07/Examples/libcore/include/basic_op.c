/*___________________________________________________________________________
  |                                                                           |
  | Basic arithmetic operators.                                               |
  |                                                                           |
  |___________________________________________________________________________|
 */

/*___________________________________________________________________________
  |                                                                           |
  |   Include-Files                                                           |
  |___________________________________________________________________________|
 */
#include "basic_op.h"



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : extract_h                                               |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Return the 16 MSB of L_var1.                                            |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32 ) whose value falls in the |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 extract_h (Word32 L_var1)
{
   Word16 var_out;

   var_out = (Word16) (L_var1 >> 16);

   return (var_out);
}

/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : extract_l                                               |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Return the 16 LSB of L_var1.                                            |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32 ) whose value falls in the |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 extract_l (Word32 L_var1)
{
   Word16 var_out;

   var_out = (Word16) L_var1;

   return (var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : add                                                     |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Performs the addition (var1+var2) with overflow control and saturation;|
  |    the 16 bit result is set at +32767 when overflow occurs or at -32768   |
  |    when underflow occurs.                                                 |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 add (Word16 var1, Word16 var2)
{
   Word16 var_out;
   Word32 L_sum;

   L_sum = (Word32) var1 + var2;
   var_out = saturate (L_sum);

   return (var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : sub                                                     |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Performs the subtraction (var1+var2) with overflow control and satu-   |
  |    ration; the 16 bit result is set at +32767 when overflow occurs or at  |
  |    -32768 when underflow occurs.                                          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 sub (Word16 var1, Word16 var2)
{
   Word16 var_out;
   Word32 L_diff;

   L_diff = (Word32) var1 - var2;
   var_out = saturate (L_diff);

   return (var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : negate                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Negate var1 with saturation, saturate in the case where input is -32768:|
  |                negate(var1) = sub(0,var1).                                |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 negate (Word16 var1)
{
   Word16 var_out;

   var_out = (Word16)((var1 == MIN_16) ? MAX_16 : -var1);

   return (var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : avg                                                     |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Performs (var1+var2+1)/2.                                              |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 avg (Word16 var1, Word16 var2)
{
   Word16 var_out;
   Word32 L_sum;

   L_sum = (Word32) var1 + (Word32)var2 + 1;
   var_out = (Word16)(L_sum >> 1);

   return (var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : navg                                                    |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Performs (var1 - var2+1)/2 with saturation.                            |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 navg (Word16 var1, Word16 var2)
{
   Word16 var_out;
   Word32 L_diff;

   L_diff = (Word32)var1 - (Word32)var2 + 1;
   var_out = saturate (L_diff>>1);

   return (var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : shl                                                     |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Arithmetically shift the 16 bit input var1 left var2 positions.Zero fill|
  |   the var2 LSB of the result. If var2 is negative, arithmetically shift   |
  |   var1 right by -var2 with sign extension. Saturate the result in case of |
  |   underflows or overflows.                                                |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 shl (Word16 var1, Word16 var2)
{
   Word16 var_out;
   Word32 result;

   if (var2 < 0)
   {
      if (var2 < -16)
         var2 = -16;
      var_out = shr (var1, (Word16)-var2);
   }
   else
   {
      result = (Word32) var1 *((Word32) 1 << var2);

      if ((var2 > 15 && var1 != 0) || (result != (Word32) ((Word16) result)))
      {
         var_out = (Word16)((var1 > 0) ? MAX_16 : MIN_16);
      }
      else
      {
         var_out = extract_l (result);
      }
   }

   return (var_out);
}

/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : shr                                                     |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Arithmetically shift the 16 bit input var1 right var2 positions with    |
  |   sign extension. If var2 is negative, arithmetically shift var1 left by  |
  |   -var2 with sign extension. Saturate the result in case of underflows or |
  |   overflows.                                                              |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 shr (Word16 var1, Word16 var2)
{
   Word16 var_out;

   if (var2 < 0)
   {
      if (var2 < -16)
         var2 = -16;
      var_out = shl (var1, (Word16)-var2);
   }
   else
   {
      if (var2 >= 15)
      {
         var_out = (Word16)((var1 < 0) ? -1 : 0);
      }
      else
      {
         if (var1 < 0)
         {
            var_out = (Word16)(~((~var1) >> var2));
         }
         else
         {
            var_out = (Word16)(var1 >> var2);
         }
      }
   }

   return (var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : shr_r                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Same as shr(var1,var2) but with rounding. Saturate the result in case of|
  |   underflows or overflows :                                               |
  |    - If var2 is greater than zero :                                       |
  |          if (sub(shl(shr(var1,var2),1),shr(var1,sub(var2,1))))            |
  |          is equal to zero                                                 |
  |                     then                                                  |
  |                     shr_r(var1,var2) = shr(var1,var2)                     |
  |                     else                                                  |
  |                     shr_r(var1,var2) = add(shr(var1,var2),1)              |
  |    - If var2 is less than or equal to zero :                              |
  |                     shr_r(var1,var2) = shr(var1,var2).                    |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 shr_r (Word16 var1, Word16 var2)
{
   Word16 var_out;

   if (var2 > 15)
   {
      var_out = 0;
   }
   else
   {
      var_out = shr (var1, var2);

      if (var2 > 0)
      {
         if ((var1 & ((Word16) 1 << (var2 - 1))) != 0)
         {
            var_out++;
         }
      }
   }

   return (var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : mult                                                    |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Performs the multiplication of var1 by var2 and gives a 16 bit result  |
  |    which is scaled i.e.:                                                  |
  |             mult(var1,var2) = extract_l(L_shr((var1 times var2),15)) and  |
  |             mult(-32768,-32768) = 32767.                                  |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 mult (Word16 var1, Word16 var2)
{
   Word16 var_out;
   Word32 L_product;

   L_product = (Word32) var1 *(Word32) var2;

   L_product = (L_product & (Word32) 0xffff8000L) >> 15;

   if (L_product & (Word32) 0x00010000L)
      L_product = L_product | (Word32) 0xffff0000L;

   var_out = saturate (L_product);

   return (var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : mult_r                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Same as mult with rounding, i.e.:                                       |
  |     mult_r(var1,var2) = extract_l(L_shr(((var1 * var2) + 16384),15)) and  |
  |     mult_r(-32768,-32768) = 32767.                                        |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 mult_r (Word16 var1, Word16 var2)
{
   Word16 var_out;
   Word32 L_product_arr;

   L_product_arr = (Word32) var1 *(Word32) var2;       /* product */
   L_product_arr += (Word32) 0x00004000L;      /* round */
   L_product_arr &= (Word32) 0xffff8000L;
   L_product_arr >>= 15;       /* shift */

   if (L_product_arr & (Word32) 0x00010000L)   /* sign extend when necessary */
   {
      L_product_arr |= (Word32) 0xffff0000L;
   }
   var_out = saturate (L_product_arr);

   return (var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : mac_r                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit    |
  |   result to L_var3 with saturation. Round the LS 16 bits of the result    |
  |   into the MS 16 bits with saturation and shift the result right by 16.   |
  |   Return a 16 bit result.                                                 |
  |            mac_r(L_var3,var1,var2) = round(L_mac(L_var3,var1,var2))       |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0x0000 8000 <= L_var_out <= 0x0000 7fff.              |
  |___________________________________________________________________________|
 */

Word16 mac_r (Word32 L_var3, Word16 var1, Word16 var2)
{
   Word16 var_out;

   L_var3 = L_mac (L_var3, var1, var2);
   L_var3 = L_add (L_var3, (Word32) 0x00008000L);
   var_out = extract_h (L_var3);

   return (var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : msu_r                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
  |   bit result to L_var3 with saturation. Round the LS 16 bits of the res-  |
  |   ult into the MS 16 bits with saturation and shift the result right by   |
  |   16. Return a 16 bit result.                                             |
  |            msu_r(L_var3,var1,var2) = round(L_msu(L_var3,var1,var2))       |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0x0000 8000 <= L_var_out <= 0x0000 7fff.              |
  |___________________________________________________________________________|
 */

Word16 msu_r (Word32 L_var3, Word16 var1, Word16 var2)
{
   Word16 var_out;

   L_var3 = L_msu (L_var3, var1, var2);
   L_var3 = L_add (L_var3, (Word32) 0x00008000L);
   var_out = extract_h (L_var3);

   return (var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : abs_s                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Absolute value of var1; abs_s(-32768) = 32767.                         |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0x0000 0000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 abs_s (Word16 var1)
{
   Word16 var_out;

   if (var1 == MIN_16)
   {
      var_out = MAX_16;
   }
   else
   {
      if (var1 < 0)
      {
         var_out = (Word16)-var1;
      }
      else
      {
         var_out = var1;
      }
   }

   return (var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : rnd                                                     |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Round the lower 16 bits of the 32 bit input number into the MS 16 bits  |
  |   with saturation. Shift the resulting bits right by 16 and return the 16 |
  |   bit number:                                                             |
  |               rnd(L_var1) = extract_h(L_add(L_var1,32768))                |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32 ) whose value falls in the |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 rnd (Word32 L_var1)
{
   Word16 var_out;
   Word32 L_rounded;

   L_rounded = L_add (L_var1, (Word32) 0x00008000L);
   var_out = extract_h (L_rounded);

   return (var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : saturate                                                |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Limit the 32 bit input to the range of a 16 bit word.                  |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 saturate (Word32 L_var1)
{
   Word16 var_out;

   if (L_var1 > 0X00007fffL)
   {
      var_out = MAX_16;
   }
   else if (L_var1 < (Word32) 0xffff8000L)
   {
      var_out = MIN_16;
   }
   else
   {
      var_out = extract_l (L_var1);
   }

   return (var_out);
}




/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : norm_s                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Produces the number of left shift needed to normalize the 16 bit varia- |
  |   ble var1 for positive values on the interval with minimum of 16384 and  |
  |   maximum of 32767, and for negative values on the interval with minimum  |
  |   of -32768 and maximum of -16384; in order to normalize the result, the  |
  |   following operation must be done :                                      |
  |                    norm_var1 = shl(var1,norm_s(var1)).                    |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0x0000 0000 <= var_out <= 0x0000 000f.                |
  |___________________________________________________________________________|
 */

Word16 norm_s (Word16 var1)
{
   Word16 var_out;

   if (var1 == 0)
   {
      var_out = 0;
   }
   else
   {
      if (var1 == -1)
      {
         var_out = 15;
      }
      else
      {
         if (var1 < 0)
         {
            var1 = (Word16)~var1;
         }
         for (var_out = 0; var1 < 0x4000; var_out++)
         {
            var1 <<= 1;
         }
      }
   }

   return (var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : max                                                     |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Return the maximum value of two 16-bit variables var1 and var2.        |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 max (Word16 var1, Word16 var2)
{
   Word16 var_out;

   var_out = ((Word32)var1 > (Word32)var2) ? var1 : var2 ; 

   return (var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : min                                                     |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Return the minimum value of two 16-bit variables var1 and var2.        |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word16 min (Word16 var1, Word16 var2)
{
   Word16 var_out;

   var_out = ((Word32)var1 < (Word32)var2) ? var1 : var2 ; 

   return (var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_deposit_h                                             |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Deposit the 16 bit var1 into the 16 MS bits of the 32 bit output. The   |
  |   16 LS bits of the output are zeroed.                                    |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= var_out <= 0x7fff 0000.                |
  |___________________________________________________________________________|
 */

Word32 L_deposit_h (Word16 var1)
{
   Word32 L_var_out;

   L_var_out = (Word32) var1 << 16;

   return (L_var_out);
}

/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_deposit_l                                             |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Deposit the 16 bit var1 into the 16 LS bits of the 32 bit output. The   |
  |   16 MS bits of the output are sign extended.                             |
  |                                                                           |
  |   Complexity weight : 2                                                   |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0xFFFF 8000 <= var_out <= 0x0000 7fff.                |
  |___________________________________________________________________________|
 */

Word32 L_deposit_l (Word16 var1)
{
   Word32 L_var_out;

   L_var_out = (Word32) var1;

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : combine                                                 |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |     Costruct a 32-bit word with two 16-bit short signed integer.          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var_h                                                                  |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var_l                                                                  |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32-bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 combine (Word16 var_h, Word16 var_l)
{
   Word32 L_var_out;

   L_var_out = ((Word32)var_h<<16) | ((Word32)var_l&0x0FFFF);

   return (L_var_out);
}




/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_add                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   32 bits addition of the two 32 bits variables (L_var1+L_var2) with      |
  |   overflow control and saturation; the result is set at +2147483647 when  |
  |   overflow occurs or at -2147483648 when underflow occurs.                |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_add (Word32 L_var1, Word32 L_var2)
{
   Word32 L_var_out;

   L_var_out = L_var1 + L_var2;

   if (((L_var1 ^ L_var2) & MIN_32) == 0)
   {
      if ((L_var_out ^ L_var1) & MIN_32)
      {
         L_var_out = (L_var1 < 0) ? MIN_32 : MAX_32;
      }
   }

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_sub                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   32 bits subtraction of the two 32 bits variables (L_var1-L_var2) with   |
  |   overflow control and saturation; the result is set at +2147483647 when  |
  |   overflow occurs or at -2147483648 when underflow occurs.                |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_sub (Word32 L_var1, Word32 L_var2)
{
   Word32 L_var_out;

   L_var_out = L_var1 - L_var2;

   if (((L_var1 ^ L_var2) & MIN_32) != 0)
   {
      if ((L_var_out ^ L_var1) & MIN_32)
      {
         L_var_out = (L_var1 < 0L) ? MIN_32 : MAX_32;
      }
   }

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_negate                                                |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Negate the 32 bit variable L_var1 with saturation; saturate in the case |
  |   where input is -2147483648 (0x8000 0000).                               |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_negate (Word32 L_var1)
{
   Word32 L_var_out;

   L_var_out = (L_var1 == MIN_32) ? MAX_32 : -L_var1;

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_avg                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   32 bits average of the two 32 bits variables L_var1 and L_var2 with     |
  |   rounding.                                                               |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_avg (Word32 L_var1, Word32 L_var2)
{
   Word32 L_var_out;

   L_var_out = (L_var1 + L_var2 + 1) >> 1;

   if (((L_var1 ^ L_var2) & MIN_32) == 0)
   {
      if ((L_var_out ^ L_var1) & MIN_32)
      {
         L_var_out ^= MIN_32;
      }
   }

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_navg                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   32 bits subtraction of the two 32 bits variables (L_var1-L_var2), and   |
  |   shift the result right by 1 with rounding and saturation.               |
  |   L_navg(0x7fff ffff, 0x8000 80000) = 0x7fff ffff.                        |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_navg (Word32 L_var1, Word32 L_var2)
{
   Word32 L_var_out;

   if ((L_var1 == MAX_32) && (L_var2 == MIN_32) )
   {
      L_var_out = MAX_32;
   }
   else
   {
      L_var_out = (L_var1 - L_var2 + 1) >> 1;

      if (((L_var1 ^ L_var2) & MIN_32) != 0)
      {
         if ((L_var_out ^ L_var1) & MIN_32)
         {
            L_var_out ^=  MIN_32;
         }
      }
   }

   return (L_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_mult                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   L_mult is the 32 bit result of the multiplication of var1 times var2    |
  |   with one shift left i.e.:                                               |
  |        L_mult(var1,var2) = L_shl((var1 times var2),1) and                 |
  |        L_mult(-32768,-32768) = 2147483647.                                |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_mult (Word16 var1, Word16 var2)
{
   Word32 L_var_out;

   L_var_out = (Word32) var1 *(Word32) var2;

   if (L_var_out != (Word32) 0x40000000L)
   {
      L_var_out *= 2;
   }
   else
   {
      L_var_out = MAX_32;
   }

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_mac                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit    |
  |   result to L_var3 with saturation, return a 32 bit result:               |
  |        L_mac(L_var3,var1,var2) = L_add(L_var3,L_mult(var1,var2)),         |
  |                            when (var1 !=0x8000)||(var2 != 0x8000)         |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_mac (Word32 L_var3, Word16 var1, Word16 var2)
{
   Word32 L_var_out;
   Word32 L_product;

   L_product = L_mult (var1, var2);
   L_var_out = L_add (L_var3, L_product);

   /* compensate saturation case */
   if (L_product == MAX_32)
   {
      L_var_out = L_add(L_var_out, 1);
   }

   return (L_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_msu                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
  |   bit result to L_var3 with saturation, return a 32 bit result:           |
  |        L_msu(L_var3,var1,var2) = L_sub(L_var3,L_mult(var1,var2)),         |
  |                            when (var1 !=0x8000)||(var2 != 0x8000)         |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_msu (Word32 L_var3, Word16 var1, Word16 var2)
{
   Word32 L_var_out;
   Word32 L_product;

   L_product = L_mult (var1, var2);
   L_var_out = L_sub (L_var3, L_product);

   /* compensate saturation case */
   if (L_product == MAX_32)
   {
      L_var_out = L_sub(L_var_out, 1);
   }

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_dmac                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply var1 by var2, var3 by var4,  and shift the results left by 1.  |
  |   Add the two 32 bit  results to L_var3 with saturation.                  |
  |        L_mac(L_var5,var1,var2, var3, var4) =                              |
  |            saturation(L_var3 + (var1*var2 + var3*var4)<<1))               |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var5   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var5 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var1                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var3                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |    var4                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_dmac (Word32 L_var5, Word16 var1, Word16 var2, Word16 var3, Word16 var4)
{
   Word32 L_var_out;
   Word64 LL_sum;

   LL_sum  =  (Word32)var1 *(Word32)var2;
   LL_sum +=  (Word32)var3 *(Word32)var4;
   LL_sum +=  LL_sum;
   LL_sum += (Word64)L_var5;

   L_var_out = L_sat (LL_sum);

   return (L_var_out);
}




/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_shl                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Arithmetically shift the 32 bit input L_var1 left var2 positions. Zero  |
  |   fill the var2 LSB of the result. If var2 is negative, arithmetically    |
  |   shift L_var1 right by -var2 with sign extension. Saturate the result in |
  |   case of underflows or overflows.                                        |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_shl (Word32 L_var1, Word16 var2)
{
   Word32 L_var_out = 0L;

   if (var2 <= 0)
   {
      if (var2 < -32)
         var2 = -32;
      L_var_out = L_shr (L_var1, (Word16)-var2);
   }
   else
   {
      for (; var2 > 0; var2--)
      {
         if (L_var1 > (Word32) 0X3fffffffL)
         {
            L_var_out = MAX_32;
            break;
         }
         else
         {
            if (L_var1 < (Word32) 0xc0000000L)
            {
               L_var_out = MIN_32;
               break;
            }
         }
         L_var1 *= 2;
         L_var_out = L_var1;
      }
   }

   return (L_var_out);
}

/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_shr                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Arithmetically shift the 32 bit input L_var1 right var2 positions with  |
  |   sign extension. If var2 is negative, arithmetically shift L_var1 left   |
  |   by -var2 and zero fill the -var2 LSB of the result. Saturate the result |
  |   in case of underflows or overflows.                                     |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_shr (Word32 L_var1, Word16 var2)
{
   Word32 L_var_out;

   if (var2 < 0)
   {
      if (var2 < -32)
         var2 = -32;
      L_var_out = L_shl (L_var1, (Word16)-var2);

   }
   else
   {
      if (var2 >= 31)
      {
         L_var_out = (L_var1 < 0L) ? -1 : 0;
      }
      else
      {
         if (L_var1 < 0)
         {
            L_var_out = ~((~L_var1) >> var2);
         }
         else
         {
            L_var_out = L_var1 >> var2;
         }
      }
   }

   return (L_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_shr_r                                                 |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Same as L_shr(L_var1,var2) but with rounding. Saturate the result in    |
  |   case of underflows or overflows :                                       |
  |    - If var2 is greater than zero :                                       |
  |          if (L_sub(L_shl(L_shr(L_var1,var2),1),L_shr(L_var1,sub(var2,1))))|
  |          is equal to zero                                                 |
  |                     then                                                  |
  |                     L_shr_r(L_var1,var2) = L_shr(L_var1,var2)             |
  |                     else                                                  |
  |                     L_shr_r(L_var1,var2) = L_add(L_shr(L_var1,var2),1)    |
  |    - If var2 is less than or equal to zero :                              |
  |                     L_shr_r(L_var1,var2) = L_shr(L_var1,var2).            |
  |                                                                           |
  |   Complexity weight : 3                                                   |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= var_out <= 0x7fff ffff.                |
  |___________________________________________________________________________|
 */

Word32 L_shr_r (Word32 L_var1, Word16 var2)
{
   Word32 L_var_out;

   if (var2 > 31)
   {
      L_var_out = 0;
   }
   else
   {
      L_var_out = L_shr (L_var1, var2);

      if (var2 > 0)
      {
         if ((L_var1 & ((Word32) 1 << (var2 - 1))) != 0)
         {
            L_var_out++;
         }
      }
   }

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_asl                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Arithmetically shift the 32 bit input L_var1 left var2 positions        |
  |   without saturation. If var2 is negative, arithmetically shift L_var1    |
  |   right by -var2 with sign extension.                                     |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_asl (Word32 L_var1, Word16 var2)
{
   Word32 L_var_out = 0L;

   if (var2 <= 0)
   {
      if (var2 < -32)
         var2 = -32;
      L_var_out = L_asr (L_var1, (Word16)-var2);
   }
   else
   {
      if (var2 > 32)
         var2 = 32;

      for (; var2 > 0; var2--)
      {
         L_var1 *= 2;
         L_var_out = L_var1;
      }
   }

   return (L_var_out);
}

/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_asr                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Arithmetically shift the 32 bit input L_var1 right var2 positions with  |
  |   sign extension. If var2 is negative, arithmetically shift L_var1 left   |
  |   by -var2 and zero fill the -var2 LSB of the result without saturation.  |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_asr (Word32 L_var1, Word16 var2)
{
   Word32 L_var_out;

   if (var2 < 0)
   {
      if (var2 < -32)
         var2 = -32;
      L_var_out = L_asl (L_var1, (Word16)-var2);

   }
   else
   {
      if (var2 >= 31)
      {
         L_var_out = (L_var1 < 0L) ? -1 : 0;
      }
      else
      {
         if (L_var1 < 0)
         {
            L_var_out = ~((~L_var1) >> var2);
         }
         else
         {
            L_var_out = L_var1 >> var2;
         }
      }
   }

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_abs                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Absolute value of L_var1; Saturate in case where the input is          |
  |                                                               -214783648  |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x0000 0000 <= var_out <= 0x7fff ffff.                |
  |___________________________________________________________________________|
 */

Word32 L_abs (Word32 L_var1)
{
   Word32 L_var_out;

   if (L_var1 == MIN_32)
   {
      L_var_out = MAX_32;
   }
   else
   {
      if (L_var1 < 0)
      {
         L_var_out = -L_var1;
      }
      else
      {
         L_var_out = L_var1;
      }
   }

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : norm_l                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Produces the number of left shifts needed to normalize the 32 bit varia-|
  |   ble L_var1 for positive values on the interval with minimum of          |
  |   1073741824 and maximum of 2147483647, and for negative values on the in-|
  |   terval with minimum of -2147483648 and maximum of -1073741824; in order |
  |   to normalize the result, the following operation must be done :         |
  |                   norm_L_var1 = L_shl(L_var1,norm_l(L_var1)).             |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0x0000 0000 <= var_out <= 0x0000 001f.                |
  |___________________________________________________________________________|
 */

Word16 norm_l (Word32 L_var1)
{
   Word16 var_out;

   if (L_var1 == 0)
   {
      var_out = 0;
   }
   else
   {
      if (L_var1 == (Word32) 0xffffffffL)
      {
         var_out = 31;
      }
      else
      {
         if (L_var1 < 0)
         {
            L_var1 = ~L_var1;
         }
         for (var_out = 0; L_var1 < (Word32) 0x40000000L; var_out++)
         {
            L_var1 <<= 1;
         }
      }
   }

   return (var_out);
}




/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_max                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Return maximun value of the two 32 bits variables L_var1 and L_var2.    |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_max (Word32 L_var1, Word32 L_var2)
{
   Word32 L_var_out;

   L_var_out = (L_var1 > L_var2) ? L_var1 : L_var2;

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_min                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Return minimun value of the two 32 bits variables L_var1 and L_var2.    |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_min (Word32 L_var1, Word32 L_var2)
{
   Word32 L_var_out;

   L_var_out = (L_var1 < L_var2) ? L_var1 : L_var2;

   return (L_var_out);
}




/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_mult32x16                                             |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply a 32-bit long signed integer L_var1 by a 16-bit short signed   |
  |   integer var2, shift result left by 1 with saturateion and truncate      |
  |   to 32-bit long signed integer.                                          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_mult32x16(Word32 L_var1, Word16 var2)
{
   Word32 L_var_out;
   Word32 L_var_h, L_var_l;

   L_var_h = (Word32)extract_h(L_var1)  *(Word32)var2;
   L_var_l = (L_var1&0x0000ffff) *(Word32)var2;

   L_var_out = (L_var_h<<1) + (L_var_l>>15);

   /* check saturation */
   if (L_var_out == MIN_32)
   {
      L_var_out = MAX_32;
   }

   return (L_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_mult32x16r                                            |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply a 32-bit long signed integer L_var1 by a 16-bit short signed   |
  |   integer var2, shift result left by 1 with saturateion and round         |
  |   to 32-bit long signed integer.                                          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var2                                                                   |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_mult32x16r(Word32 L_var1, Word16 var2)
{
   Word32 L_var_out;
   Word32 L_var_h, L_var_l, L_var_rb;

   L_var_h = (Word32)extract_h(L_var1)  *(Word32)var2;
   L_var_l = (L_var1&0x0000ffff) *(Word32)var2;

   L_var_rb = (L_var_l>>14)&1;

   L_var_out = (L_var_h<<1) + (L_var_l>>15);

   /* check saturation */
   if (L_var_out == MIN_32)
   {
      L_var_out = MAX_32;
   }
   else
   {
      L_var_out = L_add(L_var_out, L_var_rb);
   }

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_mult32                                                |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply a 32-bit long signed integer L_var1 by a 32-bit long signed    |
  |   intege L_var2, shift result left by 1 with saturateion and return upper |
  |   32-bit.                                                                 |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_mult32(Word32 L_var1, Word32 L_var2)
{
   Word32 L_var_out;
   Word32 L_var_hL , L_var_lh, L_var_ll;
   Word16  hi1, hi2;
   UWord16 lo1, lo2;

   hi1 = extract_h(L_var1);  lo1 = extract_l(L_var1);
   hi2 = extract_h(L_var2);  lo2 = extract_l(L_var2);

   L_var_ll  = (Word32)lo1 *(Word32)lo2;
   L_var_hL  = (Word32)hi1 *(Word32)lo2;
   L_var_hL += ((UWord32)L_var_ll>>16);
   L_var_lh  = (Word32)lo1 *(Word32)hi2;

   /* addition on 15 LSB */
   L_var_out = (L_var_hL &0x7FFF) + (L_var_lh &0x7FFF);
   L_var_out >>= 15;

   /* addition on other bits */
   L_var_out += (L_var_hL >>15) + (L_var_lh >>15);
   L_var_out += ( (Word32)hi1 *(Word32)hi2 ) <<1;

   /* check saturation */
   if (L_var_out == MIN_32)
   {
      L_var_out = MAX_32;
   }

   return (L_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_mac32                                                 |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply and accumulation for 32-bit (saturation is only with           |
  |   accumulation).                                                          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var3                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_mac32(Word32 L_var3, Word32 L_var1, Word32 L_var2)
{
   Word32 L_var_out;
   Word32 L_product;

   L_product = L_mult32(L_var1, L_var2);
   L_var_out = L_add(L_var3, L_product);

   /* compensate saturation case */
   if ( (L_var1 == MIN_32)&&(L_var2 == MIN_32) )
   {
      L_var_out = L_add(L_var_out, 1);
   }

   return (L_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_msu32                                                 |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply and  subtraction for 32-bit (saturation is only with           |
  |   accumulation).                                                          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var3                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_msu32(Word32 L_var3, Word32 L_var1, Word32 L_var2)
{
   Word32 L_var_out;
   Word32 L_product;

   L_product = L_mult32(L_var1, L_var2);
   L_var_out = L_sub(L_var3, L_product);

   /* compensate saturation case */
   if ( (L_var1 == MIN_32)&&(L_var2 == MIN_32) )
   {
      L_var_out = L_sub(L_var_out, 1);
   }

   return (L_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_imult32h                                              |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Performs multiplication of a 32-bit long signed integer L_var1 by a     |
  |   32-bit long signed integer L_var2 and returns 32 MSB of the result.     |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             32 bit long signed integer (Word32) whose value falls in      |
  |             the range : 0xc000 0000<= LL_var_out <= 0x4000 0000.          |
  |___________________________________________________________________________|
 */

Word32 L_imult32h(Word32 L_var1, Word32 L_var2)
{
   Word32 L_var_out;
   Word64 LL_product;

   LL_product = LL_imult32(L_var1, L_var2);
   L_var_out = L_extract_h32(LL_product);

   return (L_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_imult32hr                                             |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Performs multiplication of a 32-bit long signed integer L_var1 by a     |
  |   32-bit long signed integer L_var2 and round the 64-bit result to        |
  |   a 32-bit long signed integer.                                           |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             32 bit long signed integer (Word32) whose value falls in      |
  |             the range : 0xc000 0000< LL_var_out <= 0x4000 0000.           |
  |___________________________________________________________________________|
 */

Word32 L_imult32hr(Word32 L_var1, Word32 L_var2)
{
   Word32 L_var_out;
   Word64 LL_product;

   LL_product  = LL_imult32(L_var1, L_var2);
   LL_product += 0x080000000LL;
   L_var_out = L_extract_h32(LL_product);

   return (L_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : LL_imult32                                              |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Performs multiplication of a 32-bit long signed integer L_var1 by a     |
  |   32-bit long signed integer L_var2 and returns 64-bit result.            |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    L_var2                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             64 bit long long signed integer (Word64) whose value falls in |
  |             the range : MIN_64<= LL_var_out <= MAX_64.                    |
  |___________________________________________________________________________|
 */

Word64 LL_imult32(Word32 L_var1, Word32 L_var2)
{
   Word64 LL_var_out;
   Word16  hi1, hi2;
   UWord16 lo1, lo2;

   hi1 = extract_h(L_var1);  lo1 = extract_l(L_var1);
   hi2 = extract_h(L_var2);  lo2 = extract_l(L_var2);

   LL_var_out  = (Word64)((UWord32)( (Word32)lo1 *(Word32)lo2) );
   LL_var_out += (Word64)( (Word32)hi1 *(Word32)lo2 ) << 16;
   LL_var_out += (Word64)( (Word32)lo1 *(Word32)hi2 ) << 16;
   LL_var_out += (Word64)( (Word32)hi1 *(Word32)hi2 ) << 32;

   return (LL_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : LL_imult32u                                             |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Performs multiplication of a 32-bit long unsigned integer L_var1 by a   |
  |   32-bit long unsigned integer L_var2 and returns 64-bit result.          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long unsigned integer (UWord32) whose value falls in   |
  |             the range : 0x0000 0000 <= L_var1 <= 0xffff ffff.             |
  |                                                                           |
  |    L_var2                                                                 |
  |             32 bit long unsigned integer (UWord32) whose value falls in   |
  |             the range : 0x0000 0000 <= L_var1 <= 0xffff ffff.             |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             64 bit long long unsigned integer (UWord64) whose value falls |
  |             in the range : 0<= LL_var_out <= 0xffff ffff ffff ffff.       |
  |___________________________________________________________________________|
 */

UWord64 LL_imult32u(UWord32 L_var1, UWord32 L_var2)
{
   UWord64 LL_var_out;
   UWord16 hi1, hi2, lo1, lo2;

   hi1 = extract_h(L_var1);  lo1 = extract_l(L_var1);
   hi2 = extract_h(L_var2);  lo2 = extract_l(L_var2);

   LL_var_out  = (UWord64)((UWord32)( (Word32)lo1 *(Word32)lo2 ));
   LL_var_out += (UWord64)((UWord32)( (Word32)hi1 *(Word32)lo2 ))<< 16;
   LL_var_out += (UWord64)((UWord32)( (Word32)lo1 *(Word32)hi2 )) << 16;
   LL_var_out += (UWord64)((Word32)hi1 *(Word32)hi2) << 32;

   return (LL_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_sat                                                   |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Limit the 64 bit input to the range of a 32 bit word.                  |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64 bit signed integer (Word64) whose value falls in the range |
  |             : 0x8000 0000 0000 0000 <= LL_var1 <= 0x7fff ffff ffff ffff.  |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_sat (Word64 LL_var1)
{
   Word32 L_var_out;

   if (LL_var1 > (Word64)MAX_32)
   {
      L_var_out = MAX_32;
   }
   else if (LL_var1 < (Word64)MIN_32)
   {
      L_var_out = MIN_32;
   }
   else
   {
      L_var_out = (Word32)(LL_var1);
   }

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_extract_h32                                           |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Return the 32 MSB of LL_var1.                                          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64 bit signed integer (Word64) whose value falls in the range |
  |             : 0x8000 0000 0000 0000 <= LL_var1 <= 0x7fff ffff ffff ffff.  |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_extract_h32 (Word64 LL_var1)
{
   Word32 L_var_out;

   L_var_out = (Word32) (LL_var1 >> 32);

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_extract_l32                                           |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Return the 32 LSB of LL_var1.                                          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64 bit signed integer (Word64) whose value falls in the range |
  |             : 0x8000 0000 0000 0000 <= LL_var1 <= 0x7fff ffff ffff ffff.  |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
  |___________________________________________________________________________|
 */

Word32 L_extract_l32 (Word64 LL_var1)
{
   Word32 L_var_out;

   L_var_out = (Word32)LL_var1;

   return (L_var_out);
}




/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_combine                                               |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |     Costruct a long long word with two 32-bit long signed integer.        |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var_h                                                                |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
  |                                                                           |
  |    L_var_l                                                                |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             64 bit long long signed integer (Word64)                      |
  |___________________________________________________________________________|
 */

Word64 L_combine(Word32 L_var_h, Word32 L_var_l)
{
   Word64 LL_var_out;

   LL_var_out = ((Word64)L_var_h<<32) | ((Word64)L_var_l&0x00FFFFFFFFLL);

   return (LL_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : LL_shl                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Arithmetically shift the 64 bit input LL_var1 left L_var2 positions.    |
  |   Zero fill the var2 LSB of the result. If L_var2 is negative,            |
  |   arithematically shift LL_var1 right by -var2 with sign extension.       |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64 bit long long signed integer (Word64) whose value falls in |
  |             the range :                                                   |
  |             0x8000 0000 0000 0000<= LL_var1 <= 0x7fff ffff ffff ffff.     |
  |                                                                           |
  |    L_var2                                                                 |
  |             32 bit short signed integer (Word32) whose value falls in the |
  |             range : -64 <= var1 <= 64.                                    |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             64 bit long long signed integer (Word64).                     |
  |___________________________________________________________________________|
 */

Word64 LL_shl (Word64 LL_var1, Word32 L_var2)
{
   Word64 LL_var_out;

   if (L_var2 < 0)
   {
      LL_var_out = LL_shr (LL_var1, -L_var2);
   }
   else
   {
      LL_var_out = LL_var1;

      for (; L_var2 > 0; L_var2--)
      {
         LL_var_out <<= 1;
      }
   }

   return (LL_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : LL_shr                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Arithmetically shift the 64 bit input LL_var1 right L_var2 positions.   |
  |   if L_var is negative, zero fill the -L_var2 LSB of the result. Otherwise|
  |   arithmetically shift LL_var1 right by L_var2 with sign extension.       |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64 bit long long signed integer (Word64) whose value falls in |
  |             the range :                                                   |
  |             0x8000 0000 0000 0000<= LL_var1 <= 0x7fff ffff ffff ffff.     |
  |                                                                           |
  |    L_var2                                                                 |
  |             32 bit short signed integer (Word32) whose value falls in the |
  |             range : -64 <= var1 <= 64.                                    |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             64 bit long long signed integer (Word64).                     |
  |___________________________________________________________________________|
 */

Word64 LL_shr (Word64 LL_var1, Word32 L_var2)
{
   Word64 LL_var_out;

   if (L_var2 < 0)
   {
      LL_var_out = LL_shl (LL_var1, -L_var2);
   }
   else
   {
      LL_var_out = LL_var1;

      for (; L_var2 > 0; L_var2--)
      {
         LL_var_out >>= 1;
      }
   }

   return (LL_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : conjugate                                               |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Return conjugate of a short complex number with saturation.            | 
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             real and imaginary part of L_va1 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var1) <= 0x0000 7fff.              |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             real and imaginary part of L_va_out are 16 bit short signed   |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var_out) <= 0x0000 7fff.           |
  |___________________________________________________________________________|
 */

CWord2x16 conjugate (CWord2x16 L_var1)
{
   CWord2x16 L_var_out;
   Word16 var_i;
   
   var_i = negate(imag(L_var1));

   L_var_out = complex(real(L_var1), var_i);

   return (L_var_out);
}




/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : cadd                                                    |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Performs the complex addition (L_var1+L_var2) with overflow control    |
  |    and saturation                                                         |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             real and imaginary part of L_va1 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var1) <= 0x0000 7fff.              |
  |                                                                           |
  |    L_var2                                                                 |
  |             real and imaginary part of L_va2 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var2) <= 0x0000 7fff.              |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             real and imaginary part of L_va_out are 16 bit short signed   |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var_out) <= 0x0000 7fff.           |
  |___________________________________________________________________________|
 */

CWord2x16 cadd (CWord2x16 L_var1, CWord2x16 L_var2)
{
   CWord2x16 L_var_out;
   Word16 sum_r, sum_i;
   
   sum_r = add(real(L_var1), real(L_var2));
   sum_i = add(imag(L_var1), imag(L_var2));
   L_var_out = complex(sum_r, sum_i);

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : csub                                                    |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |    Performs the complex subtraction (L_var1-L_var2) with overflow control |
  |    and saturation                                                         |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             real and imaginary part of L_va1 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var1) <= 0x0000 7fff.              |
  |                                                                           |
  |    L_var2                                                                 |
  |             real and imaginary part of L_va2 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var2) <= 0x0000 7fff.              |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             real and imaginary part of L_va_out are 16 bit short signed   |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var_out) <= 0x0000 7fff.           |
  |___________________________________________________________________________|
 */

CWord2x16 csub (CWord2x16 L_var1, CWord2x16 L_var2)
{
   CWord2x16 L_var_out;
   Word16 sum_r, sum_i;
   
   sum_r = sub(real(L_var1), real(L_var2));
   sum_i = sub(imag(L_var1), imag(L_var2));
   L_var_out = complex(sum_r, sum_i);

   return (L_var_out);
}




/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : cmult_r                                                 |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Perform complex mult with rounding.                                     |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             real and imaginary part of L_va1 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var1) <= 0x0000 7fff.              |
  |                                                                           |
  |    L_var2                                                                 |
  |             real and imaginary part of L_va2 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var2) <= 0x0000 7fff.              |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             real and imaginary part of L_va_out are 16 bit short signed   |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var_out) <= 0x0000 7fff.           |
  |___________________________________________________________________________|
 */

CWord2x16 cmult_r (CWord2x16 L_var1, CWord2x16 L_var2)
{
   CWord2x16 L_var_out;
   Word32 L_product_r, L_product_i, L_sum_r, L_sum_i;

   /* calculate real part      */
   L_product_r = L_mult(real(L_var1), real(L_var2));
   L_sum_r = L_msu(L_product_r, imag(L_var1), imag(L_var2));
   if (L_product_r == MAX_32)
   {
      L_sum_r = L_add(L_sum_r, 1);
   }

   /* calculate imaginary part */
   L_product_i = L_mult(real(L_var1), imag(L_var2));
   L_sum_i = L_mac(L_product_i, imag(L_var1), real(L_var2));
   if (L_product_i == MAX_32)
   {
      L_sum_i = L_add(L_sum_i, 1);
   }

   L_var_out = complex(rnd(L_sum_r), rnd(L_sum_i));

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : cmult_cr                                                |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Perform complex mult L_var1 * conjugate(L_var2) with rounding.          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             real and imaginary part of L_va1 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var1) <= 0x0000 7fff.              |
  |                                                                           |
  |    L_var2                                                                 |
  |             real and imaginary part of L_va2 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var2) <= 0x0000 7fff.              |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             real and imaginary part of L_va_out are 16 bit short signed   |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var_out) <= 0x0000 7fff.           |
  |___________________________________________________________________________|
 */

CWord2x16 cmult_cr (CWord2x16 L_var1, CWord2x16 L_var2)
{
   CWord2x16 L_var_out;
   Word32 L_product_r, L_product_i, L_sum_r, L_sum_i;

   /* calculate real part      */
   L_product_r = L_mult(real(L_var1), real(L_var2));
   L_sum_r = L_mac(L_product_r, imag(L_var1), imag(L_var2));
   if (L_product_r == MAX_32)
   {
      L_sum_r = L_add(L_sum_r, 1);
   }

   /* calculate imaginary part */
   L_product_i = L_mult(imag(L_var1), real(L_var2));
   L_sum_i = L_msu(L_product_i, real(L_var1), imag(L_var2));
   if (L_product_i == MAX_32)
   {
      L_sum_i = L_add(L_sum_i, 1);
   }

   L_var_out = complex(rnd(L_sum_r), rnd(L_sum_i));

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_cmult                                                 |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Complex multiply L_var1 by L_var2 and then shift left by 1.             |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             real and imaginary part of L_va1 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var1) <= 0x0000 7fff.              |
  |                                                                           |
  |    L_var2                                                                 |
  |             real and imaginary part of L_va2 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var2) <= 0x0000 7fff.              |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             real and imaginary part of L_va_out are 32 bit long signed    |
  |             integer (Word32) whose value falls in the range:              |
  |             0x8000 0000 <= real/imag(LL_var_out) <= 0x7fff ffff.          |
  |___________________________________________________________________________|
 */

CWord2x32 L_cmult(CWord2x16 L_var1, CWord2x16 L_var2)
{
   CWord2x32 LL_var_out;
   Word32 L_product_r, L_product_i, L_sum_r, L_sum_i;

   /* calculate real part      */
   L_product_r = L_mult(real(L_var1), real(L_var2));
   L_sum_r = L_msu(L_product_r, imag(L_var1), imag(L_var2));
   if (L_product_r == MAX_32)
   {
      L_sum_r = L_add(L_sum_r, 1);
   }

   /* calculate imaginary part */
   L_product_i = L_mult(real(L_var1), imag(L_var2));
   L_sum_i = L_mac(L_product_i, imag(L_var1), real(L_var2));
   if (L_product_i == MAX_32)
   {
      L_sum_i = L_add(L_sum_i, 1);
   }

   LL_var_out = L_complex(L_sum_r, L_sum_i);

   return (LL_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_cmult_c                                               |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Complex multiply L_var1 by conjugate(L_var2) and then shift left by 1.  |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             real and imaginary part of L_va1 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var1) <= 0x0000 7fff.              |
  |                                                                           |
  |    L_var2                                                                 |
  |             real and imaginary part of L_va2 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var2) <= 0x0000 7fff.              |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             real and imaginary part of L_va_out are 32 bit long signed    |
  |             integer (Word32) whose value falls in the range:              |
  |             0x8000 0000 <= real/imag(LL_var_out) <= 0x7fff ffff.          |
  |___________________________________________________________________________|
 */

CWord2x32 L_cmult_c(CWord2x16 L_var1, CWord2x16 L_var2)
{
   CWord2x32 LL_var_out;
   Word32 L_product_r, L_product_i, L_sum_r, L_sum_i;

   /* calculate real part      */
   L_product_r = L_mult(real(L_var1), real(L_var2));
   L_sum_r = L_mac(L_product_r, imag(L_var1), imag(L_var2));
   if (L_product_r == MAX_32)
   {
      L_sum_r = L_add(L_sum_r, 1);
   }

   /* calculate imaginary part */
   L_product_i = L_mult(imag(L_var1), real(L_var2));
   L_sum_i = L_msu(L_product_i, real(L_var1), imag(L_var2));
   if (L_product_i == MAX_32)
   {
      L_sum_i = L_add(L_sum_i, 1);
   }

   LL_var_out = L_complex(L_sum_r, L_sum_i);

   return (LL_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_cmac                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply complex number L_var1 by L_var2 and shift the result left by 1.|
  |   Add the 32 bit real/imag result to LL_var3 with saturation, return      |
  |   a 64-bit complex number                                                 |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var3                                                                |
  |             real and imaginary part of LL_var3 are 32 bit long signed     |
  |             integer (Word32) whose value falls in the range:              |
  |             0x8000 0000 <= real/imag(LL_var3) <= 0x7fff ffff.             |
  |                                                                           |
  |    L_var1                                                                 |
  |             real and imaginary part of L_va1 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var1) <= 0x0000 7fff.              |
  |                                                                           |
  |    L_var2                                                                 |
  |             real and imaginary part of L_va2 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var2) <= 0x0000 7fff.              |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             real and imaginary part of L_va_out are 32 bit long signed    |
  |             integer (Word32) whose value falls in the range:              |
  |             0x8000 0000 <= real/imag(LL_var_out) <= 0x7fff ffff.          |
  |___________________________________________________________________________|
 */

CWord2x32 L_cmac(CWord2x32 LL_var3, CWord2x16 L_var1, CWord2x16 L_var2)
{
   CWord2x32 LL_var_out;
   Word32 L_var_out_r, L_var_out_i;
   Word64 LL_product_r, LL_product_i;

   /* calculate real part      */
   LL_product_r  = ((Word32)real(L_var1) *(Word32)real(L_var2));
   LL_product_r -= ((Word32)imag(L_var1) *(Word32)imag(L_var2));
   LL_product_r += LL_product_r;

   L_var_out_r  = L_sat((Word64)L_real(LL_var3) + LL_product_r);

   /* calculate imaginary part */
   LL_product_i  = ((Word32)real(L_var1) *(Word32)imag(L_var2));
   LL_product_i += ((Word32)imag(L_var1) *(Word32)real(L_var2));
   LL_product_i += LL_product_i;

   L_var_out_i  = L_sat((Word64)L_imag(LL_var3) + LL_product_i);

   /* construct long complex number */
   LL_var_out = L_complex(L_var_out_r, L_var_out_i);

   return (LL_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_cmac_c                                                |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply complex number L_var1 by conjugate(L_var2) and shift the result|
  |   left by 1. Add the 32 bit real/imag result to LL_var3 with saturation,  |
  |   return a 64-bit complex number                                          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var3                                                                |
  |             real and imaginary part of LL_var3 are 32 bit long signed     |
  |             integer (Word32) whose value falls in the range:              |
  |             0x8000 0000 <= real/imag(LL_var3) <= 0x7fff ffff.             |
  |                                                                           |
  |    L_var1                                                                 |
  |             real and imaginary part of L_va1 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var1) <= 0x0000 7fff.              |
  |                                                                           |
  |    L_var2                                                                 |
  |             real and imaginary part of L_va2 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var2) <= 0x0000 7fff.              |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             real and imaginary part of L_va_out are 32 bit long signed    |
  |             integer (Word32) whose value falls in the range:              |
  |             0x8000 0000 <= real/imag(LL_var_out) <= 0x7fff ffff.          |
  |___________________________________________________________________________|
 */

CWord2x32 L_cmac_c(CWord2x32 LL_var3, CWord2x16 L_var1, CWord2x16 L_var2)
{
   CWord2x32 LL_var_out;
   Word32 L_var_out_r, L_var_out_i;
   Word64 LL_product_r, LL_product_i;

   /* calculate real part      */
   LL_product_r  = ((Word32)real(L_var1) *(Word32)real(L_var2));
   LL_product_r += ((Word32)imag(L_var1) *(Word32)imag(L_var2));
   LL_product_r += LL_product_r;

   L_var_out_r  = L_sat((Word64)L_real(LL_var3) + LL_product_r);

   /* calculate imaginary part */
   LL_product_i  = ((Word32)imag(L_var1) *(Word32)real(L_var2));
   LL_product_i -= ((Word32)real(L_var1) *(Word32)imag(L_var2));
   LL_product_i += LL_product_i;

   L_var_out_i  = L_sat((Word64)L_imag(LL_var3) + LL_product_i);

   /* construct long complex number */
   LL_var_out = L_complex(L_var_out_r, L_var_out_i);

   return (LL_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_cmsu                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply complex number L_var1 by L_var2 and shift the result left by 1.|
  |   Sub the 32 bit real/imag result from LL_var3 with saturation, return    |
  |   a 64-bit complex number                                                 |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var3                                                                |
  |             real and imaginary part of LL_var3 are 32 bit long signed     |
  |             integer (Word32) whose value falls in the range:              |
  |             0x8000 0000 <= real/imag(LL_var3) <= 0x7fff ffff.             |
  |                                                                           |
  |    L_var1                                                                 |
  |             real and imaginary part of L_va1 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var1) <= 0x0000 7fff.              |
  |                                                                           |
  |    L_var2                                                                 |
  |             real and imaginary part of L_va2 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var2) <= 0x0000 7fff.              |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             real and imaginary part of L_va_out are 32 bit long signed    |
  |             integer (Word32) whose value falls in the range:              |
  |             0x8000 0000 <= real/imag(LL_var_out) <= 0x7fff ffff.          |
  |___________________________________________________________________________|
 */

CWord2x32 L_cmsu(CWord2x32 LL_var3, CWord2x16 L_var1, CWord2x16 L_var2)
{
   CWord2x32 LL_var_out;
   Word32 L_var_out_r, L_var_out_i;
   Word64 LL_product_r, LL_product_i;

   /* calculate real part      */
   LL_product_r  = ((Word32)real(L_var1) *(Word32)real(L_var2));
   LL_product_r -= ((Word32)imag(L_var1) *(Word32)imag(L_var2));
   LL_product_r += LL_product_r;

   L_var_out_r  = L_sat((Word64)L_real(LL_var3) - LL_product_r);

   /* calculate imaginary part */
   LL_product_i  = ((Word32)real(L_var1) *(Word32)imag(L_var2));
   LL_product_i += ((Word32)imag(L_var1) *(Word32)real(L_var2));
   LL_product_i += LL_product_i;

   L_var_out_i  = L_sat((Word64)L_imag(LL_var3) - LL_product_i);

   /* construct long complex number */
   LL_var_out = L_complex(L_var_out_r, L_var_out_i);

   return (LL_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_cmsu_c                                                |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Multiply complex number L_var1 by conjugate(L_var2) and shift the result|
  |   left by 1. Sub the 32 bit real/imag result from LL_var3 with saturation,|
  |   return a 64-bit complex number                                          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var3                                                                |
  |             real and imaginary part of LL_var3 are 32 bit long signed     |
  |             integer (Word32) whose value falls in the range:              |
  |             0x8000 0000 <= real/imag(LL_var3) <= 0x7fff ffff.             |
  |                                                                           |
  |    L_var1                                                                 |
  |             real and imaginary part of L_va1 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var1) <= 0x0000 7fff.              |
  |                                                                           |
  |    L_var2                                                                 |
  |             real and imaginary part of L_va2 are 16 bit short signed      |
  |             integer (Word16) whose value falls in the range:              |
  |             0xffff 8000 <= real/imag(L_var2) <= 0x0000 7fff.              |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             real and imaginary part of L_va_out are 32 bit long signed    |
  |             integer (Word32) whose value falls in the range:              |
  |             0x8000 0000 <= real/imag(LL_var_out) <= 0x7fff ffff.          |
  |___________________________________________________________________________|
 */

CWord2x32 L_cmsu_c(CWord2x32 LL_var3, CWord2x16 L_var1, CWord2x16 L_var2)
{
   CWord2x32 LL_var_out;
   Word32 L_var_out_r, L_var_out_i;
   Word64 LL_product_r, LL_product_i;

   /* calculate real part      */
   LL_product_r  = ((Word32)real(L_var1) *(Word32)real(L_var2));
   LL_product_r += ((Word32)imag(L_var1) *(Word32)imag(L_var2));
   LL_product_r += LL_product_r;

   L_var_out_r  = L_sat((Word64)L_real(LL_var3) - LL_product_r);

   /* calculate imaginary part */
   LL_product_i  = ((Word32)imag(L_var1) *(Word32)real(L_var2));
   LL_product_i -= ((Word32)real(L_var1) *(Word32)imag(L_var2));
   LL_product_i += LL_product_i;
   L_var_out_i  = L_sat((Word64)L_imag(LL_var3) - LL_product_i);

   /* construct long complex number */
   LL_var_out = L_complex(L_var_out_r, L_var_out_i);

   return (LL_var_out);
}




/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : clb                                                     |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Counting leading bits                                                   |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0x0000 0000 <= var_out <= 0x0000 0020.                |
  |___________________________________________________________________________|
 */

Word16 clb (Word32 L_var1)
{
   Word16 var_out;

   var_out = 0;

   if (L_var1 >= 0)
   {
      L_var1 = ~L_var1;
   }
   while (L_var1 < 0) 
   { 
      var_out++;
      L_var1 <<= 1;
   }

   return (var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : cl0                                                     |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Counting leading 0s                                                     |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long signed integer (Word32) whose value falls in the  |
  |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    var_out                                                                |
  |             16 bit short signed integer (Word16) whose value falls in the |
  |             range : 0x0000 0000 <= var_out <= 0x0000 0020.                |
  |___________________________________________________________________________|
 */

Word16 cl0 (Word32 L_var1)
{
   Word16 var_out;

   var_out = 0;

   L_var1 = ~L_var1;
   while (L_var1 < 0) 
   { 
      var_out++;
      L_var1 <<= 1;
   }

   return (var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : bitreverse                                              |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   reverse bits of L_var1                                                  |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32 bit long unsigned integer (UWord32) whose value falls in   |
  |             the range : 0x0000 0000 <= L_var1 <= 0xffff ffff.             |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long unsigned integer (UWord32) whose value falls in   |
  |             the range : 0x0000 0000 <= L_var_out <= 0xffff ffff.          |
  |___________________________________________________________________________|
 */

UWord32 bitreverse(UWord32 L_var1)
{
   Word32 L_var_out;
   int i;

   L_var_out = 0;

   for ( i= 0; i<32; i++) 
   {
      L_var_out = (L_var_out << 1)| (L_var1 & 1);
      L_var1 >>= 1;
   }

   return (L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : parity                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |     Perform parity check on (LL_var1 & LL_var2).                          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64 bit long long unsigned integer (UWord64)                   |
  |                                                                           |
  |    LL_var2                                                                |
  |             64 bit long long unsigned integer (UWord64)                   |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32 bit long unsigned integer (UWord32) whose value is 0 or 1  |
  |___________________________________________________________________________|
 */

UWord32 parity(UWord64 LL_var1, UWord64 LL_var2) 
{
   UWord32 L_var_out;
   
   LL_var1 &= LL_var2;
   L_var_out = ((UWord32)(LL_var1>>32)) ^ ((UWord32)LL_var1);

   L_var_out ^= (L_var_out>>16);
   L_var_out ^= (L_var_out>> 8);
   L_var_out ^= (L_var_out>> 4);
   L_var_out ^= (L_var_out>> 2);
   L_var_out ^= (L_var_out>> 1);
   L_var_out &= 1;

   return(L_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_Vadd                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Performs two 32-bit additions with saturation.                          |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    LL_var2                                                                |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |___________________________________________________________________________|
 */

Word64 L_Vadd(Word64 LL_var1, Word64 LL_var2)
{

   Word64 LL_var_out;
   Word32 L_var_out_h , L_var_out_l;

   L_var_out_h = L_add( L_extract_h32(LL_var1), L_extract_h32(LL_var2) );
   L_var_out_l = L_add( L_extract_l32(LL_var1), L_extract_l32(LL_var2) );

   LL_var_out = L_combine(L_var_out_h, L_var_out_l);

   return (LL_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_Vsub                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Performs two 32-bit subtractsions with saturation.                      |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    LL_var2                                                                |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |___________________________________________________________________________|
 */

Word64 L_Vsub(Word64 LL_var1, Word64 LL_var2)
{

   Word64 LL_var_out;
   Word32 L_var_out_h , L_var_out_l;

   L_var_out_h = L_sub( L_extract_h32(LL_var1), L_extract_h32(LL_var2) );
   L_var_out_l = L_sub( L_extract_l32(LL_var1), L_extract_l32(LL_var2) );

   LL_var_out = L_combine(L_var_out_h, L_var_out_l);

   return (LL_var_out);
}




/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_Vasl                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Performs two arithmetic shifts left without saturation                  |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var2                                                                   |
  |             16-bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= L_var1 <= 0x0000 7fff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |___________________________________________________________________________|
 */

Word64 L_Vasl(Word64 LL_var1, Word16 var2)
{

   Word64 LL_var_out;
   Word32 L_var_out_h , L_var_out_l;

   L_var_out_h = L_asl( L_extract_h32(LL_var1), var2 );
   L_var_out_l = L_asl( L_extract_l32(LL_var1), var2 );

   LL_var_out = L_combine(L_var_out_h, L_var_out_l);

   return (LL_var_out);
}


/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_Vasr                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Performs two arithmetic shifts right without saturation                 |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |                                                                           |
  |    var2                                                                   |
  |             16-bit short signed integer (Word16) whose value falls in the |
  |             range : 0xffff 8000 <= L_var1 <= 0x0000 7fff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
  |___________________________________________________________________________|
 */

Word64 L_Vasr(Word64 LL_var1, Word16 var2)
{

   Word64 LL_var_out;
   Word32 L_var_out_h , L_var_out_l;

   L_var_out_h = L_asr( L_extract_h32(LL_var1), var2 );
   L_var_out_l = L_asr( L_extract_l32(LL_var1), var2 );

   LL_var_out = L_combine(L_var_out_h, L_var_out_l);

   return (LL_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : L_Vsxt                                                  |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Sign-extend 16 MSB/LSB of a 32-bit long word into 2 32-bit long signed  |
  |   integer and construnct them into 64-bit long long word.                 |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    L_var1                                                                 |
  |             32-bit long word  whose MSB/LSB 16 bit represent two 16 bit   |
  |             short signed integers (Word16) with value falling in the      |
  |             range : 0xffff 8000 <= L_var1 <= 0x0000 7fff.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    LL_var_out                                                             |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0xffff 8000 <= L_var1 <= 0x0000 7fff.                 |
  |___________________________________________________________________________|
 */

Word64 L_Vsxt(Word32 L_var1)
{

   Word64 LL_var_out;
   Word32 L_var_out_h , L_var_out_l;

   L_var_out_h = (Word32)extract_h(L_var1);
   L_var_out_l = (Word32)extract_l(L_var1); 

   LL_var_out = L_combine(L_var_out_h, L_var_out_l);

   return (LL_var_out);
}



/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : Vrnd                                                    |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Round two 32-bit long signed integers into two 16-bit short signed      |
  |   signed integers.                                                        |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff 0000.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32-bit long word  whose MSB/LSB 16 bit represent two 16 bit   |
  |             short signed integers (Word16) with value falling in the      |
  |             range : 0xffff 8000 <= L_var1 <= 0x0000 7fff.                 |
  |___________________________________________________________________________|
 */

Word32 Vrnd(Word64 LL_var1)
{

   Word32 L_var_out;
   Word16 var_out_h , var_out_l;

   var_out_h = rnd(L_extract_h32(LL_var1));
   var_out_l = rnd(L_extract_l32(LL_var1));

   L_var_out = ((Word32)var_out_h<<16) | ((Word32)var_out_l & 0x0000FFFF);

   return (L_var_out);
}





/*____________________________________________________________________________
  |                                                                           |
  |   Function Name : Vsat                                                    |
  |                                                                           |
  |   Purpose :                                                               |
  |                                                                           |
  |   Saturate two 32-bit long signed integers into two 16-bit short signed   |
  |   signed integers.                                                        |
  |                                                                           |
  |   Inputs :                                                                |
  |                                                                           |
  |    LL_var1                                                                |
  |             64-bit long long word  whose MSB/LSB 32 bit represent two 32  |
  |             bit long signed integers (Word32) with value falling in the   |
  |             range : 0x8000 0000 <= L_var1 <= 0x7fff 0000.                 |
  |                                                                           |
  |   Return Value :                                                          |
  |                                                                           |
  |    L_var_out                                                              |
  |             32-bit long word  whose MSB/LSB 16 bit represent two 16 bit   |
  |             short signed integers (Word16) with value falling in the      |
  |             range : 0xffff 8000 <= L_var1 <= 0x0000 7fff.                 |
  |___________________________________________________________________________|
 */

Word32 Vsat(Word64 LL_var1)
{

   Word32 L_var_out;
   Word16 var_out_h , var_out_l;

   var_out_h = saturate(L_extract_h32(LL_var1));
   var_out_l = saturate(L_extract_l32(LL_var1));

   L_var_out = ((Word32)var_out_h<<16) | ((Word32)var_out_l & 0x0000FFFF);

   return (L_var_out);
}


//===================== END OF FILE =============================================
