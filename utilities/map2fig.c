#include <hpixlib/hpix.h>
#include <assert.h>
#include <math.h>
#include <memory.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cairo.h>
#include <cairo-ps.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>

#include "gopt.h"

#define VERSION "0.1"

#define MSG_HEADER   "map2fig: "

typedef enum { FMT_NULL, FMT_PNG, FMT_PS, FMT_EPS, FMT_PDF, FMT_SVG }
    output_format_code_t;

typedef struct {
    const char * name;
    const char * description;
    output_format_code_t code;
} output_format_t;

output_format_t list_of_output_formats[] = {
    /* The Cairo documentation guarantees that PNG is always available */
    { "png", "PNG 24-bit bitmap", FMT_PNG },
#if CAIRO_HAS_PS_SURFACE
    { "ps", "PostScript", FMT_PS },
    { "eps", "Encapsulated PostScript", FMT_EPS },
#endif
#if CAIRO_HAS_PDF_SURFACE
    { "pdf", "Adobe Portable Document Format", FMT_PDF },
#endif
#if CAIRO_HAS_SVG_SURFACE
    { "svg", "Scalable Vector Graphics", FMT_SVG },
#endif
    { NULL, FMT_NULL }
};

/* Output format to use */
output_format_code_t output_format = FMT_PNG;

/* Should we draw a color bar? Set by `-b`, `--draw-color-bar` */
int draw_color_bar_flag = 0;

/* Should we produce a number of diagnostic messages? Set by `--verbose` */
int verbose_flag = 0;

/* String to append to the measure unit, set by `-m`, `--measure-unit` */
const char * measure_unit_str = "";

/* Title to be drawn above the map, set by `-t`, `--title` */
const char * title_str = "";

/* Name of the output file name, set by `-o`, `--output` */
const char * output_file_name = NULL;

/* C-like format string for numbers, set by `-f`, `--format` */
const char * number_format = "%g";

const char * input_file_name = NULL;

/* Number of the column to display, set by `-c`, `--column` */
unsigned short column_number = 1;

/* Relative height of the title and of the color bar. Together with
 * the height of the map, their sum is 1.0 */
const float title_height_fraction = 0.1;
const float colorbar_height_fraction = 0.05;

/* Extrema of the color bar, set by `--min` and `--max` */
double min_value = NAN;
double max_value = NAN;

/* Size of the image. Depending on the output format, these number can
 * be pixels (PNG) or inches (PS, PDF, SVG). Therefore, they are set
 * once the format has been decided. */
double image_width;
double image_height;

/* Number of pixels in the bitmapped representation of the map. */
double bitmap_columns = 600;
double bitmap_rows = 400;


/******************************************************************************/


void
print_usage(const char * program_name)
{
    printf("Usage: %s [OPTIONS] INPUT_MAP\n\n", program_name);
    puts("OPTIONS can be one or more of the following:");
    puts("  -b, --draw-color-bar      Draw a color bar");
    puts("  -c, --column=NUM          Number of the column to display");
    puts("  -f, --format=STRING       C-like formatting string for numbers");
    puts("  --list-formats            Print a list of the formats that can");
    puts("                            be specified with --format");
    puts("  -m, --measure-unit=STRING Measure unit to use.");
    puts("  --min=VALUE, --max=VALUE  Minimum and maximum value to be used");
    puts("                            at the extrema of the color bar");
    puts("  -o, --output=FILE         Save the image to the specified file");
    puts("  -t, --title=TITLE         Title to be written");
    puts("  -v, --version             Print version number and exit");
    puts("  -h, --help                Print this help");
}

/******************************************************************************/


void
print_list_of_available_formats(void)
{
    int idx;
    for(idx = 0; list_of_output_formats[idx].name != NULL; ++idx)
    {
	output_format_t * format = &list_of_output_formats[idx];
	assert(format != NULL);
	printf("%s\t%s\n", format->name, format->description);
    }
}

/******************************************************************************/


void
parse_format_specification(const char * format_str)
{
    int idx;
    for(idx = 0; list_of_output_formats[idx].name != NULL; ++idx)
    {
	output_format_t * format = &list_of_output_formats[idx];
	assert(format != NULL);
	if(strcmp(format->name, format_str) == 0)
	{
	    output_format = format->code;
	    return;
	}
    }

    fprintf(stderr,
	    MSG_HEADER "unknown format `%s', get a list of the available\n"
	    MSG_HEADER "formats using `--list-formats'\n",
	    format_str);
}

/******************************************************************************/


void
parse_command_line(int argc, const char ** argv)
{
    const char * value_str;
    char * tail_ptr;
    void * options =
	gopt_sort(&argc, argv,
		  gopt_start(
		      gopt_option('h', 0, gopt_shorts('h', '?'), gopt_longs("help")),
		      gopt_option('v', 0, gopt_shorts('v'), gopt_longs("version")),
		      gopt_option('V', 0, gopt_shorts(0), gopt_longs("verbose")),
		      gopt_option('b', 0, gopt_shorts('b'), gopt_longs("draw-color-bar")),
		      gopt_option('c', 0, gopt_shorts('c'), gopt_longs("column")),	
		      gopt_option('F', 0, gopt_shorts(0), gopt_longs("list-formats")),
		      gopt_option('m', GOPT_ARG, gopt_shorts('m'), gopt_longs("measure-unit")),
		      gopt_option('o', GOPT_ARG, gopt_shorts('o'), gopt_longs("output")),
		      gopt_option('_', GOPT_ARG, gopt_shorts(0), gopt_longs("min")),
		      gopt_option('^', GOPT_ARG, gopt_shorts(0), gopt_longs("max")),
		      gopt_option('t', GOPT_ARG, gopt_shorts('t'), gopt_longs("title")),
		      gopt_option('f', GOPT_ARG, gopt_shorts('f'), gopt_longs("format"))));

    if(gopt(options, 'h'))
    {
	print_usage("map2fig");
	exit(EXIT_SUCCESS);
    }

    if(gopt(options, 'v'))
    {
	puts("map2fig version " VERSION " - Copyright(c) 2011-2012 Maurizio Tomasi");
	exit(EXIT_SUCCESS);
    }

    if(gopt(options, 'F'))
    {
	print_list_of_available_formats();
	exit(EXIT_SUCCESS);
    }

    if(gopt(options, 'V'))
	verbose_flag = 1;

    if(gopt(options, 'b'))
	draw_color_bar_flag = 1;

    if(gopt_arg(options, 'c', &value_str))
    {
	tail_ptr = NULL;
	column_number = strtoul(value_str, &tail_ptr, 10);
	if(! tail_ptr ||
	   *tail_ptr != '\x0' ||
	   column_number == 0 ||
	   column_number > USHRT_MAX)
	{
	    fprintf(stderr, MSG_HEADER "invalid column number '%s'\n",
		    value_str);
	    exit(EXIT_FAILURE);
	}
    }

    if(gopt_arg(options, 'f', &value_str))
	parse_format_specification(value_str);

    if(gopt_arg(options, '_', &value_str))
    {
	tail_ptr = NULL;
	min_value = strtod(value_str, &tail_ptr);
	if(! tail_ptr ||
	   *tail_ptr != '\x0')
	{
	    fprintf(stderr,
		    MSG_HEADER "invalid minimum '%s' specified with --min\n",
		    value_str);
	    exit(EXIT_FAILURE);
	}
    }

    if(gopt_arg(options, '^', &value_str))
    {
	tail_ptr = NULL;
	max_value = strtod(value_str, &tail_ptr);
	if(! tail_ptr ||
	   *tail_ptr != '\x0')
	{
	    fprintf(stderr,
		    MSG_HEADER "invalid maximum '%s' specified with --max\n",
		    value_str);
	    exit(EXIT_FAILURE);
	}
    }

    /* Save the option passed to `--measure-unit` into variable MEASURE_UNIT. */
    gopt_arg(options, 'm', &measure_unit_str);
    gopt_arg(options, 't', &title_str);
    gopt_arg(options, 'f', &number_format);
    gopt_arg(options, 'o', &output_file_name);

    gopt_free(options);

    if(argc > 2)
    {
	fputs(MSG_HEADER "too many command-line arguments (hint: use --help)\n",
	      stderr);
	exit(EXIT_FAILURE);
    }

    if(argc < 2)
    {
	fprintf(stderr,
		MSG_HEADER "reading maps from stdin is not supported yet\n"
		MSG_HEADER "(hint: specify the name of a FITS file to be read)\n");
	exit(EXIT_FAILURE);
    }

    input_file_name = argv[1];
}

/******************************************************************************/


hpix_map_t *
load_map(void)
{
    hpix_map_t * result;
    int status = 0;

    if(! hpix_load_fits_component_from_file(input_file_name,
					    column_number,
					    &result, &status))
    {
	fprintf(stderr, MSG_HEADER "unable to load file '%s'\n",
		input_file_name);
	exit(EXIT_FAILURE);
    }

    return result;
}

/******************************************************************************/


void
find_map_extrema(const hpix_map_t * map, double * min, double * max)
{
    const double * cur_pixel = hpix_map_pixels(map);
    size_t num_of_pixels = hpix_map_num_of_pixels(map);
    size_t counter;
    int are_minmax_initialized = 0;

    assert(map != NULL);
    assert(min != NULL);
    assert(max != NULL);

    for(counter = 0; counter < num_of_pixels; ++counter, ++cur_pixel)
    {
	if(isnan(*cur_pixel) || *cur_pixel < -1.6e+30)
	    continue;

	if (! are_minmax_initialized)
	{
	    *min = *max = *cur_pixel;
	    are_minmax_initialized = 1;
	}
	else
	{
	    if(*min > *cur_pixel)
		*min = *cur_pixel;
	    else if (*max < *cur_pixel)
		*max = *cur_pixel;
	}
    }
}

/******************************************************************************/


typedef struct {
    double red;
    double green;
    double blue;
} color_t;

static const double levels[] = { 0.0, 0.15, 0.40, 0.70, 0.90, 1.00 };
static const color_t colors[] = {
    { 0.0, 0.0, 0.5 },
    { 0.0, 0.0, 1.0 },
    { 0.0, 1.0, 1.0 },
    { 1.0, 1.0, 0.0 },
    { 1.0, 0.33, 0.0 },
    { 0.5, 0.0, 0.0 }};
static const size_t num_of_levels = sizeof(levels) / sizeof(levels[0]);

void
get_palette_color(double level, color_t * color_ptr)
{
    size_t idx;
    size_t index0, index1;

    if(level <= 0.0)
    {
	memcpy(color_ptr, (const void *) &colors[0],
	       sizeof(color_t));
	return;
    }

    if(level >= 1.0)
    {
	memcpy(color_ptr, (const void *) &colors[num_of_levels - 1],
	       sizeof(color_t));
	return;
    }

    idx = 0;
    while(level > levels[idx])
	++idx;

    index1 = idx;
    index0 = index1 - 1;

#define INTERPOLATE_COMPONENT(level, comp_name) \
    (  colors[index0].comp_name * (levels[index1] - level) / (levels[index1] - levels[index0]) \
     + colors[index1].comp_name * (level - levels[index0]) / (levels[index1] - levels[index0]))

    color_ptr->red   = INTERPOLATE_COMPONENT(level, red);
    color_ptr->green = INTERPOLATE_COMPONENT(level, green);
    color_ptr->blue  = INTERPOLATE_COMPONENT(level, blue);

#undef INTERPOLATE_COMPONENT
}

/******************************************************************************/


cairo_surface_t *
plot_bitmap_to_cairo_surface(double map_min, double map_max,
			     const double * bitmap,
			     unsigned int width,
			     unsigned int height)
{
    const double dynamic_range = map_max - map_min;
    const double * cur_pixel = bitmap;
    unsigned char * image_data;
    unsigned int cur_y;
    unsigned int stride;
    cairo_surface_t * surface;

    assert(bitmap);
    fprintf(stderr, MSG_HEADER "plotting the map on a %ux%u bitmap\n",
	    width, height);
    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
					 width, height);
    image_data = cairo_image_surface_get_data(surface);
    stride = cairo_image_surface_get_stride(surface);

    for(cur_y = 0; cur_y < height; ++cur_y)
    {
	unsigned int cur_x;
	unsigned char * row = image_data + (height - cur_y - 1) * stride;
	for(cur_x = 0; cur_x < width; ++cur_x)
	{
	    double value = *cur_pixel++;
	    color_t color;

	    /* Not sure if this works on big-endian machines... */
#define SET_PIXEL(a,r,g,b) {	      \
		int base = cur_x * 4; \
		row[base]     = b;    \
		row[base + 1] = g;    \
		row[base + 2] = r;    \
		row[base + 3] = a;    \
	    }

	    if(isinf(value))
	    {
		/* Transparent pixel */
		SET_PIXEL(0, 255, 255, 255);
	    }
	    else if (isnan(value) || value < -1.6e+30)
	    {
		/* Opaque pixel with a gray shade */
		SET_PIXEL(255, 128, 128, 128);
	    }
	    else
	    {
		double normalized_value = (value - map_min) / dynamic_range;
		get_palette_color(normalized_value, &color);
		SET_PIXEL(255, /* This makes the pixel fully opaque */
			  (int) (255 * color.red),
			  (int) (255 * color.green),
			  (int) (255 * color.blue));
	    }
#undef SET_PIXEL
	}
    }

    return surface;
}

/******************************************************************************/


void
paint_title(cairo_t * context, double start_x, double start_y,
	    double width, double height)
{
    cairo_text_extents_t title_te;
    const double title_font_size = height * 0.9;

    cairo_set_font_size(context, title_font_size);
    cairo_text_extents(context, title_str, &title_te);
    cairo_move_to(context, 0.5 * (width - title_te.width),
		  title_font_size);
    cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
    cairo_show_text(context, title_str);
}

/******************************************************************************/


void
paint_colorbar(cairo_t * context,
	       double start_x, double start_y,
	       double width, double height,
	       double min_level, double max_level)
{
    cairo_pattern_t * linear;
    size_t idx;
    char label_min[20], label_max[20];
    cairo_text_extents_t min_te, max_te;
    double bar_start_x, bar_start_y;
    double bar_width, bar_height;
    const double text_margin_factor = 1.1;
    double tick_height;

    if(output_format == FMT_PNG)
	tick_height = 6.0;
    else
	tick_height = 0.1;

    if(measure_unit_str != NULL
       && measure_unit_str[0] != '\0')
    {
	sprintf(label_min, "%.4g %s", min_level, measure_unit_str);
	sprintf(label_max, "%.4g %s", max_level, measure_unit_str);
    } else {
	sprintf(label_min, "%.4g", min_level);
	sprintf(label_max, "%.4g", max_level);
    }

    cairo_text_extents (context, label_min, &min_te);
    cairo_text_extents (context, label_max, &max_te);

    bar_start_x = start_x;
    bar_start_y = start_y;
    bar_width = width;
    bar_height = height;

    /* If zero is within the range, plot a small thick around it */
    if(max_level > 0.0 && min_level < 0.0)
    {
	double zero_pos = 
	    start_x + width * (0.0 - min_level) / (max_level - min_level);
	cairo_move_to(context, zero_pos, start_y);
	cairo_line_to(context, zero_pos, start_y + height);
	cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
	cairo_stroke(context);
    }

    /* Now plot the gradient */
    bar_start_x += min_te.width * text_margin_factor;
    bar_width -= (min_te.width + max_te.width) * text_margin_factor;
    linear = cairo_pattern_create_linear (bar_start_x, 0.0,
					  bar_start_x + bar_width, 0.0);

    for(idx = 0; idx < num_of_levels; ++idx)
    {
	cairo_pattern_add_color_stop_rgb(linear, levels[idx],
					 colors[idx].red,
					 colors[idx].green,
					 colors[idx].blue);
    }

    cairo_rectangle(context,
		    bar_start_x, bar_start_y + tick_height,
		    bar_width, bar_height - 2 * tick_height);

    /* Draw the gradient */
    cairo_set_source(context, linear);
    cairo_fill_preserve(context);

    cairo_pattern_destroy(linear);

    /* Draw the border */
    cairo_set_source_rgb(context, 0.0, 0.0, 0.0);
    cairo_stroke(context);

    /* Draw the labels */
    {
	double baseline;
	baseline =
	    start_y
	    + height * 0.5 
	    - min_te.y_bearing 
	    - min_te.height * 0.5;
	cairo_move_to(context, start_x, baseline);
	cairo_show_text(context, label_min);

	 baseline =
	     start_y
	     + height * 0.5 
	     - max_te.y_bearing 
	     - max_te.height * 0.5;
	 cairo_move_to(context, start_x + width - max_te.width,
		       baseline);
	 cairo_show_text(context, label_max);
    }
}

/******************************************************************************/


cairo_surface_t *
create_surface(double width, double height)
{
    cairo_surface_t * surface;

    switch(output_format)
    {
    case FMT_PNG:
	surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
					     width, height);
	bitmap_columns = width;
	break;

#if CAIRO_HAS_PS_SURFACE
    case FMT_PS:
    case FMT_EPS:
	surface = cairo_ps_surface_create(output_file_name,
					     width, height);
	if(output_format == FMT_EPS)
	    cairo_ps_surface_set_eps(surface, TRUE);
	break;
#endif

#if CAIRO_HAS_PDF_SURFACE
    case FMT_PDF:
	surface = cairo_pdf_surface_create(output_file_name,
					     width, height);
	break;
#endif

#if CAIRO_HAS_SVG_SURFACE
    case FMT_SVG:
	surface = cairo_svg_surface_create(output_file_name,
					     width, height);
	break;
#endif
    default:
	assert(0);
    }

    return surface;
}

/******************************************************************************/


void
paint_map(const hpix_map_t * map)
{
    cairo_surface_t * surface;
    cairo_t * context;
    hpix_bmp_projection_t * projection;
    double min, max;

    const double title_start_y = 0.0;
    const double map_start_y = title_height_fraction * image_height;
    const double colorbar_start_y = image_height
	* (1 - colorbar_height_fraction);

    const double title_height = map_start_y;
    const double map_height = image_height
	* (1 - title_height_fraction - colorbar_height_fraction);
    const double colorbar_height = image_height * colorbar_height_fraction;

    find_map_extrema(map, &min, &max);
    if(! isnan(min_value))
	min = min_value;
    if(! isnan(max_value))
	max = max_value;

    if(verbose_flag)
	fprintf(stderr,
		MSG_HEADER "map extrema are %g and %g, " 
		"with a range of %g\n",
		min, max, max - min);

    surface = create_surface(image_width, image_height);
    context = cairo_create(surface);

    if(output_format == FMT_PNG)
	bitmap_rows = map_height;

    /* Draw the background */
    cairo_set_source_rgb(context, 1.0, 1.0, 1.0);
    cairo_paint(context);

    /* Draw the title */
    paint_title(context,
		0.0, title_start_y,
		image_width, title_height);

    /* Plot the map */
    {
	cairo_surface_t * map_surface;
	double * map_bitmap;
	const double reduce_factor = 1.02;

	/* First produce a cairo image surface with the map in it */
	projection = hpix_create_bmp_projection((int) (bitmap_columns + .5),
						(int) (bitmap_rows + .5));
	map_bitmap = hpix_bmp_trace_bitmap(projection, map, NULL, NULL);
	map_surface =
	    plot_bitmap_to_cairo_surface(min, max,
					 map_bitmap,
					 hpix_bmp_projection_width(projection),
					 hpix_bmp_projection_height(projection));
	hpix_free(map_bitmap);

	/* Now copy the cairo surface into the surface we're currently
	 * using to draw the figure */
	cairo_save(context);

	/* These transformations are useful for the map containing the
	 * bitmap (i.e. the source in the copy operation). */
	cairo_translate(context, 0.0, map_start_y);
	cairo_scale(context,
		    image_width / cairo_image_surface_get_width(map_surface),
		    map_height  / cairo_image_surface_get_height(map_surface));
	cairo_set_source_surface(context, map_surface,
	  0.0, 0.0);

	/* Now we need two more transformations in order to draw an
	 * ellipse out of `cairo_arc'. */
	cairo_translate(context,
			cairo_image_surface_get_width(map_surface) / 2.0,
			cairo_image_surface_get_height(map_surface) / 2.0);
	cairo_scale(context,
		    cairo_image_surface_get_width(map_surface) / 2.0,
		    cairo_image_surface_get_height(map_surface) / 2.0);

	/* Fill an ellipse with the content of `map_surface'. */
	cairo_arc(context, 0.0, 0.0, 1.0, 0.0, 2 * M_PI);
	cairo_fill(context);
	cairo_restore(context);

	/* Cleanup */
	cairo_surface_destroy(map_surface);
	hpix_free_bmp_projection(projection);
    }

    cairo_set_font_size(context, colorbar_height * 0.8);
    paint_colorbar(context,
		   0.01 * image_width, colorbar_start_y,
		   image_width * 0.98, colorbar_height,
		   min, max);

    if(output_format == FMT_PNG)
    {
	fprintf(stderr, MSG_HEADER "writing the file to `%s'\n",
		output_file_name);
	if(cairo_surface_write_to_png(surface, output_file_name)
	   != CAIRO_STATUS_SUCCESS)
	{
	    fprintf(stderr, MSG_HEADER "unable to write to file '%s'\n",
		    output_file_name);
	    exit(EXIT_FAILURE);
	}
	fputs(MSG_HEADER "file has been written successfully\n", stderr);
    } else {
	cairo_show_page(context);
    }

    cairo_destroy(context);
    cairo_surface_destroy(surface);
}

/******************************************************************************/


int
main(int argc, const char ** argv)
{
    hpix_map_t * map;

    parse_command_line(argc, argv);

    if(verbose_flag)
	fprintf(stderr, MSG_HEADER "loading map `%s'\n", input_file_name);
    map = load_map();
    if(verbose_flag)
	fprintf(stderr, MSG_HEADER "map loaded\n");

    switch(output_format)
    {
    case FMT_PNG:
	image_width = 750;
	image_height = 500;
	break;

    case FMT_PS:
    case FMT_EPS:
    case FMT_PDF:
    case FMT_SVG:
	image_width = 7.5 * 72;
	image_height = 5.0 * 72;
	break;

    default:
	assert(0);
    }

    if(verbose_flag)
	fprintf(stderr, MSG_HEADER "painting map\n");
    paint_map(map);

    hpix_free_map(map);

    return 0;
}
