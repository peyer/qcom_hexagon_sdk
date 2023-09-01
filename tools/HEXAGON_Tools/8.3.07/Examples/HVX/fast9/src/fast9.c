/* ======================================================================== */
/*  QUALCOMM TECHNOLOGIES, INC.                                             */
/*                                                                          */
/*  HEXAGON HVX Image/Video Processing Library                              */
/*                                                                          */
/* ------------------------------------------------------------------------ */
/*          Copyright (c) 2014 QUALCOMM TECHNOLOGIES Incorporated.          */
/*                           All Rights Reserved.                           */
/*                  QUALCOMM Confidential and Proprietary                   */
/* ======================================================================== */

/**=============================================================================
Copyright (c) 2006, 2008, 2009, 2010,2012 Edward Rosten
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

*Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

*Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

*Neither the name of the University of Cambridge nor the names of
its contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================**/

/*[========================================================================]*/
/*[ FUNCTION                                                               ]*/
/*[     fast9                                                              ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ DESCRIPTION                                                            ]*/
/*[     This function performs an FAST feature detection. It checks 16     ]*/
/*[ pixels in a circle around the candidate point. If 9 contiguous pixels  ]*/
/*[ are all brighter or darker than the nucleus by a threshold, the pixel  ]*/
/*[ under the nucleus is considered to be a feature.                       ]*/
/*[                                                                        ]*/
/*[------------------------------------------------------------------------]*/
/*[ REVISION DATE                                                          ]*/
/*[     DEC-01-2014                                                        ]*/
/*[                                                                        ]*/
/*[========================================================================]*/

/* ======================================================================== */
/*  Reference C version of fast9()                                          */
/* ======================================================================== */
static void make_offsets(int pixel[], int row_stride)
{
    pixel[ 0] =  0 + row_stride * 3;
    pixel[ 1] =  1 + row_stride * 3;
    pixel[ 2] =  2 + row_stride * 2;
    pixel[ 3] =  3 + row_stride * 1;
    pixel[ 4] =  3 + row_stride * 0;
    pixel[ 5] =  3 + row_stride * -1;
    pixel[ 6] =  2 + row_stride * -2;
    pixel[ 7] =  1 + row_stride * -3;
    pixel[ 8] =  0 + row_stride * -3;
    pixel[ 9] = -1 + row_stride * -3;
    pixel[10] = -2 + row_stride * -2;
    pixel[11] = -3 + row_stride * -1;
    pixel[12] = -3 + row_stride * 0;
    pixel[13] = -3 + row_stride * 1;
    pixel[14] = -2 + row_stride * 2;
    pixel[15] = -1 + row_stride * 3;
}

/* ======================================================================== */
void fast9(
    const unsigned char *im,
    unsigned int         stride,
    unsigned int         xsize,
    unsigned int         ysize,
    unsigned int         barrier,
    unsigned int         border,
    short int           *xy,
    int                  maxnumcorners,
    int                 *numcorners
    )
{
   *numcorners = 0;

   int boundary = border>3 ? border : 3;
   int pixel[16];
   int x, y;

   make_offsets(pixel, stride);

   for(y=boundary; y < ysize - boundary; y++)
   {
      for(x=boundary; x < xsize - boundary; x++)
      {
         const unsigned char* p = im + y*stride + x;

         int cb = *p + barrier;
         int c_b= *p - barrier;

         if(p[pixel[0]] > cb)
          if(p[pixel[1]] > cb)
           if(p[pixel[2]] > cb)
            if(p[pixel[3]] > cb)
             if(p[pixel[4]] > cb)
              if(p[pixel[5]] > cb)
               if(p[pixel[6]] > cb)
                if(p[pixel[7]] > cb)
                 if(p[pixel[8]] > cb)
                  {}
                 else
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   continue;
                else if(p[pixel[7]] < c_b)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   continue;
                 else if(p[pixel[14]] < c_b)
                  if(p[pixel[8]] < c_b)
                   if(p[pixel[9]] < c_b)
                    if(p[pixel[10]] < c_b)
                     if(p[pixel[11]] < c_b)
                      if(p[pixel[12]] < c_b)
                       if(p[pixel[13]] < c_b)
                        if(p[pixel[15]] < c_b)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   continue;
                 else
                  continue;
               else if(p[pixel[6]] < c_b)
                if(p[pixel[15]] > cb)
                 if(p[pixel[13]] > cb)
                  if(p[pixel[14]] > cb)
                   {}
                  else
                   continue;
                 else if(p[pixel[13]] < c_b)
                  if(p[pixel[7]] < c_b)
                   if(p[pixel[8]] < c_b)
                    if(p[pixel[9]] < c_b)
                     if(p[pixel[10]] < c_b)
                      if(p[pixel[11]] < c_b)
                       if(p[pixel[12]] < c_b)
                        if(p[pixel[14]] < c_b)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 if(p[pixel[7]] < c_b)
                  if(p[pixel[8]] < c_b)
                   if(p[pixel[9]] < c_b)
                    if(p[pixel[10]] < c_b)
                     if(p[pixel[11]] < c_b)
                      if(p[pixel[12]] < c_b)
                       if(p[pixel[13]] < c_b)
                        if(p[pixel[14]] < c_b)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[13]] > cb)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   continue;
                 else
                  continue;
                else if(p[pixel[13]] < c_b)
                 if(p[pixel[7]] < c_b)
                  if(p[pixel[8]] < c_b)
                   if(p[pixel[9]] < c_b)
                    if(p[pixel[10]] < c_b)
                     if(p[pixel[11]] < c_b)
                      if(p[pixel[12]] < c_b)
                       if(p[pixel[14]] < c_b)
                        if(p[pixel[15]] < c_b)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else if(p[pixel[5]] < c_b)
               if(p[pixel[14]] > cb)
                if(p[pixel[12]] > cb)
                 if(p[pixel[13]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       if(p[pixel[10]] > cb)
                        if(p[pixel[11]] > cb)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else if(p[pixel[12]] < c_b)
                 if(p[pixel[6]] < c_b)
                  if(p[pixel[7]] < c_b)
                   if(p[pixel[8]] < c_b)
                    if(p[pixel[9]] < c_b)
                     if(p[pixel[10]] < c_b)
                      if(p[pixel[11]] < c_b)
                       if(p[pixel[13]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else if(p[pixel[14]] < c_b)
                if(p[pixel[7]] < c_b)
                 if(p[pixel[8]] < c_b)
                  if(p[pixel[9]] < c_b)
                   if(p[pixel[10]] < c_b)
                    if(p[pixel[11]] < c_b)
                     if(p[pixel[12]] < c_b)
                      if(p[pixel[13]] < c_b)
                       if(p[pixel[6]] < c_b)
                        {}
                       else
                        if(p[pixel[15]] < c_b)
                         {}
                        else
                         continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                if(p[pixel[6]] < c_b)
                 if(p[pixel[7]] < c_b)
                  if(p[pixel[8]] < c_b)
                   if(p[pixel[9]] < c_b)
                    if(p[pixel[10]] < c_b)
                     if(p[pixel[11]] < c_b)
                      if(p[pixel[12]] < c_b)
                       if(p[pixel[13]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[pixel[12]] > cb)
                if(p[pixel[13]] > cb)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       if(p[pixel[10]] > cb)
                        if(p[pixel[11]] > cb)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else
                 continue;
               else if(p[pixel[12]] < c_b)
                if(p[pixel[7]] < c_b)
                 if(p[pixel[8]] < c_b)
                  if(p[pixel[9]] < c_b)
                   if(p[pixel[10]] < c_b)
                    if(p[pixel[11]] < c_b)
                     if(p[pixel[13]] < c_b)
                      if(p[pixel[14]] < c_b)
                       if(p[pixel[6]] < c_b)
                        {}
                       else
                        if(p[pixel[15]] < c_b)
                         {}
                        else
                         continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else if(p[pixel[4]] < c_b)
              if(p[pixel[13]] > cb)
               if(p[pixel[11]] > cb)
                if(p[pixel[12]] > cb)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       if(p[pixel[10]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       if(p[pixel[10]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else if(p[pixel[11]] < c_b)
                if(p[pixel[5]] < c_b)
                 if(p[pixel[6]] < c_b)
                  if(p[pixel[7]] < c_b)
                   if(p[pixel[8]] < c_b)
                    if(p[pixel[9]] < c_b)
                     if(p[pixel[10]] < c_b)
                      if(p[pixel[12]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else if(p[pixel[13]] < c_b)
               if(p[pixel[7]] < c_b)
                if(p[pixel[8]] < c_b)
                 if(p[pixel[9]] < c_b)
                  if(p[pixel[10]] < c_b)
                   if(p[pixel[11]] < c_b)
                    if(p[pixel[12]] < c_b)
                     if(p[pixel[6]] < c_b)
                      if(p[pixel[5]] < c_b)
                       {}
                      else
                       if(p[pixel[14]] < c_b)
                        {}
                       else
                        continue;
                     else
                      if(p[pixel[14]] < c_b)
                       if(p[pixel[15]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               if(p[pixel[5]] < c_b)
                if(p[pixel[6]] < c_b)
                 if(p[pixel[7]] < c_b)
                  if(p[pixel[8]] < c_b)
                   if(p[pixel[9]] < c_b)
                    if(p[pixel[10]] < c_b)
                     if(p[pixel[11]] < c_b)
                      if(p[pixel[12]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              if(p[pixel[11]] > cb)
               if(p[pixel[12]] > cb)
                if(p[pixel[13]] > cb)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       if(p[pixel[10]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       if(p[pixel[10]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else
                continue;
              else if(p[pixel[11]] < c_b)
               if(p[pixel[7]] < c_b)
                if(p[pixel[8]] < c_b)
                 if(p[pixel[9]] < c_b)
                  if(p[pixel[10]] < c_b)
                   if(p[pixel[12]] < c_b)
                    if(p[pixel[13]] < c_b)
                     if(p[pixel[6]] < c_b)
                      if(p[pixel[5]] < c_b)
                       {}
                      else
                       if(p[pixel[14]] < c_b)
                        {}
                       else
                        continue;
                     else
                      if(p[pixel[14]] < c_b)
                       if(p[pixel[15]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
            else if(p[pixel[3]] < c_b)
             if(p[pixel[10]] > cb)
              if(p[pixel[11]] > cb)
               if(p[pixel[12]] > cb)
                if(p[pixel[13]] > cb)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else if(p[pixel[10]] < c_b)
              if(p[pixel[7]] < c_b)
               if(p[pixel[8]] < c_b)
                if(p[pixel[9]] < c_b)
                 if(p[pixel[11]] < c_b)
                  if(p[pixel[6]] < c_b)
                   if(p[pixel[5]] < c_b)
                    if(p[pixel[4]] < c_b)
                     {}
                    else
                     if(p[pixel[12]] < c_b)
                      if(p[pixel[13]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                   else
                    if(p[pixel[12]] < c_b)
                     if(p[pixel[13]] < c_b)
                      if(p[pixel[14]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                  else
                   if(p[pixel[12]] < c_b)
                    if(p[pixel[13]] < c_b)
                     if(p[pixel[14]] < c_b)
                      if(p[pixel[15]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             if(p[pixel[10]] > cb)
              if(p[pixel[11]] > cb)
               if(p[pixel[12]] > cb)
                if(p[pixel[13]] > cb)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      if(p[pixel[9]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else if(p[pixel[10]] < c_b)
              if(p[pixel[7]] < c_b)
               if(p[pixel[8]] < c_b)
                if(p[pixel[9]] < c_b)
                 if(p[pixel[11]] < c_b)
                  if(p[pixel[12]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[5]] < c_b)
                     if(p[pixel[4]] < c_b)
                      {}
                     else
                      if(p[pixel[13]] < c_b)
                       {}
                      else
                       continue;
                    else
                     if(p[pixel[13]] < c_b)
                      if(p[pixel[14]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                   else
                    if(p[pixel[13]] < c_b)
                     if(p[pixel[14]] < c_b)
                      if(p[pixel[15]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
           else if(p[pixel[2]] < c_b)
            if(p[pixel[9]] > cb)
             if(p[pixel[10]] > cb)
              if(p[pixel[11]] > cb)
               if(p[pixel[12]] > cb)
                if(p[pixel[13]] > cb)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[3]] > cb)
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else if(p[pixel[9]] < c_b)
             if(p[pixel[7]] < c_b)
              if(p[pixel[8]] < c_b)
               if(p[pixel[10]] < c_b)
                if(p[pixel[6]] < c_b)
                 if(p[pixel[5]] < c_b)
                  if(p[pixel[4]] < c_b)
                   if(p[pixel[3]] < c_b)
                    {}
                   else
                    if(p[pixel[11]] < c_b)
                     if(p[pixel[12]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                  else
                   if(p[pixel[11]] < c_b)
                    if(p[pixel[12]] < c_b)
                     if(p[pixel[13]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[11]] < c_b)
                   if(p[pixel[12]] < c_b)
                    if(p[pixel[13]] < c_b)
                     if(p[pixel[14]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[11]] < c_b)
                  if(p[pixel[12]] < c_b)
                   if(p[pixel[13]] < c_b)
                    if(p[pixel[14]] < c_b)
                     if(p[pixel[15]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            if(p[pixel[9]] > cb)
             if(p[pixel[10]] > cb)
              if(p[pixel[11]] > cb)
               if(p[pixel[12]] > cb)
                if(p[pixel[13]] > cb)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[3]] > cb)
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     if(p[pixel[8]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else if(p[pixel[9]] < c_b)
             if(p[pixel[7]] < c_b)
              if(p[pixel[8]] < c_b)
               if(p[pixel[10]] < c_b)
                if(p[pixel[11]] < c_b)
                 if(p[pixel[6]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[4]] < c_b)
                    if(p[pixel[3]] < c_b)
                     {}
                    else
                     if(p[pixel[12]] < c_b)
                      {}
                     else
                      continue;
                   else
                    if(p[pixel[12]] < c_b)
                     if(p[pixel[13]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                  else
                   if(p[pixel[12]] < c_b)
                    if(p[pixel[13]] < c_b)
                     if(p[pixel[14]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[12]] < c_b)
                   if(p[pixel[13]] < c_b)
                    if(p[pixel[14]] < c_b)
                     if(p[pixel[15]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else
             continue;
          else if(p[pixel[1]] < c_b)
           if(p[pixel[8]] > cb)
            if(p[pixel[9]] > cb)
             if(p[pixel[10]] > cb)
              if(p[pixel[11]] > cb)
               if(p[pixel[12]] > cb)
                if(p[pixel[13]] > cb)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[3]] > cb)
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[pixel[2]] > cb)
                if(p[pixel[3]] > cb)
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              continue;
            else
             continue;
           else if(p[pixel[8]] < c_b)
            if(p[pixel[7]] < c_b)
             if(p[pixel[9]] < c_b)
              if(p[pixel[6]] < c_b)
               if(p[pixel[5]] < c_b)
                if(p[pixel[4]] < c_b)
                 if(p[pixel[3]] < c_b)
                  if(p[pixel[2]] < c_b)
                   {}
                  else
                   if(p[pixel[10]] < c_b)
                    if(p[pixel[11]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[10]] < c_b)
                   if(p[pixel[11]] < c_b)
                    if(p[pixel[12]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[10]] < c_b)
                  if(p[pixel[11]] < c_b)
                   if(p[pixel[12]] < c_b)
                    if(p[pixel[13]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[10]] < c_b)
                 if(p[pixel[11]] < c_b)
                  if(p[pixel[12]] < c_b)
                   if(p[pixel[13]] < c_b)
                    if(p[pixel[14]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[pixel[10]] < c_b)
                if(p[pixel[11]] < c_b)
                 if(p[pixel[12]] < c_b)
                  if(p[pixel[13]] < c_b)
                   if(p[pixel[14]] < c_b)
                    if(p[pixel[15]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else
           if(p[pixel[8]] > cb)
            if(p[pixel[9]] > cb)
             if(p[pixel[10]] > cb)
              if(p[pixel[11]] > cb)
               if(p[pixel[12]] > cb)
                if(p[pixel[13]] > cb)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[15]] > cb)
                   {}
                  else
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[3]] > cb)
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[pixel[2]] > cb)
                if(p[pixel[3]] > cb)
                 if(p[pixel[4]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[7]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              continue;
            else
             continue;
           else if(p[pixel[8]] < c_b)
            if(p[pixel[7]] < c_b)
             if(p[pixel[9]] < c_b)
              if(p[pixel[10]] < c_b)
               if(p[pixel[6]] < c_b)
                if(p[pixel[5]] < c_b)
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[3]] < c_b)
                   if(p[pixel[2]] < c_b)
                    {}
                   else
                    if(p[pixel[11]] < c_b)
                     {}
                    else
                     continue;
                  else
                   if(p[pixel[11]] < c_b)
                    if(p[pixel[12]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[11]] < c_b)
                   if(p[pixel[12]] < c_b)
                    if(p[pixel[13]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[11]] < c_b)
                  if(p[pixel[12]] < c_b)
                   if(p[pixel[13]] < c_b)
                    if(p[pixel[14]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[11]] < c_b)
                 if(p[pixel[12]] < c_b)
                  if(p[pixel[13]] < c_b)
                   if(p[pixel[14]] < c_b)
                    if(p[pixel[15]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else
            continue;
         else if(p[pixel[0]] < c_b)
          if(p[pixel[1]] > cb)
           if(p[pixel[8]] > cb)
            if(p[pixel[7]] > cb)
             if(p[pixel[9]] > cb)
              if(p[pixel[6]] > cb)
               if(p[pixel[5]] > cb)
                if(p[pixel[4]] > cb)
                 if(p[pixel[3]] > cb)
                  if(p[pixel[2]] > cb)
                   {}
                  else
                   if(p[pixel[10]] > cb)
                    if(p[pixel[11]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[10]] > cb)
                   if(p[pixel[11]] > cb)
                    if(p[pixel[12]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[10]] > cb)
                  if(p[pixel[11]] > cb)
                   if(p[pixel[12]] > cb)
                    if(p[pixel[13]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[10]] > cb)
                 if(p[pixel[11]] > cb)
                  if(p[pixel[12]] > cb)
                   if(p[pixel[13]] > cb)
                    if(p[pixel[14]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[pixel[10]] > cb)
                if(p[pixel[11]] > cb)
                 if(p[pixel[12]] > cb)
                  if(p[pixel[13]] > cb)
                   if(p[pixel[14]] > cb)
                    if(p[pixel[15]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              continue;
            else
             continue;
           else if(p[pixel[8]] < c_b)
            if(p[pixel[9]] < c_b)
             if(p[pixel[10]] < c_b)
              if(p[pixel[11]] < c_b)
               if(p[pixel[12]] < c_b)
                if(p[pixel[13]] < c_b)
                 if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[3]] < c_b)
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[pixel[2]] < c_b)
                if(p[pixel[3]] < c_b)
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              continue;
            else
             continue;
           else
            continue;
          else if(p[pixel[1]] < c_b)
           if(p[pixel[2]] > cb)
            if(p[pixel[9]] > cb)
             if(p[pixel[7]] > cb)
              if(p[pixel[8]] > cb)
               if(p[pixel[10]] > cb)
                if(p[pixel[6]] > cb)
                 if(p[pixel[5]] > cb)
                  if(p[pixel[4]] > cb)
                   if(p[pixel[3]] > cb)
                    {}
                   else
                    if(p[pixel[11]] > cb)
                     if(p[pixel[12]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                  else
                   if(p[pixel[11]] > cb)
                    if(p[pixel[12]] > cb)
                     if(p[pixel[13]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[11]] > cb)
                   if(p[pixel[12]] > cb)
                    if(p[pixel[13]] > cb)
                     if(p[pixel[14]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[11]] > cb)
                  if(p[pixel[12]] > cb)
                   if(p[pixel[13]] > cb)
                    if(p[pixel[14]] > cb)
                     if(p[pixel[15]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else if(p[pixel[9]] < c_b)
             if(p[pixel[10]] < c_b)
              if(p[pixel[11]] < c_b)
               if(p[pixel[12]] < c_b)
                if(p[pixel[13]] < c_b)
                 if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[3]] < c_b)
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else if(p[pixel[2]] < c_b)
            if(p[pixel[3]] > cb)
             if(p[pixel[10]] > cb)
              if(p[pixel[7]] > cb)
               if(p[pixel[8]] > cb)
                if(p[pixel[9]] > cb)
                 if(p[pixel[11]] > cb)
                  if(p[pixel[6]] > cb)
                   if(p[pixel[5]] > cb)
                    if(p[pixel[4]] > cb)
                     {}
                    else
                     if(p[pixel[12]] > cb)
                      if(p[pixel[13]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                   else
                    if(p[pixel[12]] > cb)
                     if(p[pixel[13]] > cb)
                      if(p[pixel[14]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                  else
                   if(p[pixel[12]] > cb)
                    if(p[pixel[13]] > cb)
                     if(p[pixel[14]] > cb)
                      if(p[pixel[15]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else if(p[pixel[10]] < c_b)
              if(p[pixel[11]] < c_b)
               if(p[pixel[12]] < c_b)
                if(p[pixel[13]] < c_b)
                 if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else if(p[pixel[3]] < c_b)
             if(p[pixel[4]] > cb)
              if(p[pixel[13]] > cb)
               if(p[pixel[7]] > cb)
                if(p[pixel[8]] > cb)
                 if(p[pixel[9]] > cb)
                  if(p[pixel[10]] > cb)
                   if(p[pixel[11]] > cb)
                    if(p[pixel[12]] > cb)
                     if(p[pixel[6]] > cb)
                      if(p[pixel[5]] > cb)
                       {}
                      else
                       if(p[pixel[14]] > cb)
                        {}
                       else
                        continue;
                     else
                      if(p[pixel[14]] > cb)
                       if(p[pixel[15]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else if(p[pixel[13]] < c_b)
               if(p[pixel[11]] > cb)
                if(p[pixel[5]] > cb)
                 if(p[pixel[6]] > cb)
                  if(p[pixel[7]] > cb)
                   if(p[pixel[8]] > cb)
                    if(p[pixel[9]] > cb)
                     if(p[pixel[10]] > cb)
                      if(p[pixel[12]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else if(p[pixel[11]] < c_b)
                if(p[pixel[12]] < c_b)
                 if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       if(p[pixel[10]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       if(p[pixel[10]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else
                continue;
              else
               if(p[pixel[5]] > cb)
                if(p[pixel[6]] > cb)
                 if(p[pixel[7]] > cb)
                  if(p[pixel[8]] > cb)
                   if(p[pixel[9]] > cb)
                    if(p[pixel[10]] > cb)
                     if(p[pixel[11]] > cb)
                      if(p[pixel[12]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else if(p[pixel[4]] < c_b)
              if(p[pixel[5]] > cb)
               if(p[pixel[14]] > cb)
                if(p[pixel[7]] > cb)
                 if(p[pixel[8]] > cb)
                  if(p[pixel[9]] > cb)
                   if(p[pixel[10]] > cb)
                    if(p[pixel[11]] > cb)
                     if(p[pixel[12]] > cb)
                      if(p[pixel[13]] > cb)
                       if(p[pixel[6]] > cb)
                        {}
                       else
                        if(p[pixel[15]] > cb)
                         {}
                        else
                         continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else if(p[pixel[14]] < c_b)
                if(p[pixel[12]] > cb)
                 if(p[pixel[6]] > cb)
                  if(p[pixel[7]] > cb)
                   if(p[pixel[8]] > cb)
                    if(p[pixel[9]] > cb)
                     if(p[pixel[10]] > cb)
                      if(p[pixel[11]] > cb)
                       if(p[pixel[13]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else if(p[pixel[12]] < c_b)
                 if(p[pixel[13]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       if(p[pixel[10]] < c_b)
                        if(p[pixel[11]] < c_b)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else
                 continue;
               else
                if(p[pixel[6]] > cb)
                 if(p[pixel[7]] > cb)
                  if(p[pixel[8]] > cb)
                   if(p[pixel[9]] > cb)
                    if(p[pixel[10]] > cb)
                     if(p[pixel[11]] > cb)
                      if(p[pixel[12]] > cb)
                       if(p[pixel[13]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else if(p[pixel[5]] < c_b)
               if(p[pixel[6]] > cb)
                if(p[pixel[15]] < c_b)
                 if(p[pixel[13]] > cb)
                  if(p[pixel[7]] > cb)
                   if(p[pixel[8]] > cb)
                    if(p[pixel[9]] > cb)
                     if(p[pixel[10]] > cb)
                      if(p[pixel[11]] > cb)
                       if(p[pixel[12]] > cb)
                        if(p[pixel[14]] > cb)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else if(p[pixel[13]] < c_b)
                  if(p[pixel[14]] < c_b)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 if(p[pixel[7]] > cb)
                  if(p[pixel[8]] > cb)
                   if(p[pixel[9]] > cb)
                    if(p[pixel[10]] > cb)
                     if(p[pixel[11]] > cb)
                      if(p[pixel[12]] > cb)
                       if(p[pixel[13]] > cb)
                        if(p[pixel[14]] > cb)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else if(p[pixel[6]] < c_b)
                if(p[pixel[7]] > cb)
                 if(p[pixel[14]] > cb)
                  if(p[pixel[8]] > cb)
                   if(p[pixel[9]] > cb)
                    if(p[pixel[10]] > cb)
                     if(p[pixel[11]] > cb)
                      if(p[pixel[12]] > cb)
                       if(p[pixel[13]] > cb)
                        if(p[pixel[15]] > cb)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   continue;
                 else
                  continue;
                else if(p[pixel[7]] < c_b)
                 if(p[pixel[8]] < c_b)
                  {}
                 else
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   continue;
                else
                 if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[13]] > cb)
                 if(p[pixel[7]] > cb)
                  if(p[pixel[8]] > cb)
                   if(p[pixel[9]] > cb)
                    if(p[pixel[10]] > cb)
                     if(p[pixel[11]] > cb)
                      if(p[pixel[12]] > cb)
                       if(p[pixel[14]] > cb)
                        if(p[pixel[15]] > cb)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else if(p[pixel[13]] < c_b)
                 if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[pixel[12]] > cb)
                if(p[pixel[7]] > cb)
                 if(p[pixel[8]] > cb)
                  if(p[pixel[9]] > cb)
                   if(p[pixel[10]] > cb)
                    if(p[pixel[11]] > cb)
                     if(p[pixel[13]] > cb)
                      if(p[pixel[14]] > cb)
                       if(p[pixel[6]] > cb)
                        {}
                       else
                        if(p[pixel[15]] > cb)
                         {}
                        else
                         continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else if(p[pixel[12]] < c_b)
                if(p[pixel[13]] < c_b)
                 if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       if(p[pixel[10]] < c_b)
                        if(p[pixel[11]] < c_b)
                         {}
                        else
                         continue;
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              if(p[pixel[11]] > cb)
               if(p[pixel[7]] > cb)
                if(p[pixel[8]] > cb)
                 if(p[pixel[9]] > cb)
                  if(p[pixel[10]] > cb)
                   if(p[pixel[12]] > cb)
                    if(p[pixel[13]] > cb)
                     if(p[pixel[6]] > cb)
                      if(p[pixel[5]] > cb)
                       {}
                      else
                       if(p[pixel[14]] > cb)
                        {}
                       else
                        continue;
                     else
                      if(p[pixel[14]] > cb)
                       if(p[pixel[15]] > cb)
                        {}
                       else
                        continue;
                      else
                       continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else if(p[pixel[11]] < c_b)
               if(p[pixel[12]] < c_b)
                if(p[pixel[13]] < c_b)
                 if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       if(p[pixel[10]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       if(p[pixel[10]] < c_b)
                        {}
                       else
                        continue;
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else
                continue;
              else
               continue;
            else
             if(p[pixel[10]] > cb)
              if(p[pixel[7]] > cb)
               if(p[pixel[8]] > cb)
                if(p[pixel[9]] > cb)
                 if(p[pixel[11]] > cb)
                  if(p[pixel[12]] > cb)
                   if(p[pixel[6]] > cb)
                    if(p[pixel[5]] > cb)
                     if(p[pixel[4]] > cb)
                      {}
                     else
                      if(p[pixel[13]] > cb)
                       {}
                      else
                       continue;
                    else
                     if(p[pixel[13]] > cb)
                      if(p[pixel[14]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                   else
                    if(p[pixel[13]] > cb)
                     if(p[pixel[14]] > cb)
                      if(p[pixel[15]] > cb)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else if(p[pixel[10]] < c_b)
              if(p[pixel[11]] < c_b)
               if(p[pixel[12]] < c_b)
                if(p[pixel[13]] < c_b)
                 if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      if(p[pixel[9]] < c_b)
                       {}
                      else
                       continue;
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                continue;
              else
               continue;
             else
              continue;
           else
            if(p[pixel[9]] > cb)
             if(p[pixel[7]] > cb)
              if(p[pixel[8]] > cb)
               if(p[pixel[10]] > cb)
                if(p[pixel[11]] > cb)
                 if(p[pixel[6]] > cb)
                  if(p[pixel[5]] > cb)
                   if(p[pixel[4]] > cb)
                    if(p[pixel[3]] > cb)
                     {}
                    else
                     if(p[pixel[12]] > cb)
                      {}
                     else
                      continue;
                   else
                    if(p[pixel[12]] > cb)
                     if(p[pixel[13]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                  else
                   if(p[pixel[12]] > cb)
                    if(p[pixel[13]] > cb)
                     if(p[pixel[14]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[12]] > cb)
                   if(p[pixel[13]] > cb)
                    if(p[pixel[14]] > cb)
                     if(p[pixel[15]] > cb)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 continue;
               else
                continue;
              else
               continue;
             else
              continue;
            else if(p[pixel[9]] < c_b)
             if(p[pixel[10]] < c_b)
              if(p[pixel[11]] < c_b)
               if(p[pixel[12]] < c_b)
                if(p[pixel[13]] < c_b)
                 if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[3]] < c_b)
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     if(p[pixel[8]] < c_b)
                      {}
                     else
                      continue;
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else
             continue;
          else
           if(p[pixel[8]] > cb)
            if(p[pixel[7]] > cb)
             if(p[pixel[9]] > cb)
              if(p[pixel[10]] > cb)
               if(p[pixel[6]] > cb)
                if(p[pixel[5]] > cb)
                 if(p[pixel[4]] > cb)
                  if(p[pixel[3]] > cb)
                   if(p[pixel[2]] > cb)
                    {}
                   else
                    if(p[pixel[11]] > cb)
                     {}
                    else
                     continue;
                  else
                   if(p[pixel[11]] > cb)
                    if(p[pixel[12]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[11]] > cb)
                   if(p[pixel[12]] > cb)
                    if(p[pixel[13]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[11]] > cb)
                  if(p[pixel[12]] > cb)
                   if(p[pixel[13]] > cb)
                    if(p[pixel[14]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[11]] > cb)
                 if(p[pixel[12]] > cb)
                  if(p[pixel[13]] > cb)
                   if(p[pixel[14]] > cb)
                    if(p[pixel[15]] > cb)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               continue;
             else
              continue;
            else
             continue;
           else if(p[pixel[8]] < c_b)
            if(p[pixel[9]] < c_b)
             if(p[pixel[10]] < c_b)
              if(p[pixel[11]] < c_b)
               if(p[pixel[12]] < c_b)
                if(p[pixel[13]] < c_b)
                 if(p[pixel[14]] < c_b)
                  if(p[pixel[15]] < c_b)
                   {}
                  else
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                 else
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[3]] < c_b)
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[pixel[2]] < c_b)
                if(p[pixel[3]] < c_b)
                 if(p[pixel[4]] < c_b)
                  if(p[pixel[5]] < c_b)
                   if(p[pixel[6]] < c_b)
                    if(p[pixel[7]] < c_b)
                     {}
                    else
                     continue;
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              continue;
            else
             continue;
           else
            continue;
         else
          if(p[pixel[7]] > cb)
           if(p[pixel[8]] > cb)
            if(p[pixel[9]] > cb)
             if(p[pixel[6]] > cb)
              if(p[pixel[5]] > cb)
               if(p[pixel[4]] > cb)
                if(p[pixel[3]] > cb)
                 if(p[pixel[2]] > cb)
                  if(p[pixel[1]] > cb)
                   {}
                  else
                   if(p[pixel[10]] > cb)
                    {}
                   else
                    continue;
                 else
                  if(p[pixel[10]] > cb)
                   if(p[pixel[11]] > cb)
                    {}
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[10]] > cb)
                  if(p[pixel[11]] > cb)
                   if(p[pixel[12]] > cb)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[10]] > cb)
                 if(p[pixel[11]] > cb)
                  if(p[pixel[12]] > cb)
                   if(p[pixel[13]] > cb)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[pixel[10]] > cb)
                if(p[pixel[11]] > cb)
                 if(p[pixel[12]] > cb)
                  if(p[pixel[13]] > cb)
                   if(p[pixel[14]] > cb)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              if(p[pixel[10]] > cb)
               if(p[pixel[11]] > cb)
                if(p[pixel[12]] > cb)
                 if(p[pixel[13]] > cb)
                  if(p[pixel[14]] > cb)
                   if(p[pixel[15]] > cb)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
            else
             continue;
           else
            continue;
          else if(p[pixel[7]] < c_b)
           if(p[pixel[8]] < c_b)
            if(p[pixel[9]] < c_b)
             if(p[pixel[6]] < c_b)
              if(p[pixel[5]] < c_b)
               if(p[pixel[4]] < c_b)
                if(p[pixel[3]] < c_b)
                 if(p[pixel[2]] < c_b)
                  if(p[pixel[1]] < c_b)
                   {}
                  else
                   if(p[pixel[10]] < c_b)
                    {}
                   else
                    continue;
                 else
                  if(p[pixel[10]] < c_b)
                   if(p[pixel[11]] < c_b)
                    {}
                   else
                    continue;
                  else
                   continue;
                else
                 if(p[pixel[10]] < c_b)
                  if(p[pixel[11]] < c_b)
                   if(p[pixel[12]] < c_b)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
               else
                if(p[pixel[10]] < c_b)
                 if(p[pixel[11]] < c_b)
                  if(p[pixel[12]] < c_b)
                   if(p[pixel[13]] < c_b)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
              else
               if(p[pixel[10]] < c_b)
                if(p[pixel[11]] < c_b)
                 if(p[pixel[12]] < c_b)
                  if(p[pixel[13]] < c_b)
                   if(p[pixel[14]] < c_b)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
             else
              if(p[pixel[10]] < c_b)
               if(p[pixel[11]] < c_b)
                if(p[pixel[12]] < c_b)
                 if(p[pixel[13]] < c_b)
                  if(p[pixel[14]] < c_b)
                   if(p[pixel[15]] < c_b)
                    {}
                   else
                    continue;
                  else
                   continue;
                 else
                  continue;
                else
                 continue;
               else
                continue;
              else
               continue;
            else
             continue;
           else
            continue;
          else
           continue;

         *(xy++) = x;
         *(xy++) = y;
         ++(*numcorners);

         if ( *numcorners >= maxnumcorners )
         {
            return;
         }
      }
   }
}

