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

#include <stdio.h>

#include <gnuradio/io_signature.h>
#include "ofdm_coarse_frequency_correct_impl.h"
#include <gnuradio/expj.h>

#define M_TWOPI (2*M_PI)

namespace gr {
  namespace dab {

ofdm_coarse_frequency_correct::sptr
ofdm_coarse_frequency_correct::make(unsigned int fft_length, unsigned int num_carriers, unsigned int cp_length)
{
  return gnuradio::get_initial_sptr
    (new ofdm_coarse_frequency_correct_impl(fft_length, num_carriers, cp_length));
}

ofdm_coarse_frequency_correct_impl::ofdm_coarse_frequency_correct_impl(unsigned int fft_length, unsigned int num_carriers, unsigned int cp_length)
  : gr::sync_block("ofdm_coarse_frequency_correct",
             gr::io_signature::make2 (2, 2, sizeof(gr_complex)*fft_length, sizeof(char)),
             gr::io_signature::make2 (2, 2, sizeof(gr_complex)*num_carriers, sizeof(char))),
  d_fft_length(fft_length), d_num_carriers(num_carriers), d_cp_length(cp_length), 
  d_symbol_num(0), d_freq_offset(0), d_delta_f(0)
{
  d_zeros_on_left = (d_fft_length-d_num_carriers)/2;
}

float
ofdm_coarse_frequency_correct_impl::mag_squared(const gr_complex sample) {
    const float __x = sample.real();
    const float __y = sample.imag();
    return __x * __x + __y * __y;
}

void
ofdm_coarse_frequency_correct_impl::correlate_energy(const gr_complex *symbol)
{
  unsigned int i, index;
 
  double sum=0, max=0;

  /* energy based correlation - note that DAB uses a zero central carrier -
   * we always sum up the energy for all carriers except DC; this is however
   * only done for the first offset; for the others, the diff is calculated
   */

  /* offset 0 */
  for (i=0; i<d_num_carriers+1; i++) {
    if (i != d_num_carriers/2)
      sum+=(double)mag_squared(symbol[i]);
  }
  max = sum;
  index = 0;
  /* other offsets */
  for (i=1; i<d_fft_length-d_num_carriers; i++) {
    /* diff on left side */
    sum -= (double)mag_squared(symbol[i-1]);
    /* diff for zero carrier */
    sum += (double)mag_squared(symbol[i+d_num_carriers/2-1]);
    sum -= (double)mag_squared(symbol[i+d_num_carriers/2]);
    /* diff on rigth side */
    sum += (double)mag_squared(symbol[i+d_num_carriers]);
    /* new max found? */
    if (sum > max) {
      max = sum;
      index = i;
    }
  }

  d_freq_offset = index;
}

int 
ofdm_coarse_frequency_correct_impl::work(int noutput_items,
                        gr_vector_const_void_star &input_items,
                        gr_vector_void_star &output_items)
{
  unsigned int i;
  gr_complex phase_offset_correct;

  /* partially adapted from gr_ofdm_frame_acquisition.cc 
     however: 
      - energy based offset frequency estimation instead of using the pilot symbol
      - correlation in linear time instead of sqare
      - only magnitude equalisation (phase equalisation is unnecessery, because of the diff_phasor later in the chain)
      - calculation of the magnitude scale factors is done in the same step as the calculation of the energy for the freq. offset estimation -> very efficient :)
  */
  const gr_complex *iptr = (const gr_complex *) input_items[0];
  const char *frame_start = (const char *) input_items[1];
  
  gr_complex *optr = (gr_complex *) output_items[0];
  char *frame_start_out = (char *) output_items[1];

  if (frame_start[0]) {
    frame_start_out[0] = 1;
    correlate_energy(iptr);
    d_delta_f = d_freq_offset+d_num_carriers/2-d_fft_length/2;
    fprintf(stderr, "cfs: coarse freq. offset (subcarriers): %d\n", d_freq_offset);
    d_symbol_num = 0;
  } else {
    frame_start_out[0] = 0;
    d_symbol_num++;
  }
  
  /* correct phase offset from removing cp */
  /* could be done after diff phasor, then it would be the same offset for each symbol; but its hardly much of an overhead */
  phase_offset_correct = gr_expj(-M_TWOPI*(float)d_delta_f*(float)d_cp_length/(float)d_fft_length * (float)d_symbol_num);

  for (i=0;i<d_num_carriers/2;i++) {
    optr[i] = iptr[d_freq_offset+i]*phase_offset_correct;
  }
  for (i=d_num_carriers/2;i<d_num_carriers;i++) {
    optr[i] = iptr[d_freq_offset+i+1]*phase_offset_correct;
  }

  return 1;
}

}
}
