#include<stdio.h>
#include <stdbool.h>
#include<limits.h>
#include<stddef.h>
#include<unistd.h>
#include <assert.h>
#include<sys/mman.h>
#include<memory.h>
#include "HMM.h"


static page_of_node *page_head = NULL;
static void* start = NULL;

// Function to request VM pages from kernel
void* req_vm_page(int no_of_pages){

    size_t size = (size_t)no_of_pages * (size_t)sysconf(_SC_PAGE_SIZE);

    char* page = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON, 0, 0);

    // If mmap fails
    if(page == (void*)(-1))
        return NULL;

    return (void*)page;
}

// Function to return VM pages to the kernel
int return_vm_page(void **page, size_t size){
    int temp = munmap(*page, size);

    // If munmap fails, return 0
    if(temp != 0)
        return 0;
    *page = NULL;
    return 1;
}

// Function to check vm page is empty or not
bool isEmpty_vm_page(vm_page *v){

    if(v->b.next == NULL && v->b.prev == NULL &&
        v->b.is_free == true)
        return true;
    return false;
}

// Function to initialise vm page as empty 
void mark_vm_page_empty(vm_page *v){

    v->b.next = v->b.prev = NULL;
    v->b.is_free = true;
    return;
}

// function to return maximum allocatable memory
size_t max_memory(int units){

    return (size_t)((size_t)sysconf(_SC_PAGE_SIZE) * units - offsetof(vm_page, page_memory));
}

// Function to insert structure info in the vm page
void insert(page_of_node** head, char *name, size_t size){

    page_of_node *new_page = NULL;

    size_t page_size = (size_t)sysconf(_SC_PAGE_SIZE);

    // If size of structure is more than size of VM page
    if(size > page_size){
        printf("Error\n");
        return;
    }

    // If head pointer is NULL
    if(!(*head)){

        (*head) = (page_of_node*)req_vm_page(1);
        (*head)->next = NULL;

        strncpy((*head)->arr[0].name, name, 32);
        (*head)->arr[0].size = size;
        start = (void*)(&((*head)->arr[0]) + sysconf(_SC_PAGE_SIZE));

        init_list_node(&((*head)->arr[0].list_head));

        return;
    }

    // Traverse in VM page
    size_t count = 0;
    while(count != MAX_INFOS && (*head)->arr[count].size != 0){
            count ++;
    }

    // If VM page is full
    if(count == MAX_INFOS){
        // Request for one more VM page
        new_page = (page_of_node*)req_vm_page(1);
        new_page->next = *head;
        *head = new_page;
        strncpy((*head)->arr[0].name, name, 32);
        (*head)->arr[0].size = size;
        start = (void*)(&((*head)->arr[0]) + sysconf(_SC_PAGE_SIZE));
        init_list_node(&((*head)->arr[0].list_head));
        return;
    }
    strncpy((*head)->arr[count].name, name, 32);
    (*head)->arr[count].size = size;
    start = (void*)(&((*head)->arr[count]) + sysconf(_SC_PAGE_SIZE));
    init_list_node(&((*head)->arr[count].list_head));
    return;
}

// Function to search for structure in vm page
node* search_for_struct(page_of_node *head, char* name){

    if(!head)
        return NULL;
    int i = 0;
    while(i != MAX_INFOS && (head)->arr[i].size != 0){

        if(strcmp(head->arr[i].name, name) == 0)
            return &(head->arr[i]);
        i++;
    }
    return NULL;
}

// Function to allocate vm page
vm_page* allocate_vm_page(node *n){

    vm_page* v = (vm_page*)req_vm_page(1);

    // Initilise the page with block of block_info
    mark_vm_page_empty(v);

    v->b.size = max_memory(1);
    v->b.offset = offsetof(struct vm_page, b);
    v->next = v->prev = NULL;
    init_list_node(&(v->b.n));

    // Set the struct_info pointer
    v->struct_info = n;

    // If head is NULL
    if(!n->vm_page_head)
        n->vm_page_head = v;

    // Insert page at the beginning of linked list
    else{
        v->next = n->vm_page_head;
        n->vm_page_head->prev = v;
    }

    return v;
}

// Function to delete vm page 
void delete_vm_page(vm_page *v){

    node *n = v->struct_info;

    assert(n->vm_page_head);
    if(n->vm_page_head == v){

        n->vm_page_head = v->next;
        if(v->next)
            v->next->prev = NULL;
        return_vm_page_to_heap(v);
        return;
    }

    if(v->next)
        v->next->prev = v->prev;
    v->prev->next = v->next;

    return_vm_page_to_heap(v);

    return;
}

// Function to allocate free block
bool allocate_free_block(node *n, block_info *b, int size){

    if(!b->is_free)
        return false;

    if(b->size < size)
        return false;

    int rem_size = b->size - size;

    b->is_free = false;
    b->size = size;

    list_remov(&(b->n));

    if(!rem_size)
        return true;

    if(rem_size < (sizeof(block_info) + n->size))
        return true;

    block_info *next = NULL;

    next = get_next_block_by_size(b);

    next->is_free = true;
    next->size = rem_size - sizeof(block_info);

    next->offset = b->offset + sizeof(block_info) + b->size;
    init_list_node(&(next->n));

    bind_blocks_allocation(b, next);
    list_add_free_block(n, next);

    return true;
}

// Function to add new vm page 
vm_page* add_new_page(node* n){

    vm_page *nn = allocate_vm_page(n);

    if(!nn)
        return NULL;

    list_add_free_block(n, &(nn->b));

    return nn;
}

// Function to allocate valid block
vm_page* allocate_valid_block(node *n, int req_size, block_info **b){

    bool status = false;
    vm_page *v = NULL;

    block_info *biggest = get_biggest_free_block(n);

    if(!biggest || biggest->size < req_size){

        v = add_new_page(n);

        status = allocate_free_block(n, &(v->b), req_size);

        if(!status){
            *b = NULL;
            delete_vm_page(v);
            return NULL;
        }
        *b = &(v->b);
        return v;
    }
    status = allocate_free_block(n, biggest, req_size);
    if(!status){

        *b = NULL;
        return NULL;
    }

    *b = biggest;

    return ((vm_page*)(biggest - biggest->offset));
}

// Function to register given structure
void init(char* name, int size){

    insert(&page_head, name, size);
    return;
}

// my calloc function
void* mycalloc(char *name, int units){


    node* n = search_for_struct(page_head, name);

    if(!n){

        printf("Init the %s structure first\n", name);
        return NULL;
    }

    if(units * n->size > max_memory(1)){
        printf("Error : Memory Requested Exceeds Page Size\n");
        return NULL;
    }

    if(!n->vm_page_head){
        n->vm_page_head = add_new_page(n);

        if(allocate_free_block(n, &(n->vm_page_head->b), units*n->size)){

            memset((char*)n->vm_page_head->page_memory, 0, units*n->size);
            return(void*)n->vm_page_head->page_memory;
        }
    }

    block_info *free_block;

    allocate_valid_block(n, units*n->size, &free_block);

    if(free_block){

        if(free_block->is_free == true){
            assert(0);
        }
        memset((char*)(free_block+1), 0, free_block->size);
        return (void*)(free_block+1);
    }
    return NULL;
}

// Function to return next or prev vm page pointer
vm_page* get_next_vm_page_in_heap(vm_page* v, char c){

    return ((c == '+') ? ((vm_page*)((char*)v + sysconf(_SC_PAGE_SIZE))) : ((vm_page*)((char*)v - sysconf(_SC_PAGE_SIZE))));
}

// Function to return vm page
void return_vm_page_to_heap(vm_page *v){

    mark_vm_page_empty(v);

    if((void*)v != ((char*)sbrk(0) - sysconf(_SC_PAGE_SIZE)))
        return;

    vm_page *bottom = NULL;

    for(bottom = get_next_vm_page_in_heap(v, '-'); isEmpty_vm_page(bottom); bottom = get_next_vm_page_in_heap(v, '-')){

        if((void*)bottom == start)
            break;
    }

    if((void*)bottom != start)
        bottom = get_next_vm_page_in_heap(bottom, '+');

    assert(!brk((void*)bottom));
}

// Function to free the allocated block
block_info* free_blocks(block_info *b){

    block_info *temp = NULL;

    assert(b->is_free == false);

    vm_page *host = get_page_ptr(b);

    b->is_free = true;

    temp = b;

    block_info *next = get_next_block(b);

    if(next && next->is_free == true){
        merge_free_blocks(b, next);
        temp = b;
    }

    block_info *prev = get_prev_block(b);

    if(prev && prev->is_free == true){
        merge_free_blocks(prev, b);
        temp = prev;
    }
    if(isEmpty_vm_page(host)){
        delete_vm_page(host);
        return NULL;
    }

    list_add_free_block(host->struct_info, temp);
    return temp;
}

// myfree function
void myfree(void **data){

    block_info *b = (block_info*)((char*)(*data) -sizeof(block_info));

    if(b->is_free == true){
        printf("Already freed\n");
        assert(0);
    }
    free_blocks(b);
    *data = NULL;
    return;
}

// Function to print the memory usage
void print_usage(){
    
    int i = 0, memory;
    while(page_head->arr[i].size != 0){
    	printf("\nStructure name : %s\n", page_head->arr[i].name);
    	vm_page *temp = page_head->arr[i].vm_page_head;
    	memory = 0;
    	
    	while(temp){

    		printf("Starting address : %p\n", page_head->arr[0].vm_page_head->page_memory);
    		block_info *temp1 = &(temp->b);

    		while(temp1->next){

    			memory += temp1->size;
    			temp1 = temp1->next;

    		}
    		temp = temp->next;
    	}
    	printf("Memory : %d\n", memory);
    	i++;
	}

}
