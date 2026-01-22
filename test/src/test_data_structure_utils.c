#include "argon2/core.h"
#include <CVault/utils/data_structure_utils.h>
#include <stdio.h>
#include <string.h>

#define COLOR_RESET  "\033[0m"
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_RED    "\033[0;31m"
#define COLOR_BLUE   "\033[34m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_CYAN   "\033[0;36m"

DLinkedList *wrapper;

bool test_create_list(){
	wrapper = dlinked_list_create();
	
	if(wrapper || !wrapper->head || !wrapper->tail || !wrapper->size){
		printf(COLOR_GREEN"-> doubly linked list created successfully\n"COLOR_RESET);
		return true;
	}else {
		printf(COLOR_RED"-> error in creating doubly liked list \n"COLOR_RESET);
		return false;
	}
}
bool test_push_node(){
	char* data1 = "A";
	char* data2 = "B";
	char* data3 = "C";

	dlinked_list_push_back_node(wrapper,data1);
	dlinked_list_push_back_node(wrapper,data2);
	dlinked_list_push_back_node(wrapper,data3);

	if(wrapper->size != 3){
		printf(COLOR_RED"-> size is not 3\n"COLOR_RESET);
		return false;
	}
	printf(COLOR_GREEN"-> size updated successfully\n"COLOR_RESET);
	printf("visualizing the linked list and checking the links : \n");

	char linked_list_forward[20] = "A -> B -> C -> NULL";
	char linked_list_backward[20] = "C -> B -> A -> NULL";
	char buffer[100] = "";

	DLinkedListNode *iterator = wrapper->head;
	while (iterator) {
		strcat(buffer,(char*)iterator->data);
		strcat(buffer," -> ");
		iterator = iterator->next;
	}
	strcat(buffer,"NULL");

	printf("forward: \n");
	if(strcmp(buffer,linked_list_forward) == 0){
		printf(COLOR_GREEN"-> SUCCESS: "COLOR_RESET"%s\n",buffer);
	}else {
		printf(COLOR_RED"-> FAILED: \n"COLOR_RESET);
		printf("Expected: %s\n",linked_list_forward);
		printf("Found: %s\n",buffer);
		return false;
	}

	memset(buffer,'\0',100);

	iterator = wrapper->tail;
	while (iterator) {
		strcat(buffer,(char*)iterator->data);
		strcat(buffer," -> ");
		iterator = iterator->prev;
	}
	strcat(buffer,"NULL");

	printf("backward: \n");
	if(strcmp(buffer,linked_list_backward) == 0){
		printf(COLOR_GREEN"-> SUCCESS: "COLOR_RESET"%s\n",buffer);
	}else {
		printf(COLOR_RED"-> FAILED: \n"COLOR_RESET);
		printf("Expected: %s\n",linked_list_backward);
		printf("Found: %s\n",buffer);
		return false;
	}

	return true;
}
bool test_delete_node(){
	if(wrapper->size != 3){
		printf(COLOR_RED"-> list size is not 3, cannot delete\n"COLOR_RESET);
		return false;
	}

	DLinkedListNode *to_delete = wrapper->head->next;
	dlinked_list_delete_node(wrapper, to_delete);

	if(wrapper->size != 2){
		printf(COLOR_RED"-> size is not 2 after deletion\n"COLOR_RESET);
		return false;
	}
	printf(COLOR_GREEN"-> node deleted successfully, size updated\n"COLOR_RESET);

	char buffer[100] = "";
	DLinkedListNode *iterator = wrapper->head;
	while (iterator) {
		strcat(buffer,(char*)iterator->data);
		strcat(buffer," -> ");
		iterator = iterator->next;
	}
	strcat(buffer,"NULL");

	char expected[20] = "A -> C -> NULL";
	if(strcmp(buffer, expected) == 0){
		printf(COLOR_GREEN"-> SUCCESS: "COLOR_RESET"%s\n",buffer);
	}else {
		printf(COLOR_RED"-> FAILED: \n"COLOR_RESET);
		printf("Expected: %s\n",expected);
		printf("Found: %s\n",buffer);
		return false;
	}

	return true;
}
bool test_destroy_node(){
	if(!dlinked_list_is_created(wrapper)){
		printf(COLOR_RED"-> list is not created\n"COLOR_RESET);
		return false;
	}

	dlinked_list_destroy(wrapper, NULL);

	printf(COLOR_GREEN"-> list destroyed successfully\n"COLOR_RESET);
	return true;
}

int main() {
	printf(COLOR_BLUE"\nTEST DATA STRUCTURE UTILS\n\n"COLOR_RESET);

	printf(COLOR_CYAN"\nDOUBLY LINKED LIST LOGIC\n\n"COLOR_RESET);
	printf("Test creation : \n");
	if(!test_create_list()) {
		printf(COLOR_RED"creation failed, couldn't continue\n"COLOR_RESET);
		return 1;
	}

	printf("\nTest pushing : \n");
	if(!test_push_node()){
		printf(COLOR_RED"pushing failed, couldn't continue\n"COLOR_RESET);
		return 1;
	}
	
	printf("\nTest deleting : \n");
	if(!test_delete_node()) {
		printf(COLOR_RED"delete failed\n"COLOR_RESET);
	}	
	
	printf("\nTest destroying : \n");
	if(!test_destroy_node()){
		printf(COLOR_RED"destroying failed\n"COLOR_RESET);
	}

	printf(COLOR_GREEN"\nAll tests passed!\n"COLOR_RESET);
	return 0;
}
