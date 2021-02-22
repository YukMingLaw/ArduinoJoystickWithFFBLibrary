/***
 * IIR Filter Library - Definitions and constants
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

#pragma once

// Uncomment to use `double` instead of `float` for all library functions.
//#define LIBFILTER_USE_DOUBLE

#ifndef LIBFILTER_USE_DOUBLE
  typedef float   float_t;
  typedef int     int_t;
#else
  typedef double  float_t;
  typedef int64_t int_t;
#endif 

namespace IIR {
  const uint8_t MAX_ORDER = 5;
  enum class TYPE   : uint8_t {LOWPASS = 0, HIGHPASS = 1};

  const float_t SQRT2 = sqrt(2.0);
  const float_t SQRT3 = sqrt(3.0);
  const float_t SQRT5 = sqrt(5.0);

  const float_t EPSILON   = 0.00001;    // Tolerance for numerical constants
  const float_t WEPSILON  = 0.00010;    // Warning threshold for numerical degradation
  const float_t KM        = 100.0;      // Pre-multiplier to reduce the impact of the AVRs limited float representation
}
