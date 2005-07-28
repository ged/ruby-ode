/*
 * Playing around with rb_frame_last_func().
 * 
 */

#include <ruby.h>
#include <intern.h>

VALUE
rb_lastfunc()
{
	ID lastfunc = rb_frame_last_func();
	return ID2SYM( lastfunc );
}


void
Init_lastfunc()
{
	rb_define_global_function( "lastfunc", rb_lastfunc, 0 );
}

