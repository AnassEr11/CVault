#ifndef DATA_STRUCTURE_UTILS_H
#define DATA_STRUCTURE_UTILS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct DLinkedListNode DLinkedListNode;

/**
 * @brief: Generic node structure for a doubly linked list
 */
struct DLinkedListNode {
    void *data;
    DLinkedListNode *next;
    DLinkedListNode *prev;
};

/**
 * @brief: Wrapper structure for a doubly linked list
 */
typedef struct {
    DLinkedListNode *head;
    DLinkedListNode *tail; /**< pointer to the tail for O(1) back puching/deleting */
    uint64_t size;
} DLinkedList;

/**
 * @brief: Creates a new doubly linked list
 *
 * @return: A pointer to a newly allocated DLinkedList, or NULL on failure
 *
 * @note: The returned list must be freed using dlinked_list_destroy()
 */
DLinkedList *dlinked_list_create(void);

/**
 * @brief: Appends data to the end of the doubly linked list
 *
 * @param: wrapper The doubly linked list wrapper
 * @param: data Pointer to the data to append
 *
 * @return: The wrapper pointer on success, NULL on failure
 */
DLinkedList *dlinked_list_push_back_node(DLinkedList *wrapper, void *data);

/**
 * @brief: Removes a node from the doubly linked list
 *
 * @param: wrapper The doubly linked list wrapper
 * @param: node The node to delete from the list
 *
 * @return: The wrapper pointer on success, NULL on failure
 *
 * @note: The node must belong to the given wrapper list
 */
DLinkedList *dlinked_list_delete_node(DLinkedList *wrapper, DLinkedListNode *node);

/**
 * @brief: Checks if the doubly linked list is empty
 *
 * @param: wrapper The doubly linked list wrapper
 *
 * @return: true if the list is empty, false otherwise
 */
bool dlinked_list_is_empty(DLinkedList *wrapper);

/**
 * @brief: Checks if the doubly linked list has been properly initialized
 *
 * @param: wrapper The doubly linked list wrapper
 *
 * @return: true if the list is valid and initialized, false otherwise
 */
bool dlinked_list_is_created(DLinkedList *wrapper);

/**
 * @brief: Destroys the doubly linked list and frees all associated memory
 *
 * @param: wrapper The doubly linked list wrapper
 * @param: destroy_data Function pointer to free individual node data, or NULL if no cleanup needed
 *
 * @note: The destroy_data callback will be called for each node's data before the node is freed
 */
void dlinked_list_destroy(DLinkedList *wrapper,void (*destroy_data)(void*));

// TODO: add hash map logic
// TODO: add Prefix Tree logic later

#endif
