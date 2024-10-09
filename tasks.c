#include "functional.h"
#include "tasks.h"
#include "tests.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX 100

// REVERSE

void inverse_list(void *acc, void *elem) {
	array_t *inv_list = (array_t *)acc;

	// shift the elements to the right to insert the current elem
	memmove(inv_list->data + inv_list->elem_size, inv_list->data,
			inv_list->len * inv_list->elem_size);

	// insert the new one on first position
	memcpy(inv_list->data, elem, inv_list->elem_size);

	// increase the length
	inv_list->len++;
}

array_t reverse(array_t list) {
	array_t inversed_list = (array_t){NULL, list.elem_size, 0,
									  list.destructor};
	inversed_list.data = malloc(list.len * list.elem_size);
	reduce(inverse_list, &inversed_list, list);
	return inversed_list;
}

// CREATE_NUMBER_ARRAY

void destructor_number_t(void *elem) {
	number_t element = *((number_t *)elem);
	free(element.string);
}

void create_number_t(void *new_elem, void **parts) {
	int integer_part = *((int *)parts[0]);
	int fractional_part = *((int *)parts[1]);
	number_t num;
	num.integer_part = integer_part;
	num.fractional_part = fractional_part;
	num.string = malloc(20 * sizeof(char));
	sprintf(num.string, "%d.%d", integer_part, fractional_part);
	memcpy(new_elem, &num, sizeof(number_t));
}

array_t create_number_array(array_t integer_part, array_t fractional_part) {
	int new_list_elem_size = sizeof(number_t);
	array_t list = map_multiple(create_number_t,
								new_list_elem_size, destructor_number_t, 2,
								integer_part, fractional_part);
	return list;
}

// GET_PASSING_STUDENTS_NAME

boolean if_stud_passed(void *student) {
	student_t stud = *((student_t *)student);
	if (stud.grade >= 5.00)
		return true;
	return false;
}

void string_destructor(void *elem) {
	free(*(char **)elem);
}

void extract_name(void *new_elem, void *old_elem) {
	student_t old_stud = *((student_t *)old_elem);
	*(char **)new_elem = (char *)malloc(MAX * sizeof(char));
	strcpy(*(char **)new_elem, old_stud.name);
}

array_t get_passing_students_names(array_t list) {
	// list with the students who passed
	array_t filtered_list = filter(if_stud_passed, list);

	// get the list only with the name
	array_t final_list = map(extract_name, MAX * sizeof(char),
							 string_destructor, filtered_list);

	return final_list;
}

// CHECK_BIGGER_SUM

void simple_list_destructor(void *elem) {
	free(((array_t *)elem)->data);
}

void do_sum(void *acc, void *elem) {
	*((int *)acc) += *((int *)elem);
}

void reduce_list_list(void *new_elem, void *old_elem) {
	array_t array = *((array_t *)old_elem);
	*((int *)new_elem) = 0;
	new_elem = reduce(do_sum, new_elem, array);
}

void check_if_equal(void *new_elem, void **old_elements) {
	int sum = *((int *)old_elements[0]);
	int int_to_compare_to = *((int *)old_elements[1]);
	if (sum >= int_to_compare_to)
		*(boolean *)new_elem = true;
	else
		*(boolean *)new_elem = false;
}

array_t check_bigger_sum(array_t list_list, array_t int_list) {
	array_t sum_list = map(reduce_list_list, sizeof(array_t),
						   NULL, list_list);

	array_t final_list = map_multiple(check_if_equal, sizeof(boolean),
									  NULL, 2, sum_list,
									  int_list);
	return final_list;
}

// GET EVEN INDEXED STRINGS

boolean check_if_null(void *elem) {
	if (*(int *)elem != 0)
		return true;
	return false;
}

void create_index_list(void *acc, void *list_elem) {
	array_t *index_list = (array_t *)acc;

	// get the address where to put the index in the index_list
	void *elem_to_modify = (index_list)->data +
						   (index_list)->len * (index_list)->elem_size;
	char *new_elem = malloc(MAX * sizeof(char));
	if (index_list->len % 2  == 0) {
		strcpy(new_elem, *(char **)list_elem);
		*(char **)elem_to_modify = new_elem;
	} else {
		free(new_elem);
	}

	// increase the length of the list created
	(index_list)->len++;
}

array_t get_even_indexed_strings(array_t list) {
	// initiate list where to keep even index names and the rest null
	array_t even_indexed_list = (array_t){NULL, MAX * sizeof(char), 0,
								string_destructor};
	even_indexed_list.data = (char *)calloc(list.len, MAX * sizeof(char));

	// create the list with only the specific indexed names, the rest are null
	reduce(create_index_list, &even_indexed_list, list);

	// filter and keep the ones != NULL
	array_t final_list = filter(check_if_null, even_indexed_list);

	// free initial list, as reduce doesn't
	for_each(string_destructor, list);
	free(list.data);

	return final_list;
}

// GENERATE_SQUARE_MATRIX

void create_vector_of_indexes(void *acc, void *elem) {
	int *curr_index = (int *)acc;
	int *to_put_index = (int *)elem;
	*to_put_index = *curr_index;
	(*curr_index)++;
}

void create_subvector(void *acc, void *elem) {
	int *value = (int *)acc;
	int *elem_from_subvector = (int *)elem;
	*elem_from_subvector = *value;
	(*value)++;
}

void create_matrix(void *acc, void *ptr_index) {
	array_t *matrix = (array_t *)acc;
	int index = *(int *)ptr_index;
	int n = matrix->len;

	// create each line of the matrix (from index)
	array_t subvector = (array_t){NULL, sizeof(int), n, NULL};
	subvector.data = malloc(n * sizeof(int));
	int start_value = index + 1;
	reduce(create_subvector, &start_value, subvector);

	// put subvector in the final matrix at the current index
	((array_t *)(matrix->data))[index] = subvector;
}

array_t generate_square_matrix(int n) {
	// create list of indexes
	int index[n];
	array_t vector_of_indexes = (array_t){index, sizeof(int), n, NULL};
	int curr_index = 0;
	reduce(create_vector_of_indexes, &curr_index, vector_of_indexes);

	// create final matrix
	array_t matrix = (array_t){NULL, sizeof(array_t), n,
							   simple_list_destructor};
	matrix.data = malloc(n * sizeof(array_t));
	reduce(create_matrix, &matrix, vector_of_indexes);

	return matrix;
}
