#include <stdio.h>
#include <stdlib.h>
#include "linked list.h"
#include <stdbool.h>


int f(int i){
	return i+10;
}

int main(void){

	linked_list *ll = create_linked_list();

	append(ll, 0);
	append(ll, 1);
	append(ll, 3);
	append(ll, 4);

	insert(ll, 2, 2);

	int (*func)(int) = &f;

	linked_list *new_ll = map(ll, func);

	for(int i = 0; i<new_ll->size; ++i){
		printf("@%d\n", get(new_ll, i));
	}

	return 0;
}
