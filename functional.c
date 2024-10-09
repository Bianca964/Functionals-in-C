#include "functional.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void for_each(void (*func)(void *), array_t list) {
	// iterate over each element of the array
	for (int i = 0; i < list.len; i++) {
		// calculate the memory address of the current element
		void *elem_addr = list.data + i * list.elem_size;
		// apply the given function to the current element
		func(elem_addr);
	}
}

array_t map(void (*func)(void *, void *),
			int new_list_elem_size,
			void (*new_list_destructor)(void *),
			array_t list) {
	array_t new_list;
	new_list.len = list.len;
	new_list.elem_size = new_list_elem_size;
	new_list.destructor = new_list_destructor;
	new_list.data = malloc(new_list.elem_size * new_list.len);
	if (!new_list.data)
		exit(-1);
	for (int i = 0; i < list.len; i++) {
		void *elem_addr_old_list = list.data + i * list.elem_size;
		void *elem_addr_new_list = new_list.data + i * new_list.elem_size;
		func(elem_addr_new_list, elem_addr_old_list);
	}

	// free the memory consumed by the old list
	if (list.destructor) {
		for (int i = 0; i < list.len; i++) {
			void *elem_addr = list.data + i * list.elem_size;
			list.destructor(elem_addr);
		}
	}
	free(list.data);

	return new_list;
}

array_t filter(boolean(*func)(void *), array_t list) {
	array_t new_list;
	new_list.elem_size = list.elem_size;
	new_list.destructor = list.destructor;
	new_list.len = 0;
	new_list.data = malloc(new_list.elem_size * list.len);
	if (!new_list.data)
		exit(-1);

	for (int i = 0; i < list.len; i++) {
		// calculate the memory address of the current element in the old list
		void *elem_addr = list.data + i * list.elem_size;
		// check if the current element satisfies the filter condition
		if (func(elem_addr)) {
			// memory address where the current element will be stored
			void *new_elem_addr = new_list.data +
								  new_list.len * new_list.elem_size;
			memcpy(new_elem_addr, elem_addr, new_list.elem_size);
			new_list.len++;
		} else if (list.destructor) {
			void *elem_addr = list.data + i * list.elem_size;
			list.destructor(elem_addr);
		}
	}

	// if the new list is smaller than the old list, reallocate memory
	if (new_list.len < list.len) {
		new_list.data = realloc(new_list.data,
								new_list.len * new_list.elem_size);
		if (!new_list.data)
			exit(-1);
	}
	free(list.data);

	return new_list;
}

void *reduce(void (*func)(void *, void *), void *acc, array_t list) {
	for (int i = 0; i < list.len; i++) {
		void *elem_addr = list.data + i * list.elem_size;
		func(acc, elem_addr);
	}
	return acc;
}

void for_each_multiple(void (*func)(void **), int varg_c, ...) {
	va_list args;
	va_start(args, varg_c);

	// find the minimum length of lists
	array_t list1 = va_arg(args, array_t);
	int min = list1.len;
	for (int i = 0; i < varg_c - 1; i++) {
		array_t list2 = va_arg(args, array_t);
		if (list2.len < min)
			min = list2.len;
	}
	va_end(args);

	for (int i = 0; i < min; i++) {
		// go over the lists to create my vector
		va_start(args, varg_c);
		void **my_vector = malloc(varg_c * sizeof(void *));
		// complete the vector with elements from the others lists
		for (int j = 0; j < varg_c; j++) {
			array_t list = va_arg(args, array_t);
			void *elem_from_list = list.data + i * list.elem_size;
			my_vector[j] = malloc(list.elem_size);
			memcpy(my_vector[j], elem_from_list, list.elem_size);
		}
		va_end(args);

		// apply func
		func(my_vector);

		// free memory for my_vector
		for (int j = 0; j < varg_c; j++)
			free(my_vector[j]);
		free(my_vector);
	}
}

array_t map_multiple(void (*func)(void *, void **),
					 int new_list_elem_size,
					 void (*new_list_destructor)(void *),
					 int varg_c, ...) {
	va_list args;
	va_start(args, varg_c);

	// find the minimum length of lists
	array_t list = va_arg(args, array_t);
	int min = list.len;
	for (int i = 0; i < varg_c - 1; i++) {
		list = va_arg(args, array_t);
		if (list.len < min)
			min = list.len;
	}
	va_end(args);

	array_t new_list;
	new_list.elem_size = new_list_elem_size;
	new_list.destructor = new_list_destructor;
	new_list.len = min;
	new_list.data = malloc(new_list.len * new_list.elem_size);

	for (int i = 0; i < min; i++) {
		// go over the lists to create my vector
		va_start(args, varg_c);
		void **my_vector = malloc(varg_c * sizeof(void *));
		// complete the vector with elements from the lists
		for (int j = 0; j < varg_c; j++) {
			list = va_arg(args, array_t);
			void *elem_from_list = list.data + i * list.elem_size;
			my_vector[j] = malloc(list.elem_size);
			memcpy(*(my_vector + j), elem_from_list, list.elem_size);
		}
		va_end(args);

		// apply func
		func(new_list.data + i * new_list.elem_size, my_vector);

		// free memory for my_vector
		for (int j = 0; j < varg_c; j++)
			free(my_vector[j]);
		free(my_vector);
	}

	// free old list
	va_start(args, varg_c);
	for (int i = 0; i < varg_c; i++) {
		list = va_arg(args, array_t);
		if (list.destructor) {
			for (int j = 0; j < list.len; j++)
				list.destructor(list.data + j * list.elem_size);
		}
		free(list.data);
	}

	return new_list;
}

void *reduce_multiple(void (*func)(void *, void **), void *acc,
					  int varg_c, ...) {
	va_list args;
	va_start(args, varg_c);

	// find the minimum length of lists
	array_t list1 = va_arg(args, array_t);
	int min = list1.len;
	for (int i = 0; i < varg_c - 1; i++) {
		array_t list2 = va_arg(args, array_t);
		if (list2.len < min)
			min = list2.len;
	}
	va_end(args);

	for (int i = 0; i < min; i++) {
		// go over the lists to create my vector
		va_start(args, varg_c);
		void **my_vector = malloc(varg_c * sizeof(void *));
		// complete the vector with elements from the others lists
		for (int j = 0; j < varg_c; j++) {
			array_t list = va_arg(args, array_t);
			void *elem_from_list = list.data + i * list.elem_size;
			my_vector[j] = malloc(list.elem_size);
			memcpy(my_vector[j], elem_from_list, list.elem_size);
		}
		va_end(args);

		// apply func
		func(acc, my_vector);

		// free memory for my_vector
		for (int j = 0; j < varg_c; j++)
			free(my_vector[j]);
		free(my_vector);
	}
	return acc;
}
