typedef struct node{
	struct node *next_node;
	void *value;
}node_t;

node_t *create_node(node_t *next_node,const void *value, size_t size){
	node_t *n = malloc(sizeof(node_t));
	n->next_node = next_node;
	void *v = malloc(size);
	
	
	for(size_t i = 0; i<size; i++){
		*(char*)(v+i) = *(char*)(value+i);
	}
	
	n->value = v;
	return n;
}
