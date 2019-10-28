typedef struct node{
	struct node *next_node;
	int value;
}node;

node *create_node(node *next_node, int value){
	node *n = malloc(sizeof(node));
	n->next_node = next_node;
	n->value = value;
	return n;
}
