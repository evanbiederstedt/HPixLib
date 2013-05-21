/* matrices.c -- matrix functions
 *
 * Copyright 2011-2013 Maurizio Tomasi.
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
#include <assert.h>

/**********************************************************************/


void
hpix_set_matrix_to_unity(hpix_matrix_t * matrix)
{
    assert(matrix);

    matrix->m[0][0] = 1.0; matrix->m[0][1] = 0.0; matrix->m[0][2] = 0.0; 
    matrix->m[1][0] = 0.0; matrix->m[1][1] = 1.0; matrix->m[1][2] = 0.0; 
    matrix->m[2][0] = 0.0; matrix->m[2][1] = 0.0; matrix->m[2][2] = 1.0; 
}

/**********************************************************************/


void
hpix_set_matrix_to_zero(hpix_matrix_t * matrix)
{
    assert(matrix);

    matrix->m[0][0] = 0.0; matrix->m[0][1] = 0.0; matrix->m[0][2] = 0.0; 
    matrix->m[1][0] = 0.0; matrix->m[1][1] = 0.0; matrix->m[1][2] = 0.0; 
    matrix->m[2][0] = 0.0; matrix->m[2][1] = 0.0; matrix->m[2][2] = 0.0; 
}

/**********************************************************************/


void
hpix_set_matrix_to_scale_transform(hpix_matrix_t * matrix,
				   double scale_x,
				   double scale_y,
				   double scale_z)
{
    assert(matrix);

    matrix->m[0][0] = scale_x; matrix->m[0][1] = 0.0;     matrix->m[0][2] = 0.0; 
    matrix->m[1][0] = 0.0;     matrix->m[1][1] = scale_y; matrix->m[1][2] = 0.0; 
    matrix->m[2][0] = 0.0;     matrix->m[2][1] = 0.0;     matrix->m[2][2] = scale_z; 
}

/**********************************************************************/


_Bool
hpix_is_matrix_zero(const hpix_matrix_t * matrix)
{
    assert(matrix);

    return 
	(matrix->m[0][0] == 0.0) && (matrix->m[0][1] == 0.0) && (matrix->m[0][2] == 0.0) && 
	(matrix->m[1][0] == 0.0) && (matrix->m[1][1] == 0.0) && (matrix->m[1][2] == 0.0) && 
	(matrix->m[2][0] == 0.0) && (matrix->m[2][1] == 0.0) && (matrix->m[2][2] == 0.0);
}

/**********************************************************************/


void
hpix_print_matrix(FILE * output_file, 
		  const hpix_matrix_t * matrix,
		  unsigned int num_of_indents,
		  _Bool indent_first_line)
{
    assert(output_file);
    assert(matrix);

    for(int j = 0; j < 3; ++j) {
	if((j == 0 && indent_first_line) || j > 0) {
	    for(int indents = 0; indents < num_of_indents; ++indents) {
		fputc(' ', output_file);
	    }
	}

	for(int i = 0; i < 3; ++i) {
	    fprintf(output_file, "%g\t", matrix->m[i][j]);
	}

	fputc('\n', output_file);
    }
}

/**********************************************************************/


void
hpix_matrix_vector_mul(hpix_vector_t * result,
		      const hpix_matrix_t * matrix,
		      const hpix_vector_t * vector)
{
    assert(result);
    assert(matrix);
    assert(vector);

    result->x = 
	matrix->m[0][0] * vector->x +
	matrix->m[0][1] * vector->y +
	matrix->m[0][2] * vector->z;

    result->y = 
	matrix->m[1][0] * vector->x +
	matrix->m[1][1] * vector->y +
	matrix->m[1][2] * vector->z;

    result->z = 
	matrix->m[2][0] * vector->x +
	matrix->m[2][1] * vector->y +
	matrix->m[2][2] * vector->z;
}

/**********************************************************************/


void
hpix_matrix_mul(hpix_matrix_t * result,
		const hpix_matrix_t * matrix1,
		const hpix_matrix_t * matrix2)
{
    assert(result);
    assert(matrix1);
    assert(matrix1);

    /* This code was automatically generated by Wolfram Mathematica 9
     * using CForm */
    result->m[0][0] = 
	matrix1->m[0][0] * matrix2->m[0][0] + 
	matrix1->m[0][1] * matrix2->m[1][0] + 
	matrix1->m[0][2] * matrix2->m[2][0];
    result->m[0][1] = 
	matrix1->m[0][0] * matrix2->m[0][1] + 
	matrix1->m[0][1] * matrix2->m[1][1] + 
	matrix1->m[0][2] * matrix2->m[2][1];
    result->m[0][2] = 
	matrix1->m[0][0] * matrix2->m[0][2] + 
	matrix1->m[0][1] * matrix2->m[1][2] + 
	matrix1->m[0][2] * matrix2->m[2][2];

    result->m[1][0] = 
	matrix1->m[1][0] * matrix2->m[0][0] + 
	matrix1->m[1][1] * matrix2->m[1][0] + 
	matrix1->m[1][2] * matrix2->m[2][0];
    result->m[1][1] = 
	matrix1->m[1][0] * matrix2->m[0][1] + 
	matrix1->m[1][1] * matrix2->m[1][1] + 
	matrix1->m[1][2] * matrix2->m[2][1];
    result->m[1][2] = 
	matrix1->m[1][0] * matrix2->m[0][2] + 
	matrix1->m[1][1] * matrix2->m[1][2] + 
	matrix1->m[1][2] * matrix2->m[2][2];

    result->m[2][0] = 
	matrix1->m[2][0] * matrix2->m[0][0] + 
	matrix1->m[2][1] * matrix2->m[1][0] + 
	matrix1->m[2][2] * matrix2->m[2][0];
    result->m[2][1] = 
	matrix1->m[2][0] * matrix2->m[0][1] + 
	matrix1->m[2][1] * matrix2->m[1][1] + 
	matrix1->m[2][2] * matrix2->m[2][1];
    result->m[2][2] = 
	matrix1->m[2][0] * matrix2->m[0][2] + 
	matrix1->m[2][1] * matrix2->m[1][2] + 
	matrix1->m[2][2] * matrix2->m[2][2];
}

/**********************************************************************/


double
hpix_matrix_determinant(const hpix_matrix_t * matrix)
{
    /* This code was automatically generated by Wolfram Mathematica 9
     * using CForm */
    return (- matrix->m[0][2] * matrix->m[1][1] * matrix->m[2][0] 
	    + matrix->m[0][1] * matrix->m[1][2] * matrix->m[2][0] 
	    + matrix->m[0][2] * matrix->m[1][0] * matrix->m[2][1] 
	    - matrix->m[0][0] * matrix->m[1][2] * matrix->m[2][1] 
	    - matrix->m[0][1] * matrix->m[1][0] * matrix->m[2][2] 
	    + matrix->m[0][0] * matrix->m[1][1] * matrix->m[2][2]);
}

/**********************************************************************/


void
hpix_matrix_inverse(hpix_matrix_t * result,
		    const hpix_matrix_t * matrix)
{
    assert(result);
    assert(matrix);

    double det = hpix_matrix_determinant(matrix);

    /* This code was automatically generated by Wolfram Mathematica 9
     * using CForm */
    result->m[0][0] = (- matrix->m[1][2]*matrix->m[2][1] 
		       + matrix->m[1][1]*matrix->m[2][2]) / det;
    result->m[0][1] = (+ matrix->m[0][2]*matrix->m[2][1] 
		       - matrix->m[0][1]*matrix->m[2][2]) / det;
    result->m[0][2] = (- matrix->m[0][2]*matrix->m[1][1] 
		       + matrix->m[0][1]*matrix->m[1][2]) / det;
   
    result->m[1][0] = (+ matrix->m[1][2]*matrix->m[2][0] 
		       - matrix->m[1][0]*matrix->m[2][2]) / det;
    result->m[1][1] = (- matrix->m[0][2]*matrix->m[2][0] 
		       + matrix->m[0][0]*matrix->m[2][2]) / det;
    result->m[1][2] = (+ matrix->m[0][2]*matrix->m[1][0] 
		       - matrix->m[0][0]*matrix->m[1][2]) / det;

    result->m[2][0] = (- matrix->m[1][1]*matrix->m[2][0] 
		       + matrix->m[1][0]*matrix->m[2][1]) / det;
    result->m[2][1] = (+ matrix->m[0][1]*matrix->m[2][0] 
		       - matrix->m[0][0]*matrix->m[2][1]) / det;
    result->m[2][2] = (- matrix->m[0][1]*matrix->m[1][0] 
		       + matrix->m[0][0]*matrix->m[1][1]) / det;
}