#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>
#include "HMM.h"

// function to get page_pointer
void* get_page_ptr(block_info *b){
	if(!b)
		return NULL;
	return (void*)((char*)b - b->offset);
}

// Function to return next block
block_info* get_next_block(block_info *b){
	if(!b || !(b->next))
			return NULL;
	return b->next;
}

// Function to return next block by size
struct block_info* get_next_block_by_size(block_info *b){
	if(!b)
		return NULL;

	return(block_info*)((char*)(b + 1) + b->size);
}

// Function to return prev block
struct block_info* get_prev_block(block_info *b){
	if(!b || !(b->prev))
		return NULL;

	return b->prev;
}

// Bind blocks for allocation
void bind_blocks_allocation(struct block_info *allocated, block_info *_free){
	if(!allocated || !_free)
		return;

	_free->prev = allocated;
	_free->next = allocated->next;
	allocated->next = _free;

	if(_free->next)
		_free->next->prev = _free;
	return;
}

// Bind blocks for deallocation
void bind_blocks_deallocation(struct block_info *top, block_info *down){

	top->next = down->next;
	if(down->next)
		down->next->prev = top;
	return;
}

// Function to merge two blocks
void merge_free_blocks(struct block_info *b1, block_info *b2){

	if(!b1 || !b2 || !(b1->is_free) || !(b2->is_free))
		return;

	b1->size += sizeof(block_info) + b2->size;
	

	list_remov(&b1->n);
	list_remov(&b2->n);
	bind_blocks_deallocation(b1, b2);
	return;
}

// Function to compare two blocks
int compare_free_blocks(block_info *b1, block_info *b2){

	if(b1->size > b2->size)
		return -1;

	if(b2->size > b1->size)
		return 1;

	return 0;
}

// Function to add free block to list
void list_add_free_block(node *n, block_info *b){

	if(!b->is_free)
		return;

	list_insert(&(n->list_head), &(b->n), offsetof(block_info, n));
}

// Function get biggest free block
struct  block_info* get_biggest_free_block(node *n){

	return list_node_to_block(&n->list_head);
}
