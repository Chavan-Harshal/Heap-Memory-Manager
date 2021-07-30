#ifndef HMM_H_INCLUDED
#define HMM_H_INCLUDED
#include<stdbool.h>
#include<stddef.h>

// Priority list node
typedef struct list_node{

	struct list_node *left;
	struct list_node *right;
}list_node;

// Structure to store memory block details
typedef struct block_info{

	bool is_free;
	size_t size;
	size_t offset;
	struct list_node n;
	struct  block_info *next, *prev;
}block_info;

// Structure to connect virtual memory pages
typedef struct vm_page{

	struct node *struct_info;
	struct  vm_page *next, *prev;
	struct block_info b;
	char page_memory[0];
}vm_page;

// Structure to store structure information
typedef struct node{
    char name[32];
    size_t size;
    struct vm_page *vm_page_head;
    struct list_node list_head;
}node;

// Connects pages containing structure information
typedef struct page_of_node{
    node arr[63];
    struct page_of_node* next;
}page_of_node;



void* req_vm_page(int no_of_pages);
int return_vm_page(void **page, size_t size);
bool isEmpty_vm_page(vm_page *v);
void mark_vm_page_empty(vm_page *v);
size_t max_memory(int units);
vm_page* allocate_vm_page(node *n);
vm_page* allocate_valid_block(struct node *n, int req_size, struct block_info **b);
vm_page* add_new_page(node* n);
void init(char* name, int size);
void* mycalloc(char *name, int units);
vm_page* get_next_vm_page_in_heap(vm_page* v, char c);
void delete_vm_page(vm_page *v);
void return_vm_page_to_heap(vm_page *v);
block_info* free_blocks(block_info *b);
void myfree(void **data);
void print_usage();


// block_info.h
void* get_page_ptr( block_info *b);
block_info* get_next_block( block_info *b);
block_info* get_next_block_by_size( block_info *b);
void bind_blocks_allocation(struct block_info *allocated,  block_info *_free);
void bind_blocks_deallocation(struct block_info *top, block_info *down);
block_info* get_prev_block( block_info *b);
void merge_free_blocks(block_info *b1,  block_info *b2);
int compare_free_blocks(block_info *b1,  block_info *b2);
void list_add_free_block(node *n,  block_info *b);
struct  block_info* get_biggest_free_block( node *n);


void insert(page_of_node** head, char *name, size_t size);
node* search_for_struct(page_of_node *head, char* name);

#define MAX_INFOS    \
    ((size_t)sysconf(_SC_PAGE_SIZE) - sizeof(page_of_node*))/sizeof(node)


// list.h
void init_list_node(list_node *n);
void list_add_next(list_node *curr, list_node *nn);
void list_add_before(list_node **head, list_node *curr, list_node *nn);
void list_add_last(list_node **head,  list_node *nn);
block_info* get_block_info(list_node *n, int offset);
void list_insert(list_node *head, list_node *nn, int offset);
void list_remov(list_node *n);
block_info* list_node_to_block(list_node *n);


#endif // HMM_H_INCLUDED
