#ifndef __LIST_H_
#define __LIST_H_

#include "lib_api.h"
#include <stdlib.h> /* for size_t definition */
#include <util/thread.h>

/**
 * Structure to hold all data for one list node
 */
typedef struct list_node_struct{
	struct list_node_struct *prev, /**< pointer to previous list element */
							*next;	/**< pointer to next list element */
	void* content;					/**< pointer to element content */
	size_t size;  					/**< heap storage used */
} list_node;

/**
 * Structure to hold all data for one list
 * this is a thread-safty list.
 */
typedef struct{
	list_node *first,	/**< first element in the list */
				*last,	/**< last element in the list */
				*current;	/**< current element in the list, for iteration */
	int count;  /**< no of items */
	size_t size;  /**< heap storage used */
	mutex_type mutex; /* for thread mutex.*/
} list;

LIBAPI void list_zero(list* newl);
/**
 * Allocates and initializes a new list structure.
 * @return a pointer to the new list structure
 */
LIBAPI list* list_init(void);
/**
 * Append an already allocated ListNode to a list.  Can be used to move
 * an item from one list to another.
 * @param aList the list to which the item is to be added
 * @param node the ListNode to be used in adding the new node.
 */
LIBAPI void list_append_node(list* alist, list_node* node);
/**
 * Append an conten as node into a list.  Can be used to move
 * an item from one list to another.
 * @param aList the list to which the item is to be added
 * @param content the list item content itself
 * @param size the size of the element
 * @return the list node found, or NULL
 */
LIBAPI list_node* list_append(list* alist, void* content, size_t size);
/**
 * Insert an conten as node into a list at a specific position.
 * @param aList the list to which the item is to be added
 * @param content the list item content itself
 * @param size the size of the element
 * @param index the position in the list. If NULL, this function is equivalent
 * to list_append.
 * @return the list node found, or NULL
 */
LIBAPI list_node* list_insert(list* alist, void* content, size_t size, list_node* index);
/**
 * Finds an node in a list by comparing the content pointers, rather than the contents
 * @param aList the list in which the search is to be conducted
 * @param content pointer to the list node content itself
 * @return the list node found, or NULL
 */
LIBAPI list_node* list_find(list* alist, void* content);
/**
 * Finds an node in a list by comparing the content pointers, rather than the contents
 * @param aList the list in which the search is to be conducted
 * @param content pointer to the user data
 * @param callback pointer to a function which compares each node (NULL means compare by content pointer)
 * @return the list node found, or NULL
 */
LIBAPI void* list_find_v2(list* alist, void* content, int(*callback)(void*, void*));
/**
 * Removes node but does not free an content in a list by comparing the pointer to the content.
 * @param aList the list in which the search is to be conducted
 * @param content pointer to the content to look for
 * @return 1=item removed, 0=item not removed
 */
LIBAPI int list_remove(list* alist, void* content);
/**
 * Removes node and frees an content in a list by comparing the pointer to the content.
 * @param aList the list from which the item is to be removed
 * @param content pointer to the content to look for
 * @return 1=item removed, 0=item not removed
 */
LIBAPI int list_delete(list* alist, void* content);
/**
 * Forward iteration through a list
 * @param aList the list to which the operation is to be applied
 * @param pos pointer to the current position in the list.  NULL means start from the beginning of the list
 * This is updated on return to the same value as that returned from this function
 * @return pointer to the current list element
 */
LIBAPI list_node* list_next_node(list* alist, list_node** pos);
/*
* find and pop content from the list head and remove the node.
* @param aList the list from which the item is to be pop.
* @param content pointer to the content to look for.
* @param callback pointer to a function which compares each element
* @return pointer to the node's content.
*/
LIBAPI void* list_find_and_pop_data(list* alist, void* content, int(*callback)(void*, void*));
/*
* pop the content from the list head and remove the head.
*@param aList the list from which the item is to be pop.
* @return pointer to the node's content.
*/
LIBAPI void* list_pop_head(list* alist);
/*
* push the content into the list head.
* @param aList the list from which the item is to be push.
* @param content the list item content itself
* @param size the size of the element
*/
LIBAPI void list_push_head(list* alist, void* content, size_t size);
/*
* pop the content from the list tail and remove the tail.
* @param aList the list from which the item is to be pop.
* @return pointer to the node's content.
*/
LIBAPI void* list_pop_tail(list* alist);
/*
* push the content into the list tail.
* @param aList the list from which the item is to be push.
* @param content the list item content itself
* @param size the size of the element
*/
LIBAPI void list_push_tail(list* alist, void* content, size_t size);
/**
 * Removes and frees all node in a list, leaving the list ready for new nodes.
 * @param aList the list to which the operation is to be applied
 */
LIBAPI void list_empty(list* alist);
/**
 * Removes and frees all nodes in a list, and frees the list itself
 * @param aList the list to which the operation is to be applied
 */
LIBAPI void list_destory(list* alist);

#endif /*__LIST_H_ */
