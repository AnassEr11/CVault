#include <CVault/utils/data_structure_utils.h>
#include <stdlib.h>
#include <time.h>

static DLinkedList *dlinked_list_delete_tail_helper(DLinkedList* wrapper);
static DLinkedList *dlinked_list_delete_first_helper(DLinkedList *wrapper);
static DLinkedList *dlinked_list_delete_middle_helper(DLinkedList *wrapper, DLinkedListNode *node);

DLinkedList *dlinked_list_create(void) {
    DLinkedList *wrapper = malloc(sizeof(DLinkedList));
    if (!wrapper) {
        return NULL;
    }

    wrapper->head = NULL;
    wrapper->tail = NULL;
    wrapper->size = 0;

    return wrapper;
}

DLinkedList *dlinked_list_push_back_node(DLinkedList *wrapper, void *data) {
    if (!wrapper) {
        return NULL;
    }

	DLinkedListNode *node = malloc(sizeof(DLinkedListNode));
	if(!node){
		return NULL;
	}

	node->next = NULL;
	node->prev = wrapper->tail;
	node->data = data;

	if(dlinked_list_is_empty(wrapper)){
		wrapper->head = node;
		wrapper->tail = node;
	}else {
		wrapper->tail->next = node;
		wrapper->tail = node;
	}

	wrapper->size++;
	return wrapper;
}

DLinkedList *dlinked_list_delete_node(DLinkedList *wrapper, DLinkedListNode *node){
	if (!dlinked_list_create() || dlinked_list_is_empty(wrapper)){
		return NULL;
	}
	if(!node){
		return NULL;
	}

	if(wrapper->head == node){
		return dlinked_list_delete_first_helper(wrapper);
	}
	if(wrapper->tail == node) {
		return dlinked_list_delete_tail_helper(wrapper);
	}
	if (node->next && node->prev) {
		return dlinked_list_delete_middle_helper(wrapper,node);
	}

	return NULL;
}

bool dlinked_list_is_empty(DLinkedList *wrapper) {
    return  !wrapper->size;
}

bool dlinked_list_is_created(DLinkedList *wrapper){
	return wrapper;
}

static void noop(void*p){
	(void)p;
}

void dlinked_list_destroy(DLinkedList *wrapper, void (*destroy_data)(void*)) {
	if(!dlinked_list_is_created(wrapper)){
		return;
	}

	if(!destroy_data){
		destroy_data = noop;
	}

    DLinkedListNode *iterator = wrapper->head;
    void *tmp;

	while (iterator) {
		destroy_data(iterator->data);
		tmp = iterator;
		iterator = iterator->next;
		free(tmp);
	}

    free(wrapper);
}

static DLinkedList *dlinked_list_delete_first_helper(DLinkedList *wrapper){

	DLinkedListNode* tmp = wrapper->head;
	wrapper->head = wrapper->head->next;
	free(tmp);

	if(wrapper->head == NULL) {
		wrapper->tail = NULL;
		wrapper->size = 0;
		return wrapper;
	}

	wrapper->head->prev = NULL;
	wrapper->size--;
	return wrapper;
}
static DLinkedList *dlinked_list_delete_tail_helper(DLinkedList* wrapper){

	DLinkedListNode* tmp = wrapper->tail;
	wrapper->tail = wrapper->tail->prev;
	free(tmp);

	if(wrapper->tail == NULL){
		wrapper->head = NULL;
		wrapper->size = 0;
		return wrapper;
	}

	wrapper->tail->next = NULL;
	wrapper->size--;
	return wrapper;
}

static DLinkedList *dlinked_list_delete_middle_helper(DLinkedList *wrapper, DLinkedListNode *node){

	DLinkedListNode *tmp = node;

	node->prev->next = node->next;
	node->next->prev = node->prev;

	free(tmp);
	wrapper->size--;
	return wrapper;
}
