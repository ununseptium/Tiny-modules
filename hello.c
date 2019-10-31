#include <stdio.h>
#include <stdlib.h>
#include "linked list.h"
#include <stdbool.h>

void* foo(const void *i){
	int *a = malloc(sizeof(int));
	*a = *(int*)i +100;
	return a;
	
}

int main(void){

	linked_list *ll = create_linked_list();

	int a, b, c, d, e;
	a = 0;
	b = 1;
	c = 2;
	d = 3;
	e = 4;

	append(ll, &a, sizeof(int));
	append(ll, &b, sizeof(int));
	append(ll, &d, sizeof(int));
	append(ll, &e, sizeof(int));

	insert(ll, 2, &c, sizeof(int));
	set(ll, 4, &a, sizeof(int));
	
	
	linked_list *new_ll = map(ll, foo, sizeof(int));
	remove_at(new_ll, 0);
	
	for(int i = 0; i<new_ll->size; ++i){
		printf("@%d\n", *(int*)get(new_ll, i));
	}
	
	
	free_linked_list(ll);
	free_linked_list(new_ll);

	return 0;
}
