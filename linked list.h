#include "node.h"
#include <stdbool.h>

typedef struct{
	node *first_node;
	node *last_node;
	int size;
} linked_list;


linked_list *create_linked_list(void){
	linked_list *space = malloc(sizeof(linked_list));
	space->first_node = NULL;
	space->last_node = NULL;
	space->size = 0;
	return space;
}

void free_linked_list(linked_list *ll){
	node *n0 = ll->first_node;
	node *n1 = n0->next_node;
	for (int i = 0; i < ll->size-1; i++){
		free(n0);
		n0 = n1;
		n1 = n0->next_node;
	}
	free(n1);
	free(ll);
}

void append(linked_list* ll, const void *value){
	if(ll->first_node == NULL && ll->last_node == NULL){
		node *n = create_node(NULL, value);
		ll->first_node = n;
		ll->size = 1;
	}
	else if(ll->last_node == NULL){
		node *n = create_node(NULL, value);
		ll->first_node->next_node = n;
		ll->last_node = n;
		ll->size = 2;
	}
	else{
		node *n = create_node(NULL, value);
		(*ll->last_node).next_node = n;
		ll->last_node = n;
		ll->size++;
	}
}

void insert(linked_list* ll,const void *value, int index){
	if(index == 0){
		node *n = create_node(ll->first_node, value);
		ll->first_node = n;
		ll->size++;
	}
	else if(index == ll->size){
		append(ll, value);
	}
	else{
		node *current_node = ll->first_node;
		for(int index_node = 0; index_node < index - 1; index_node++){
			current_node = current_node->next_node;
		}

		node *prenode = current_node;
		node *postnode = current_node->next_node;
		node *n = create_node(postnode, value);

		prenode->next_node = n;
		ll->size++;
	}
}


const void* get(linked_list *ll, int index){

	if(index == 0){
		return (*ll->first_node).value;
	}

	if(index == ll->size-1){
		return (*ll->last_node).value;
	}

	node *p_current_node = ll->first_node;
	for(int node_index = 0; node_index < index; node_index++){
		p_current_node = (p_current_node->next_node);
	}

	return (*p_current_node).value;
}

void set(linked_list *ll, int index, const void *value){
	if(index == 0){
		(*ll->first_node).value = value;
	}

	if(index == (*ll->last_node).value){
		(*ll->last_node).value = value;
	}

	node *p_current_node = ll->first_node;
	for(int node_index = 0; node_index < index; node_index++){
		p_current_node = (p_current_node->next_node);
	}

	(*p_current_node).value = value;
}


void removeAt(linked_list *ll, int index){
	if(index == 0){
		node *n = ll->first_node->next_node;
		free(ll->first_node);
		ll->first_node = n;
	}
	else if(index == ll->size-1){
		node *n = ll->first_node;
		free(ll->last_node);

		for (int i = 0; i < index-1; i++) {
			n = n->next_node;
		}
		n->next_node = NULL;

		ll->last_node = n;

	}
	else{
		node *prenode = ll->first_node;
		for(int node_index = 0; node_index<index-1; node_index++){
			prenode = prenode->next_node;
		}

		prenode->next_node = prenode->next_node->next_node;
		free(prenode->next_node);


	}
	ll->size--;
}

linked_list *filter(linked_list *ll, bool (*lambda)(void*)){
	linked_list *new_ll = create_linked_list();

	for(int index_node = 0; index_node < ll->size; index_node++){
		if( (*lambda)(get(ll, index_node)) ){
			append(new_ll, get(ll, index_node));
		}
	}

	return new_ll;
}

linked_list *map(linked_list *ll, int (*lambda)(void*)){
	linked_list *new_ll = create_linked_list();

	for(int index_node = 0; index_node < ll->size; index_node++){
		append(new_ll, (*lambda)(get(ll, index_node)));
	}

	return new_ll;
}
