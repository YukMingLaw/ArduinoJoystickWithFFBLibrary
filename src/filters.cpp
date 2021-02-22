/***
 * IIR Filter Library - Implementation
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

#include "filters.h"

using namespace IIR;

// CONSTRUCTOR AND DESTRUCTOR * * * * * * * * * * * * * * *

Filter::Filter(float_t hz_, float_t ts_, TYPE ty_) :
  ts( ts_ ),
  hz( hz_ ),
  ty( ty_ )
{
  init();
}

Filter::~Filter() { }

// PUBLIC METHODS * * * * * * * * * * * * * * * * * * * * *

void Filter::init(bool doFlush) {
  if(doFlush) flush();
  f_err  = false;
  f_warn = false;

  initLowPass();
}

float_t Filter::filterIn(float input) {
  if(f_err) return 0.0;
  return computeLowPass(input);

}

void Filter::flush() {
  for(uint8_t i=0; i<MAX_ORDER; i++) {
    u[i] = 0.0;
    y[i] = 0.0;
  }
}


// PRIVATE METHODS  * * * * * * * * * * * * * * * * * * * *

inline float_t Filter::computeLowPass(float_t input) {
  for(uint8_t i=MAX_ORDER-1; i>0; i--) {
    y[i] = y[i-1];
    u[i] = u[i-1];
  }

  y[0] = k1*y[1] + k0*input;
  return y[0];
}

inline void  Filter::initLowPass() {
    a  = 2.0*PI*hz;
    k1 = exp(-a*ts);
    k0 = 1.0 - k1;
}


float_t Filter::ap(float_t p) {
  f_err  = f_err  | (abs(p) <= EPSILON );
  f_warn = f_warn | (abs(p) <= WEPSILON);
  return (f_err) ? 0.0 : p;
}

