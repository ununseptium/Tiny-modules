#include <stdio.h>
#include <stdlib.h>
#include "linked list.h"
#include <stdbool.h>

int main(void){

	linked_list *ll = create_linked_list();

	int a, b, c, d, e;
	a = 0;
	b = 1;
	c = 2;
	d = 3;
	e = 4;

	append(ll, &a);
	append(ll, &b);
	append(ll, &d);
	append(ll, &e);

	insert(ll, &c, 2);


	free_linked_list(ll);

	for(int i = 0; i<ll->size; ++i){
		printf("@%d\n", *(int*)get(ll, i));
	}

	return 0;
}
