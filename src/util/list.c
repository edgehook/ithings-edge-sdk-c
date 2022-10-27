#include <string.h>
#include <stdlib.h>

#include <util/list.h>
#include <util/util.h>

void list_zero(list* newl){
	memset(newl, 0, sizeof(list));
}

/**
 * Allocates and initializes a new list structure.
 * @return a pointer to the new list structure
 */
list* list_init(void){
	list* newl = (list*)malloc(sizeof(list));

	if(newl) list_zero(newl);

	if(Thread_create_mutex(&newl->mutex)){
		free(newl);
		return NULL;
	}

	return newl;
}

/**
 * Append an already allocated ListNode to a list.  Can be used to move
 * an item from one list to another.
 * @param aList the list to which the item is to be added
 * @param node the ListNode to be used in adding the new node.
 */
void list_append_node(list* alist, list_node* node){
	size_t sizes;

	if(node == NULL) return;

	Thread_lock_mutex(alist->mutex);

	node->prev = alist->last;
	node->next = NULL;

	if(alist->last == NULL){
		alist->first = node;
	}else{
		alist->last->next = node;
	}
	alist->last = node;
	(alist->count)++;

	sizes = sizeof(list_node) + node->size;
	alist->size += sizes;
	Thread_unlock_mutex(alist->mutex);
}

/**
 * Append an conten as node into a list.  Can be used to move
 * an item from one list to another.
 * @param aList the list to which the item is to be added
 * @param content the list item content itself
 * @param size the size of the element
 * @return the list node found, or NULL
 */
list_node* list_append(list* alist, void* content, size_t size){
	list_node* node = (list_node*)malloc(sizeof(list_node));

	if(node){
		memset(node, 0, sizeof(list_node));
		node->content = content;
		node->size = size;
		list_append_node(alist, node);
	}

	return node;
}

/**
 * Insert an conten as node into a list at a specific position.
 * @param aList the list to which the item is to be added
 * @param content the list item content itself
 * @param size the size of the element
 * @param index the position in the list. If NULL, this function is equivalent
 * to list_append.
 * @return the list node found, or NULL
 */
list_node* list_insert(list* alist, void* content, size_t size, list_node* index){
	list_node* node = NULL;

	if(index == NULL) 
		return list_append(alist, content, size);

	node = (list_node*)malloc(sizeof(list_node));
	if(node == NULL) return NULL;

	Thread_lock_mutex(alist->mutex);

	node->content = content;
	node->size = size;

	node->prev = index->prev;
	node->next = index;
	index->prev = node;
	
	if(index->prev == NULL){
		alist->first = node;
	}else{
		index->prev->next = node;
	}

	(alist->count)++;
	alist->size += sizeof(list_node) + node->size;

	Thread_unlock_mutex(alist->mutex);

	return node;
}

/**
 * Forward iteration through a list
 * @param aList the list to which the operation is to be applied
 * @param pos pointer to the current position in the list.  NULL means start from the beginning of the list
 * This is updated on return to the same value as that returned from this function
 * @return pointer to the current list element
 */
list_node* list_next_node(list* alist, list_node** pos){
	return *pos = (*pos == NULL) ? alist->first : (*pos)->next;
}

/**
 * Backward iteration through a list
 * @param aList the list to which the operation is to be applied
 * @param pos pointer to the current position in the list.  NULL means start from the end of the list
 * This is updated on return to the same value as that returned from this function
 * @return pointer to the current list element
 */
list_node* list_prev_node(list* alist, list_node** pos){
	return *pos = (*pos == NULL) ? alist->last : (*pos)->prev;
}

/**
 * Finds an node in a list by comparing the content or pointer to the content.  A callback
 * function is used to define the method of comparison for each node.
 * @param aList the list in which the search is to be conducted
 * @param content pointer to the content to look for
 * @param callback pointer to a function which compares each node (NULL means compare by content pointer)
 * @return the list node found, or NULL
 */
static list_node* list_find_node(list* alist, void* content, int(*callback)(void*, void*)){
	list_node* node = NULL;
	list_node* current = NULL;

	if(alist->current != NULL && ((callback == NULL && alist->current->content == content) ||
		(callback != NULL && callback(alist->current->content, content)))){
		return alist->current;
	}

	while(list_next_node(alist, &current) != NULL){
		if(callback){
			if(callback(current->content, content)) 
				break;
		}else{
			if(current->content == content)
				break;
		}
	}

	return current;
}

/**
 * Finds an node in a list by comparing the content pointers, rather than the contents
 * @param aList the list in which the search is to be conducted
 * @param content pointer to the list node content itself
 * @return the list node found, or NULL
 */
list_node* list_find(list* alist, void* content){
	list_node* node = NULL;

	Thread_lock_mutex(alist->mutex);
	node = list_find_node(alist, content, NULL);
	Thread_unlock_mutex(alist->mutex);

	return node;
}

/**
 * Finds an node in a list by comparing the content pointers, rather than the contents
 * @param aList the list in which the search is to be conducted
 * @param content pointer to the user data.
 * @param callback pointer to a function which compares each node (NULL means compare by content pointer)
 * @return the list node found, or NULL
 */
void* list_find_v2(list* alist, void* content, int(*callback)(void*, void*)){
	list_node* node = NULL;

	Thread_lock_mutex(alist->mutex);
	node = list_find_node(alist, content, callback);
	Thread_unlock_mutex(alist->mutex);

	return node == NULL ? NULL : node->content;
}

/**
 * Removes and optionally frees an element in a list by comparing the content.
 * A callback function is used to define the method of comparison for each element.
 * @param aList the list in which the search is to be conducted
 * @param content pointer to the content to look for
 * @param callback pointer to a function which compares each element
 * @param freeContent boolean value to indicate whether the item found is to be freed
 * @return 1=item removed, 0=item not removed
 */
static int list_unlink_node(list* alist, void* content, int(*callback)(void*, void*), int free_content){
	list_node* node = NULL;
	list_node* next = NULL;

	node = list_find_node(alist, content, callback);
	if(node == NULL)
		return 0;

	if(node->prev == NULL){
		alist->first = node->next;
	}else{
		node->prev->next = node->next;
	}
	if(node->next == NULL){
		alist->last = node->prev;
	}else{
		node->next->prev = node->prev;
	}

	//free the node with content and content.
	if(free_content) free_memory((char**)(&node->content));
	if(alist->current == node){
		next = node->next;
		alist->current = next;
	}

	(alist->count)--;
	alist->size -= (node->size + sizeof(list_node));
	free(node);

	return 1;
}

/**
 * Removes node but does not free an content in a list by comparing the pointer to the content.
 * @param aList the list in which the search is to be conducted
 * @param content pointer to the content to look for
 * @return 1=item removed, 0=item not removed
 */
int list_remove(list* alist, void* content){
	int ret;
	
	Thread_lock_mutex(alist->mutex);
	ret = list_unlink_node(alist, content, NULL, 0);
	Thread_unlock_mutex(alist->mutex);

	return ret;
}

/**
 * Removes node and frees an content in a list by comparing the pointer to the content.
 * @param aList the list from which the item is to be removed
 * @param content pointer to the content to look for
 * @return 1=item removed, 0=item not removed
 */
int list_delete(list* alist, void* content){
	int ret;
	
	Thread_lock_mutex(alist->mutex);
	ret = list_unlink_node(alist, content, NULL, 1);
	Thread_unlock_mutex(alist->mutex);

	return ret;
}

/*
* find and pop content from the list head and remove the node.
* @param aList the list from which the item is to be pop.
* @param content pointer to the content to look for.
* @param callback pointer to a function which compares each element
* @return pointer to the node's content.
*/
void* list_find_and_pop_data(list* alist, void* content, int(*callback)(void*, void*)){
	void* data = NULL;
	list_node* node = NULL;

	if(alist == NULL || content == NULL) return NULL;

	node = list_find_v2(alist, content, callback);
	if(node == NULL) return NULL;

	data = node->content;
	list_remove(alist, node->content);

	return data;
}

/*
* pop the content from the list head and remove the head.
*@param aList the list from which the item is to be pop.
* @return pointer to the node's content.
*/
void* list_pop_head(list* alist){
	void *content = NULL;
	list_node* node = NULL;

	Thread_lock_mutex(alist->mutex);
	node = alist->first;
	if(node == NULL){
		Thread_unlock_mutex(alist->mutex);
		return NULL;
	}

	content = node->content;
	list_unlink_node(alist, content, NULL, 0);
	Thread_unlock_mutex(alist->mutex);
	return content;	
}

/*
* push the content into the list head.
* @param aList the list from which the item is to be push.
* @param content the list item content itself
* @param size the size of the element
*/
void list_push_head(list* alist, void* content, size_t size){
	list_insert(alist, content, size, alist->first);
}
/*
* pop the content from the list tail and remove the tail.
* @param aList the list from which the item is to be pop.
* @return pointer to the node's content.
*/
void* list_pop_tail(list* alist){
	void *content = NULL;
	list_node* node = NULL;

	Thread_lock_mutex(alist->mutex);
	node = alist->last;
	if(node == NULL){
		Thread_unlock_mutex(alist->mutex);
		return NULL;
	}

	content = node->content;
	list_unlink_node(alist, content, NULL, 0);
	Thread_unlock_mutex(alist->mutex);
	return content;
}

/*
* push the content into the list tail.
* @param aList the list from which the item is to be push.
* @param content the list item content itself
* @param size the size of the element
*/
void list_push_tail(list* alist, void* content, size_t size){
	list_insert(alist, content, size, NULL);
}

/**
 * Removes and frees all node in a list, leaving the list ready for new nodes.
 * @param aList the list to which the operation is to be applied
 */
void list_empty(list* alist){
	list_node* node = NULL;

	Thread_lock_mutex(alist->mutex);

	while(alist->first){
		node = alist->first;
		alist->first = node->next;
		if(node->next){
			node->next->prev = NULL;
		}

		if(node->content) 
			free_memory((char**)(&node->content));

		free(node);
	}

	alist->count = 0;
	alist->size = 0;
	alist->first = alist->current = alist->last = NULL;
	Thread_unlock_mutex(alist->mutex);
}

/**
 * Removes and frees all nodes in a list, and frees the list itself
 * @param aList the list to which the operation is to be applied
 */
void list_destory(list* alist){
	list_empty(alist);
	Thread_destroy_mutex(alist->mutex);
	alist->mutex = NULL;
	free(alist);
}
