/*	main.c
	Copyright (C) 2004-2006 Mark Tyler

	This file is part of mtPaint.

	mtPaint is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	mtPaint is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with mtPaint in the file COPYING.
*/

#include <stdlib.h>
#include <gtk/gtk.h>

#include "global.h"
#include "memory.h"
#include "png.h"
#include "mainwindow.h"
#include "otherwindow.h"
#include "viewer.h"
#include "inifile.h"
#include "canvas.h"
#include "layer.h"
#include "csel.h"


int main( int argc, char *argv[] )
{
	gboolean new_empty = TRUE, get_screenshot = FALSE;
	char ppath[256];

	if (argc > 1)
	{
		if ( strcmp(argv[1], "--version") == 0 )
		{
			printf("%s\n\n", VERSION);
			exit(0);
		}
		if ( strcmp(argv[1], "--help") == 0 )
		{
			printf("%s\n\n"
				"Usage: mtpaint [option] [imagefile ... ]\n\n"
				"Options:\n"
				"  --help          Output this help\n"
				"  --version       Output version information\n"
				"  -s              Grab screenshot\n"
				"  -v              Start in viewer mode\n\n"
			, VERSION);
			exit(0);
		}
	}

	global_argv = argv;
	putenv( "G_BROKEN_FILENAMES=1" );	// Needed to read non ASCII filenames in GTK+2
	inifile_init("/.mtpaint");

	gtk_init( &argc, &argv );

#ifdef U_NLS
	setup_language();
	bindtextdomain("mtpaint", MT_LANG_DEST);
	textdomain("mtpaint");
#if GTK_MAJOR_VERSION == 2
	bind_textdomain_codeset("mtpaint", "UTF-8");
#endif
#endif

	files_passed = argc - 1;
	if (argc > 1)		// Argument received, so assume user is trying to load a file
	{
		file_arg_start = 1;
		if ( strcmp(argv[1], "-g") == 0 )	// Loading GIF animation frames
		{
			file_arg_start+=2;
			files_passed-=2;
			sscanf(argv[2], "%i", &preserved_gif_delay);
		}
		if ( strcmp(argv[1], "-v") == 0 )	// Viewer mode
		{
			file_arg_start++;
			files_passed--;
			viewer_mode = TRUE;
		}
		if ( strcmp(argv[1], "-s") == 0 )	// Screenshot
		{
			file_arg_start++;
			files_passed--;
			get_screenshot = TRUE;
		}
		if ( strstr(argv[0], "mtv") != NULL ) viewer_mode = TRUE;
	}

	mem_init();					// Set up memory & back end
	layers_init();
	init_cols();

// !!! Temporary init for gradients
grad_info zad = {
  0, 0, 0, 0, 0, /* tool */
  (GRAD_DEF_RGB | GRAD_FLAG_DEFAULT), (GRAD_DEF_CONST | GRAD_FLAG_DEFAULT),
  0, 0, 0, /* length, repeat, offset */
  GRAD_MODE_LINEAR, GRAD_BOUND_STOP };
gradient[0] = gradient[1] = gradient[2] = gradient[3] = zad;

	if ( get_screenshot )
	{
		if ( grab_screen() )
			new_empty = FALSE;		// Successfully grabbed so no new empty
		else
			get_screenshot = FALSE;		// Screenshot failed
	}
	main_init();					// Create main window

	if ( get_screenshot )
	{
		do_new_chores();
		notify_changed();
	}
	else
	{
		if (files_passed >0)
		{
			strncpy( ppath, argv[file_arg_start], 250 );
			if ( do_a_load( ppath ) == 0 ) new_empty = FALSE;
		}
	}

	if ( new_empty )		// If no file was loaded, start with a blank canvas
	{
		create_default_image();
	}
	update_menus();

	gtk_main();
	inifile_quit();

	return 0;
}
