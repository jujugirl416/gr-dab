/* -*- c++ -*- */
/*
 * Copyright 2004 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/*
 * config.h is generated by configure.  It contains the results
 * of probing for features, options etc.  It should be the first
 * file included in your .cc file.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "complex_to_interleaved_float_vcf_impl.h"

namespace gr {
  namespace dab {

complex_to_interleaved_float_vcf::sptr
complex_to_interleaved_float_vcf::make(unsigned int length)
{
  return gnuradio::get_initial_sptr
    (new complex_to_interleaved_float_vcf_impl(length));
}

complex_to_interleaved_float_vcf_impl::complex_to_interleaved_float_vcf_impl(unsigned int length)
  : gr::sync_block("complex_to_interleaved_float_vcf",
             gr::io_signature::make (1, 1, sizeof(gr_complex)*length),
             gr::io_signature::make (1, 1, sizeof(float)*length*2)),
  d_length(length)
{
}


int 
complex_to_interleaved_float_vcf_impl::work(int noutput_items,
                        gr_vector_const_void_star &input_items,
                        gr_vector_void_star &output_items)
{
  gr_complex const *in = (const gr_complex *) input_items[0];
  float *out = (float *) output_items[0];

  for (int i=0; i<noutput_items; i++) {
    for (unsigned int j=0;j<d_length;j++) {
      out[j] = in[j].real();
      out[j+d_length] = in[j].imag();
    }
    in += d_length;
    out+= 2*d_length;
  }
    
  return noutput_items;
}

}
}
