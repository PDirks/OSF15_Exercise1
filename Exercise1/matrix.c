#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <assert.h>

#include "matrix.h"


#define MAX_CMD_COUNT 50

/*protected functions*/
void load_matrix (Matrix_t* m, unsigned int* data);

/* 
 * PURPOSE: instantiates a new matrix with the passed name, rows, cols 
 * INPUTS: 
 *	name the name of the matrix limited to 50 characters 
 *  rows the number of rows the matrix
 *  cols the number of cols the matrix
 * RETURN:
 *  If no errors occurred during instantiation then true
 *  else false for an error in the process.
 *
 **/
bool create_matrix (Matrix_t** new_matrix, const char* name, const unsigned int rows, const unsigned int cols) {

	if( !new_matrix || !name  ){
		perror("create_matrix: bad input\n");
		return false;
	}

	*new_matrix = calloc(1,sizeof(Matrix_t));
	if (!(*new_matrix)) {
		return false;
	}
	(*new_matrix)->data = calloc(rows * cols,sizeof(unsigned int));
	if (!(*new_matrix)->data) {
		return false;
	}
	(*new_matrix)->rows = rows;
	(*new_matrix)->cols = cols;
	unsigned int len = strlen(name) + 1; 
	if (len > MATRIX_NAME_LEN) {
		return false;
	}
	strncpy((*new_matrix)->name,name,len);
	return true;

}// end create_matrix

/*
 * PURPOSE: deallocates passed matrix
 * INPUTS: 
 *      Matrix to deallocate
 * RETURN:
 *      void
 **/
void destroy_matrix (Matrix_t** m) {        
    if( *m ){
        free((*m)->data);
        free(*m);
        *m = NULL;
    }
}// end destory matrix

/*
 * PURPOSE: compare memory blocks of two matrices, to see if they are the same
 * INPUTS: 
 *      two matrices to compare
 * RETURN:
 *      if blocks of memory are the same, return true. Else, return false.
 **/
bool equal_matrices (Matrix_t* a, Matrix_t* b) {	
	if (!a || !b || !a->data || !b->data ) {
        perror("equal_matrices: bad input\n");
		return false;	
	}

	int result = memcmp(a->data,b->data, sizeof(unsigned int) * a->rows * a->cols);
	if (result == 0) {
		return true;
	}
	return false;
}

/*
 * PURPOSE: copy matrix to another memory block
 * INPUTS:
 *      source matrix src
 *      destination matrix dest
 * RETURN:
 *      If dest matrix the same as src, return true.
 *      Else, return false.
 **/
bool duplicate_matrix (Matrix_t* src, Matrix_t* dest) {
	if (!src || !dest || !src->data ) {
        perror("duplicate_matrix: bad input\n");
		return false;
	}
	/*
	 * copy over data
	 */
	unsigned int bytesToCopy = sizeof(unsigned int) * src->rows * src->cols;
	memcpy(dest->data,src->data, bytesToCopy);	
	return equal_matrices (src,dest);
}// end duplicate_matrix

/*
 * PURPOSE: Preform a bitwise shift on the members of an array
 * INPUTS:
 *		Direction the shift should move, direction
 *		Matrix to preform shift on, a
 *		Amount to shift by, shift
 * RETURN:
 *		If bad inputs are given, return false.
 *		Else, return true.
 **/
bool bitwise_shift_matrix (Matrix_t* a, char direction, unsigned int shift) {
	
	if ( !a || !a->data ) {
        perror("bitwise_shift_matrix: bad input\n");
		return false;
	}

	if (direction == 'l') {
		unsigned int i = 0;
		for (; i < a->rows; ++i) {
			unsigned int j = 0;
			for (; j < a->cols; ++j) {
				a->data[i * a->cols + j] = a->data[i * a->cols + j] << shift;
			}
		}

	}
	else {
		unsigned int i = 0;
		for (; i < a->rows; ++i) {
			unsigned int j = 0;
			for (; j < a->cols; ++j) {
				a->data[i * a->cols + j] = a->data[i * a->cols + j] >> shift;
			}
		}
	}
	
	return true;
}// end bitwise_shift_matrix

/*
 * PURPOSE: Add two matrices together
 * INPUTS: 
 *      1st matrix to add, a.
 *      2nd matrix to add, b.
 *      Destinaiton of the sum, c.
 * RETURN:
 *      If parameters are invalid, return false.
 *      Else, return true.
 **/
bool add_matrices (Matrix_t* a, Matrix_t* b, Matrix_t* c) {
	if ( !a || !b || !c || !a->data || !b->data || !c->data ) {
        perror("add_matrices: bad input\n");
		return false;
	}

	for (int i = 0; i < a->rows; ++i) {
		for (int j = 0; j < b->cols; ++j) {
			c->data[i * a->cols +j] = a->data[i * a->cols + j] + b->data[i * a->cols + j];
		}
	}
	return true;
}// end add_matrices

/*
 * PURPOSE: Print the contents of a matrix to the screen
 * INPUTS:
 *      Matrix to print, m
 * RETURN:
 *      void
 **/
void display_matrix (Matrix_t* m) {
    if( !m || !m->data ){
        perror("display_matrix: bad input");
        return;
    }
	printf("\nMatrix Contents (%s):\n", m->name);
	printf("DIM = (%u,%u)\n", m->rows, m->cols);
	for (int i = 0; i < m->rows; ++i) {
		for (int j = 0; j < m->cols; ++j) {
			printf("%u ", m->data[i * m->cols + j]);
		}
		printf("\n");
	}
	printf("\n");
}// end display_matrix

/*
 * PURPOSE: Load a matrix from a file
 * INPUTS:
 *      File containing the matrix, matrix_input_filename.
 *      Destrination for loaded matrix, m.
 * RETURN:
 *      If there is an error in reading the file, return false.
 *      Else, return true.
 **/
bool read_matrix (const char* matrix_input_filename, Matrix_t** m) {
    if( !matrix_input_filename || !m ){
        perror("read_matrix: bad input\n");
        return false;
    }

	int fd = open(matrix_input_filename,O_RDONLY);
	if (fd < 0) {
		printf("FAILED TO OPEN FOR READING\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}
		return false;
	}

	/*read the wrote dimensions and name length*/
	unsigned int name_len = 0;
	unsigned int rows = 0;
	unsigned int cols = 0;
	
	if (read(fd,&name_len,sizeof(unsigned int)) != sizeof(unsigned int)) {
		printf("FAILED TO READING FILE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}
		return false;
	}
	char name_buffer[50];
	if (read (fd,name_buffer,sizeof(char) * name_len) != sizeof(char) * name_len) {		// segfaults around here...
		printf("FAILED TO READ MATRIX NAME\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;	
	}

	if (read (fd,&rows, sizeof(unsigned int)) != sizeof(unsigned int)) {
		printf("FAILED TO READ MATRIX ROW SIZE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;
	}

	if (read(fd,&cols,sizeof(unsigned int)) != sizeof(unsigned int)) {
		printf("FAILED TO READ MATRIX COLUMN SIZE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;
	}

	unsigned int numberOfDataBytes = rows * cols * sizeof(unsigned int);
	unsigned int *data = calloc(rows * cols, sizeof(unsigned int));
	if (read(fd,data,numberOfDataBytes) != numberOfDataBytes) {
		printf("FAILED TO READ MATRIX DATA\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}

		return false;	
	}

	if (!create_matrix(m,name_buffer,rows,cols)) {
		return false;
	}

	load_matrix(*m,data);

    if( !m ){
            return false;
    }
        
	free(data);

	if (close(fd)) {
		return false;

	}
	return true;
}//end read_matrix

/*
 * PURPOSE: Write a matrix to a file
 * INPUTS:
 *      Destination file for the matrix, matrix_output_filename
 *      Matrix to write from, m
 * RETURN:
 *      If there is an error in writing to the file, return false.
 *      Else, return true.
 **/
bool write_matrix (const char* matrix_output_filename, Matrix_t* m) {
    if(!matrix_output_filename || !m || !m->name || !m->rows || !m->cols){
	    perror("write_matrix: bad input\n");
        return false;
    }

	int fd = open (matrix_output_filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
	/* ERROR HANDLING USING errorno*/
	if (fd < 0) {
		printf("FAILED TO CREATE/OPEN FILE FOR WRITING\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXISTS\n");
		}
		return false;
	}
	/* Calculate the needed buffer for our matrix */
	unsigned int name_len = strlen(m->name) + 1;
	unsigned int numberOfBytes = sizeof(unsigned int) + (sizeof(unsigned int)  * 2) + name_len + sizeof(unsigned int) * m->rows * m->cols + 1;
	/* Allocate the output_buffer in bytes
	 * IMPORTANT TO UNDERSTAND THIS WAY OF MOVING MEMORY
	 * --------------------------------------------------
	 * Here we create an output buffer big enough to hold all out-going memory (+ EOF)
	 */
	unsigned char* output_buffer = calloc(numberOfBytes,sizeof(unsigned char));
	unsigned int offset = 0;
	/*
	 * start copying data from the matrix into the output buffer
	 * increment the offset with each copy to prevent unwanted overwriting
	 */
	memcpy(&output_buffer[offset], &name_len, sizeof(unsigned int)); // IMPORTANT C FUNCTION TO KNOW
	offset += sizeof(unsigned int);	
	memcpy(&output_buffer[offset], m->name,name_len);
	offset += name_len;
	memcpy(&output_buffer[offset],&m->rows,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy(&output_buffer[offset],&m->cols,sizeof(unsigned int));
	offset += sizeof(unsigned int);
	memcpy (&output_buffer[offset],m->data,m->rows * m->cols * sizeof(unsigned int));
	offset += (m->rows * m->cols * sizeof(unsigned int));
	output_buffer[numberOfBytes - 1] = EOF;

	if (write(fd,output_buffer,numberOfBytes) != numberOfBytes) {
		printf("FAILED TO WRITE MATRIX TO FILE\n");
		if (errno == EACCES ) {
			perror("DO NOT HAVE ACCESS TO FILE\n");
		}
		else if (errno == EADDRINUSE ){
			perror("FILE ALREADY IN USE\n");
		}
		else if (errno == EBADF) {
			perror("BAD FILE DESCRIPTOR\n");	
		}
		else if (errno == EEXIST) {
			perror("FILE EXIST\n");
		}
		return false;
	}
	
	if (close(fd)) {
		return false;
	}
	free(output_buffer);

	return true;
}//end write_matrix

/*
 * PURPOSE: Allocates a matrix with random numbers
 * INPUTS:
 *      Matrix to populate, m
 *      Start range of matrix, start_range
 *      End range of matrix, end_range
 * RETURN:
 *      If there is an error in allocating the matrix, return false.
 *      Else, return true.
 **/
bool random_matrix(Matrix_t* m, unsigned int start_range, unsigned int end_range) {
    if( !m || start_range > end_range){
        perror("random_matrix: bad input\n");
        return false;
    }

	for (unsigned int i = 0; i < m->rows; ++i) {
		for (unsigned int j = 0; j < m->cols; ++j) {
			m->data[i * m->cols + j] = rand() % (end_range + 1 - start_range) + start_range;
		}
	}
	return true;
}//end random_matrix

/*Protected Functions in C*/

/*
 * PURPOSE: Translate the unsigned int into a Matrix
 * INPUTS:
 *      Matrix to populate, m
 *      Data to read from, data
 * RETURN:
 *      void
 **/
void load_matrix (Matrix_t* m, unsigned int* data) {
    if(!data || !m || !m->data ){
        perror("load_matrix: bad input\n");
        return;
    }

	memcpy(m->data,data,m->rows * m->cols * sizeof(unsigned int));
}//end load_matrix

/*
 * PURPOSE: 
 * INPUTS:
 *      The master-list of matrices, mats.
 *      The matrix to be added, new_matrix.
 *      The count of matrices in the master-list, num_mats
 * RETURN:
 *      If there is an error, return -1.
 *      Else, return the pos of the new matrix
 **/
unsigned int add_matrix_to_array (Matrix_t** mats, Matrix_t* new_matrix, unsigned int num_mats) {
	if( !mats || !new_matrix ){
		perror("add_matrix_to_array: bad input\n");
		return -1;
	}

	static long int current_position = 0;
	const long int pos = current_position % num_mats;
	if ( mats[pos] ) {
		destroy_matrix(&mats[pos]);
	} 
	mats[pos] = new_matrix;
	current_position++;
	return pos;
}// end add_matrix_to_array
