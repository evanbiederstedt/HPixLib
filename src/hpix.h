/* hpxlib.h -- header file for the HPixLib library
 *
 * Copyright 2011-2012 Maurizio Tomasi.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef HPXLIB_H
#define HPXLIB_H

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include "fitsio.h"

#define CHEALPIX_VERSION "0.1"

typedef unsigned short hpix_nside_t;
typedef unsigned long hpix_pixel_num_t;

typedef enum {
    HPXLIB_ORDER_RING,
    HPXLIB_ORDER_NEST
} hpix_ordering_t;

typedef enum {
    HPXLIB_COORD_CUSTOM,
    HPXLIB_COORD_ECLIPTIC,
    HPXLIB_COORD_GALACTIC,
    HPXLIB_COORD_CELESTIAL
} hpix_coordinates_t;

typedef struct {
    hpix_ordering_t    order;
    hpix_coordinates_t coord;
    hpix_nside_t       nside;
    double           * pixels;
} hpix_map_t;

#define HPIX_MAP_PIXEL(map, index)				\
    (*((double *) (((char *) map->pixels)			\
		   + (index) * sizeof(map->pixels[0]))))

struct ___hpix_bmp_projection_t;
typedef struct ___hpix_bmp_projection_t hpix_bmp_projection_t;

/* Functions implemented in mem.c */

void * hpix_malloc(size_t size, size_t num);
void * hpix_calloc(size_t size, size_t num);
void * hpix_realloc(void * ptr, size_t size);
void hpix_free(void * ptr);

/* Functions implemented in misc.c */

hpix_pixel_num_t hpix_nside_to_npixel(hpix_nside_t);
hpix_nside_t hpix_npixel_to_nside(hpix_pixel_num_t);
double hpix_max_pixel_radius(hpix_nside_t);

/* Functions implemented in map.c */

hpix_map_t * hpix_create_map(hpix_nside_t nside, hpix_ordering_t order);

hpix_map_t * hpix_create_map_from_array(double * array,
					    size_t num_of_elements,
					    hpix_ordering_t order);

void hpix_free_map(hpix_map_t * map);

hpix_ordering_t hpix_map_ordering(const hpix_map_t * map);

hpix_coordinates_t hpix_map_coordinate_system(const hpix_map_t * map);

hpix_nside_t hpix_map_nside(const hpix_map_t * map);

double * hpix_map_pixels(const hpix_map_t * map);

size_t hpix_map_num_of_pixels(const hpix_map_t * map);

/* Functions implemented in io.c */

int
hpix_load_component_from_fitsptr(fitsfile * fptr,
				   unsigned short column_number,
				   hpix_map_t ** map,
				   int * status);

int hpix_load_component_from_file(const char * file_name,
				    unsigned short column_number,
				    hpix_map_t ** map,
				    int * status);

int
hpix_create_empty_fits_table_for_map(fitsfile * fptr,
				       const hpix_map_t * template_map,
				       unsigned short num_of_components,
				       const char * measure_unit,
				       int * status);

int
hpix_save_fits_component_to_fitsfile(fitsfile * fptr,
				       const hpix_map_t * map,
				       int data_type,
				       const char * measure_unit,
				       int * status);

int
hpix_save_fits_component_to_file(const char * file_name,
				   const hpix_map_t * map,
				   int data_type,
				   const char * measure_unit,
				   int * status);

int
hpix_load_fits_pol_from_fitsfile(fitsfile * fptr,
				   hpix_map_t ** map_i,
				   hpix_map_t ** map_q,
				   hpix_map_t ** map_u,
				   int * status);

int
hpix_load_fits_pol_from_file(const char * file_name,
			       hpix_map_t ** map_i,
			       hpix_map_t ** map_q,
			       hpix_map_t ** map_u,
			       int * status);

int
hpix_save_fits_pol_fitsfile(const char * file_name,
			      const hpix_map_t * map_i,
			      const hpix_map_t * map_q,
			      const hpix_map_t * map_u,
			      int data_type,
			      const char * measure_unit,
			      int * status);

int
hpix_save_fits_pol_map(const char * file_name,
			 const hpix_map_t * map_i,
			 const hpix_map_t * map_q,
			 const hpix_map_t * map_u,
			 int data_type,
			 const char * measure_unit,
			 int * status);

/* Functions implemented in positions.c */

void hpix_angles_to_3dvec(double theta, double phi,
			    double * x, double * y, double * z);

typedef hpix_pixel_num_t hpix_angles_to_pixel_fn_t(hpix_nside_t,
						       double, double);

hpix_pixel_num_t hpix_angles_to_ring_pixel(hpix_nside_t nside,
					       double theta,
					       double phi);

hpix_pixel_num_t hpix_angles_to_nest_pixel(hpix_nside_t nside,
					       double theta,
					       double phi);

void hpix_3dvec_to_angles(double x, double y, double z,
			    double * theta, double * phi);

typedef hpix_pixel_num_t hpix_3dvec_to_pixel_fn_t(hpix_nside_t,
						      double, double, double);

hpix_pixel_num_t hpix_3dvec_to_ring_pixel(hpix_nside_t nside,
					      double x, double y, double z);

hpix_pixel_num_t hpix_3dvec_to_nest_pixel(hpix_nside_t nside,
					      double x, double y, double z);

typedef void hpix_pixel_to_angles(hpix_nside_t, hpix_pixel_num_t,
				    double *, double *);

void hpix_ring_pixel_to_angles(hpix_nside_t nside, hpix_pixel_num_t pixel,
				 double * theta, double * phi);

void hpix_nest_pixel_to_angles(hpix_nside_t nside, hpix_pixel_num_t pixel,
				 double * theta, double * phi);

typedef void hpix_pixel_to_3dvec(hpix_nside_t, hpix_pixel_num_t,
				   double *, double *, double *);

void hpix_ring_pixel_to_3dvec(hpix_nside_t nside,
				double * x, double * y, double * z);

void hpix_nest_pixel_to_3dvec(hpix_nside_t nside,
				double * x, double * y, double * z);

/* Functions implemented in bitmap.c */

hpix_bmp_projection_t * 
hpix_create_bmp_projection(unsigned int width, unsigned int height);

void
hpix_free_bmp_projection(hpix_bmp_projection_t * proj);

unsigned int
hpix_bmp_projection_width(const hpix_bmp_projection_t * proj);

unsigned int
hpix_bmp_projection_height(const hpix_bmp_projection_t * proj);

void
hpix_set_bmp_projection_width(hpix_bmp_projection_t * proj,
				unsigned int width);

void
hpix_set_bmp_projection_height(hpix_bmp_projection_t * proj,
				 unsigned int height);

double *
hpix_bmp_trace_bitmap(const hpix_bmp_projection_t * proj,
			const hpix_map_t * map,
			double * min_value,
			double * max_value);

/* Functions implemented in order_conversion.c */

hpix_pixel_num_t
hpix_nest_to_ring_idx(hpix_nside_t nside, hpix_pixel_num_t nest_index);

hpix_pixel_num_t
hpix_ring_to_nest_idx(hpix_nside_t nside, hpix_pixel_num_t ring_index);

void
hpix_switch_order(hpix_map_t * map);

/* Functions implemented in query_disc.c */

void hpix_query_disc(double theta, double phi, double radius,
		       hpix_pixel_num_t ** pixels,
		       size_t * num_of_matches);

void hpix_query_disc_inclusive(double theta, double phi, double radius,
				 hpix_pixel_num_t ** pixels,
				 size_t * num_of_matches);


#ifdef __cplusplus
};
#endif /* __cplusplus */
#endif