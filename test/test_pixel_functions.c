/* test_ring_to_nest.c -- check the implementation of
 * hpix_ring_to_nest_idx
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

#include <hpixlib/hpix.h>
#include <math.h>
#include <stdlib.h>
#include <check.h>
#include "check_helpers.h"

double red = 1.0;
double green = 2.0;
double blue = 3.0;

/**********************************************************************/

START_TEST(valid_nside)
{
    for(hpix_nside_t nside = 1; nside < 1024; nside *= 2)
	ck_assert(hpix_valid_nside(nside));
    
    ck_assert(! hpix_valid_nside(13));
    ck_assert(! hpix_valid_nside(28));
    ck_assert(! hpix_valid_nside(1025));
    ck_assert(! hpix_valid_nside(3166));
}
END_TEST

/**********************************************************************/

START_TEST(npixel_to_nside)
{
    int result = 1;
    unsigned short nside;

    for(nside = 1; nside < 1024; nside *= 2)
    {
	ck_assert_int_eq(hpix_npixel_to_nside(nside * nside * 12),
			 nside);
    }

    /* Check for failures */
    ck_assert_int_eq(hpix_npixel_to_nside(11),
		     0);
}
END_TEST

/**********************************************************************/

START_TEST(nside_to_npixel)
{
    ck_assert_int_eq(hpix_nside_to_npixel(64),   49152);
    ck_assert_int_eq(hpix_nside_to_npixel(2048), 50331648);
    ck_assert_int_eq(hpix_nside_to_npixel(0),    0);
}
END_TEST
/**********************************************************************/

START_TEST(angles_to_pixel)
{
    /* Check for the conversion to the RING scheme */
    ck_assert_int_eq(hpix_angles_to_ring_pixel(256, 0.1, 0.1),  1861);
    ck_assert_int_eq(hpix_angles_to_ring_pixel(256, 0.2, 0.1),  7567);
    ck_assert_int_eq(hpix_angles_to_ring_pixel(256, 0.3, 0.1), 17117);

    ck_assert_int_eq(hpix_angles_to_ring_pixel(256, 0.1, 0.2),  1863);
    ck_assert_int_eq(hpix_angles_to_ring_pixel(256, 0.2, 0.2),  7571);
    ck_assert_int_eq(hpix_angles_to_ring_pixel(256, 0.3, 0.2), 17123);

    ck_assert_int_eq(hpix_angles_to_ring_pixel(256, 0.1, 0.3),  1865);
    ck_assert_int_eq(hpix_angles_to_ring_pixel(256, 0.2, 0.3),  7575);
    ck_assert_int_eq(hpix_angles_to_ring_pixel(256, 0.3, 0.3), 17129);

    /* Check for the conversion to the NEST scheme */
    ck_assert_int_eq(hpix_angles_to_nest_pixel(256, 0.1, 0.1), 65196);
    ck_assert_int_eq(hpix_angles_to_nest_pixel(256, 0.2, 0.1), 64177);
    ck_assert_int_eq(hpix_angles_to_nest_pixel(256, 0.3, 0.1), 61128);

    ck_assert_int_eq(hpix_angles_to_nest_pixel(256, 0.1, 0.2), 65200);
    ck_assert_int_eq(hpix_angles_to_nest_pixel(256, 0.2, 0.2), 64193);
    ck_assert_int_eq(hpix_angles_to_nest_pixel(256, 0.3, 0.2), 61044);

    ck_assert_int_eq(hpix_angles_to_nest_pixel(256, 0.1, 0.3), 65180);
    ck_assert_int_eq(hpix_angles_to_nest_pixel(256, 0.2, 0.3), 64113);
    ck_assert_int_eq(hpix_angles_to_nest_pixel(256, 0.3, 0.3), 60856);
}
END_TEST

/**********************************************************************/

START_TEST(pixel_to_angles)
{
    double theta, phi;

#define CHECK_ANGLE(nside, schema, pixel, target_theta, target_phi)	\
    hpix_ ## schema ## _pixel_to_angles(nside, pixel, &theta, &phi);	\
    TEST_FOR_CLOSENESS(theta, target_theta);					\
    TEST_FOR_CLOSENESS(phi, target_phi);

    CHECK_ANGLE(256, ring,  1861, 0.09891295, 0.07600627);
    CHECK_ANGLE(256, ring,  7567, 0.19806888, 0.08867399);
    CHECK_ANGLE(256, ring, 17117, 0.29771618, 0.09289656);

    CHECK_ANGLE(256, ring,  1863, 0.09891295, 0.17734797);
    CHECK_ANGLE(256, ring,  7571, 0.19806888, 0.19001568);
    CHECK_ANGLE(256, ring, 17123, 0.29771618, 0.19423826);

    CHECK_ANGLE(256, ring,  1865, 0.09891295, 0.27868967);
    CHECK_ANGLE(256, ring,  7575, 0.19806888, 0.29135738);
    CHECK_ANGLE(256, ring, 17129, 0.29771618, 0.29557995);

    CHECK_ANGLE(256, nest, 65196, 0.09891295, 0.07600627);
    CHECK_ANGLE(256, nest, 64177, 0.19806888, 0.08867399);
    CHECK_ANGLE(256, nest, 61128, 0.29771618, 0.09289656);

    CHECK_ANGLE(256, nest, 65200, 0.09891295, 0.17734797);
    CHECK_ANGLE(256, nest, 64193, 0.19806888, 0.19001568);
    CHECK_ANGLE(256, nest, 61044, 0.29771618, 0.19423826);

    CHECK_ANGLE(256, nest, 65180, 0.09891295, 0.27868967);
    CHECK_ANGLE(256, nest, 64113, 0.19806888, 0.29135738);
    CHECK_ANGLE(256, nest, 60856, 0.29771618, 0.29557995);
}
END_TEST

/**********************************************************************/

START_TEST(nest_to_ring)
{
    ck_assert_int_eq(hpix_nest_to_ring_idx( 64,    9632),    9010);
    ck_assert_int_eq(hpix_nest_to_ring_idx(256,    1652),  324237);
    ck_assert_int_eq(hpix_nest_to_ring_idx(512, 2966186), 2800416);

    /* Check for failures */
    ck_assert_int_eq(hpix_nest_to_ring_idx(4, 1000000), 0);
    ck_assert_int_eq(hpix_nest_to_ring_idx(0, 1),       0);
}
END_TEST

/**********************************************************************/

START_TEST(ring_to_nest)
{
    ck_assert_int_eq(hpix_ring_to_nest_idx( 64,    9010),    9632);
    ck_assert_int_eq(hpix_ring_to_nest_idx(256,  324237),    1652);
    ck_assert_int_eq(hpix_ring_to_nest_idx(512, 2800416), 2966186);

    /* Check for failures */
    ck_assert_int_eq(hpix_ring_to_nest_idx(4, 1000000), 0);
    ck_assert_int_eq(hpix_ring_to_nest_idx(0, 1),       0);
}
END_TEST

/**********************************************************************/

START_TEST(switch_order)
{
    /* A sample map with NSIDE = 2, assumed to be in RING ordering.
     * Remember that integer numbers smaller than 2^53 are represented
     * exactly as floating-point. */
    double ring_idx[] = {  0,  1,  2,  3,  4,  5,  6,  7,
			   8,  9, 10, 11, 12, 13, 14, 15, 
			  16, 17, 18, 19, 20, 21, 22, 23, 
			  24, 25, 26, 27, 28, 29, 30, 31, 
			  32, 33, 34, 35, 36, 37, 38, 39, 
			  40, 41, 42, 43, 44, 45, 46, 47 };

    /* The same map, but with NEST ordering */
    double nest_idx[] = {  3,  7, 11, 15,  2,  1,  6,  5,
			  10,  9, 14, 13, 19,  0, 23,  4,
			  27,  8, 31, 12, 17, 22, 21, 26, 
			  25, 30, 29, 18, 16, 35, 20, 39, 
			  24, 43, 28, 47, 34, 33, 38, 37, 
			  42, 41, 46, 45, 32, 36, 40, 44 };

    const size_t num_of_pixels = 48;
    hpix_map_t * map =
	hpix_create_map_from_array(ring_idx, num_of_pixels,
				   HPIX_ORDER_SCHEME_RING);

    hpix_switch_order(map);
    double * map_pixels = hpix_map_pixels(map);

    for(size_t i = 0; i < num_of_pixels; ++i)
	ck_assert_int_eq(map_pixels[i], nest_idx[i]);

    hpix_switch_order(map);
    map_pixels = hpix_map_pixels(map);

    for(size_t i = 0; i < num_of_pixels; ++i)
	ck_assert_int_eq(map_pixels[i], ring_idx[i]);
    
    hpix_free_map(map);
}
END_TEST

/**********************************************************************/

START_TEST(query_disc)
{
    ck_assert_int_eq(1, 0);
}
END_TEST

/**********************************************************************/

void
add_color_and_palette_tests_to_testcase(TCase * testcase)
{
    tcase_add_test(testcase, valid_nside);
    tcase_add_test(testcase, npixel_to_nside);
    tcase_add_test(testcase, nside_to_npixel);
    tcase_add_test(testcase, angles_to_pixel);
    tcase_add_test(testcase, pixel_to_angles);
    tcase_add_test(testcase, nest_to_ring);
    tcase_add_test(testcase, ring_to_nest);
    tcase_add_test(testcase, switch_order);
    tcase_add_test(testcase, query_disc);
}

/**********************************************************************/

Suite *
create_hpix_test_suite(void)
{
    Suite * suite = suite_create("Pixel functions");

    TCase * tc_core = tcase_create("Pixel functions");
    add_color_and_palette_tests_to_testcase(tc_core);

    suite_add_tcase(suite, tc_core);
    return suite;
}

/**********************************************************************/

int
main(void)
{
    int number_failed;
    Suite * palette_suite = create_hpix_test_suite();
    SRunner * runner = srunner_create(palette_suite);
    srunner_run_all(runner, CK_VERBOSE);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
