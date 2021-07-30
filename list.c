#include<stdio.h>
#include "HMM.h"

// Function to init the list node
void init_list_node(list_node *n){

	n->left = n->right = NULL;
	return;
}

// Add node next to current node
void list_add_next(list_node *curr, list_node *nn){

	if(!curr->right){

		curr->right = nn;
		nn->left = curr;
		return;
	}

	struct list_node *temp = curr->right;
	curr->right = nn;
	nn->left = curr;
	nn->right = temp;
	temp->left = nn;
	return;
}

// Get block_info pointer from list node
block_info* get_block_info(list_node *n, int offset){

	return (block_info*)((char*)n - offset);
}

// Function to insert node at appropriate position
void list_insert(list_node *head, list_node *nn, int offset){

	init_list_node(nn);


	if(!(head)->left && !(head)->right){

		list_add_next(head, nn);
		return;
	}

	if(head->right && !(head->right->right)){

		if(compare_free_blocks(get_block_info(head->right, offset), get_block_info(nn, offset))==-1)
			list_add_next(head->right, nn);
		else
			list_add_next(head, nn);
		return;
	}

	if (compare_free_blocks(get_block_info(nn, offset), get_block_info(head->right, offset)) == -1){
		
		list_add_next(head, nn);
		return;
	}

	list_node *curr = head->right;
	list_node *prev;

	while(curr && compare_free_blocks(get_block_info(nn, offset), get_block_info(curr, offset)) != -1){

		curr = curr->right;
		prev = curr;

	}
	if(!curr)
		list_add_next(prev, nn);
	else
		list_add_next(curr, nn);
	return;

}

// Function to remove node from list
void list_remov(list_node *n){

	if(!n->left){
		if(n->right){
			n->right->left = NULL;
			n->right = NULL;
			return;
		}
		return;
	}
	if(!n->right){
		n->left->right = NULL;
		n->left = NULL;
		return;
	}
	n->left->right = n->right;
	n->right->left = n->left;
	n->left = NULL;
	n->right = NULL;
	return;
}


block_info* list_node_to_block(list_node *n){

	return (block_info*)((char*)n - offsetof(block_info, n));
}