typedef struct node{
	struct node *next_node;
	const void *value;
}node;

node *create_node(node *next_node,const void *value, size_t size){
	node *n = malloc(sizeof(node));
	n->next_node = next_node;
	void *v = malloc(size);
	
	
	for(size_t i = 0; i<size; i++){
		*(char*)(v+i) = *(char*)(value+i);
	}
	
	n->value = v;
	return n;
}
