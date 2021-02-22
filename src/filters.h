/***
 * IIR Filter Library - Header
 *
 * Copyright (C) 2016  Martin Vincent Bloedorn
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \TODO Rename low-pass filter coefficients to a more standard a0..aN, b0..bN notation.
 *  \TODO Recompute low-pass formulations with Bilinear transforms. 
 *  \TODO Put all required coefficients in vector form (a[0] instead of a0), ...
 *  \TODO Rewrite all difference equations to be in a single form. (eliminate High/Low distinction), 
 *        and allow everything to be shoved nicely into a single for loop (for i=0; ... i<od)
 */

#pragma once

/// Has to be executed on Arduino IDE > 1.6.7
#include <Arduino.h>

#include "filters_defs.h"

using namespace IIR;

class Filter {
public:

  /** \brief Filter class.
   *
   */
  Filter(float_t hz_, float_t ts_, TYPE ty_ = TYPE::LOWPASS);
  ~Filter();

  float_t filterIn(float_t input);

  void flush();
  void init(bool doFlush=true);

private:
  float_t ts;
  float_t hz;
  TYPE  ty;

  // Helper variables during coefficient calcutions
  float_t a, b, c, d, e;
  // Filter coefficients
  float_t b0, b1, b2, b3, b4, a0, a1, a2;
  // Difference equation terms 
  float_t k0, k1, k2, k3, k4, k5;
  float_t j0, j1, j2;
  // Filter buffer 
  float_t y[MAX_ORDER], u[MAX_ORDER];

  bool f_err, f_warn; ///< Numerical error or warning; only relevant for 8-bit micros

  float_t ap(float_t p); ///< Assert Parameter

  inline float_t computeLowPass(float_t input);

  /** \brief Computes the discrete coefficients for a Butterworth low-pass filter via pole-zero matching. 
   *  Up to order 4. 
   */ 
  inline void  initLowPass();

  /** \brief Computes the discrete coefficients for a Butterworth high-pass filter via Bilinear-Transformation.
   *  Up to order 2 (I was lazy).
   */
};
