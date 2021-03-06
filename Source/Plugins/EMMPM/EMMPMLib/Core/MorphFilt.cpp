/* ============================================================================
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Written by Joel Dumke on 1/30/09 */

/* This function calculates the curvature cost values stored in curve based on
	the classification image, xt, morphologically opened under structuring
	element se */

/* Note: This is the function where I really got windowing right for the first time. */

#include <stddef.h>
#include <stdlib.h>


#include "MorphFilt.h"


#define NUM_SES 8

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MorphFilter::MorphFilter()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
MorphFilter::~MorphFilter()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
unsigned int MorphFilter::maxi(int a, int b) {
	if (a < b)
		return (unsigned int)b;
	return (unsigned int)a;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
unsigned int MorphFilter::mini(int a, int b) {
	if (a < b)
		return (unsigned int)a;
	return (unsigned int)b;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MorphFilter::morphFilt(EMMPM_Data* data, unsigned char* curve, unsigned char* se, int r)
{
  unsigned char* erosion;
  unsigned int  l, maxr, maxc;
  int ii, jj;
  int h, w;
  unsigned int se_cols;
  size_t ij, i1j1, iirjjr;

  int rows = data->rows;
  int cols = data->columns;
  int classes = data->classes;

  se_cols = 2*r + 1;

  erosion = (unsigned char*)malloc(cols * rows * sizeof(unsigned char));

  for (int32_t i = 0; i < rows; i++)
  {
    for (int32_t j = 0; j < cols; j++)
    {
      ij = (cols * i) + j;

      curve[ij] = classes;
      l = data->xt[ij];
      erosion[ij] = l;
      maxr = mini(r, rows - 1 - i);
      maxc = mini(r, cols - 1 - j);
      for (ii = -mini(r, i); ii <= (int)maxr; ii++)
      {
        for (jj = -mini(r, j); jj <= (int)maxc && erosion[ij] == l; jj++)
        {
          i1j1 = (cols * (i+ii)) + (j+jj);
          iirjjr = (se_cols * (ii+r)) + (jj+r);
          if (se[iirjjr] == 1 && data->xt[i1j1] != l)
          {
            erosion[ij] = classes;
          }
        }
      }
    }
  }

  h = r - -r + 1;
  w = r - -r + 1;
  for (ii = -r; ii <= r; ii++)
  {
    for (jj = -r; jj <= r; jj++)
    {
      iirjjr = (w * (ii+r)) + (jj+r);
      if (se[iirjjr] == 1)
      {
        maxr = rows - maxi(0, ii);
        maxc = cols - maxi(0, jj);
        for (unsigned int i = maxi(0, -ii); i < maxr; i++)
          for (unsigned int j = maxi(0, -jj); j < maxc; j++)
          {
            ij = (cols * i) + j;
            l = erosion[ij];
            if (l != (unsigned int)(classes))
			{
              i1j1 = (cols * (i+ii)) + (j+jj);
              curve[i1j1] = l;
            }
          }
      }
    }
  }

  free(erosion);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void MorphFilter::multiSE(EMMPM_Data* data)
{
  int  k, l;
  int  ri;
  unsigned char* se = NULL;
  unsigned char* curve = NULL;
  real_t r, r_sq, pnlty;
  size_t ij, lij, iirijjri;
  size_t se_cols;
  size_t se_rows;
 // int dims = data->dims;
  int rows = data->rows;
  int cols = data->columns;
  int classes = data->classes;

  pnlty = 1 / (real_t)NUM_SES;

  curve = (unsigned char*)malloc(cols * rows * sizeof(unsigned char));

  for (k = 0; k < NUM_SES; k++)
  {
    /* Calculate new r */
    r = data->r_max / (k + 1);

    r_sq = r * r;
    ri = (int)r;

    /* Create Morphological SE */
    se_cols = (2 * ri + 1);
    se_rows = (2 * ri + 1);
    se = (unsigned char*)malloc(se_cols * se_rows * sizeof(unsigned char));
    for (int ii = -((int)ri); ii <= (int)ri; ii++)
    {
      for (int jj = -((int)ri); jj <= (int)ri; jj++)
      {
        iirijjri = (se_cols * (ii+ri)) + (jj+ri);
        if (ii * ii + jj * jj <= r_sq)
        {
          se[iirijjri] = 1;
//          se[ii + ri][jj + ri] = 1;
        }
        else
        {
          se[iirijjri] = 0;
//          se[ii + ri][jj + ri] = 0;
        }
      }
    }
    morphFilt(data, curve, se, ri);

    for (int32_t i = 0; i < rows; i++)
    {
      for (int32_t j = 0; j < cols; j++)
      {
        ij = (cols * i) + j;
        l = curve[ij];
        if (l == classes)
        {
          l = data->xt[ij];
          lij = (cols * rows * l) + (cols * i) + j;
          data->ccost[lij] += pnlty;
        }
      }
    }

    free(se);
  }
  free(curve);
}
