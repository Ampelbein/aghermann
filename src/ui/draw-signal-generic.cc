// ;-*-C++-*-
/*
 *       File name:  ui/draw-signal-generic.cc
 *         Project:  Aghermann
 *          Author:  Andrei Zavada <johnhommer@gmail.com>
 * Initial version:  2011-05-09
 *
 *         Purpose:  generic draw_signal to a cairo_t
 *
 *         License:  GPL
 */




#include <samplerate.h>
#include "draw-signal-generic.hh"

using namespace std;

// must strictly take float due to samplerate routines requirements
void
draw_signal( const valarray<float>& signal,
	     size_t start, size_t end,
	     unsigned width, int vdisp, float display_scale,
	     cairo_t *cr, bool use_resample)
{
	static float* _resample_buffer = NULL;
	static size_t _resample_buffer_size = 0;

	size_t	true_length = min(end - start, signal.size() - start),
		true_width  = width * ((double)true_length/(end - start));

	if ( use_resample ) {
		if ( _resample_buffer_size != width )
			_resample_buffer = (float*)realloc( _resample_buffer,
							    (_resample_buffer_size = width) * sizeof(float));
		SRC_DATA samples;
		samples.data_in       = const_cast<float*>(&signal[start]);
		samples.input_frames  = true_length;
		samples.output_frames = true_width;
		samples.data_out      = _resample_buffer;
		samples.src_ratio     = (double)samples.output_frames / samples.input_frames;

		src_simple( &samples, SRC_SINC_FASTEST, 1);

		size_t i;
		cairo_move_to( cr, 0,
			       - samples.data_out[0]
			       * display_scale
			       + vdisp);
		for ( i = 0; i < true_width-1; ++i )
			cairo_line_to( cr, i,
				       - samples.data_out[i]
				       * display_scale
				       + vdisp);

	} else {
		size_t i;
		cairo_move_to( cr, 0,
			       - signal[start]
			       * display_scale
			       + vdisp);
		for ( i = 0; i < true_length; ++i ) {
			cairo_line_to( cr, ((float)i)/true_length * true_width,
				       - signal[start + i]
				       * display_scale
				       + vdisp);
		}
	}
	cairo_stroke( cr);
}



// eof
