
#include <SortedList.h>
#include <stdio.h>
#include <stdlib.h>

// struct SortedListElement {
// 	struct SortedListElement *prev;
// 	struct SortedListElement *next;
// 	const char *key;
// };
// typedef struct SortedListElement SortedList_t;
// typedef struct SortedListElement SortedListElement_t;



/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 *
 * Note: if (opt_yield & INSERT_YIELD)
 *		call pthread_yield in middle of critical section
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
	SortedListElement_t* cur = list;

	while (cur->next != NULL && cur->key < element->key) {
		// find where it belongs in SortedList
		cur = cur->next;
	}
	if (cur->next == NULL) {
		// insert at tail of list
		cur->next = element;
		element->prev = cur;
		element->next = NULL;
	}
	else {
		// insert in middle of list
		SortedListElement_t* temp = cur->next;
		cur->next = element;
		element->prev = cur;
		element->next = temp;
		temp->prev = element;
	}
}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 * Note: if (opt_yield & DELETE_YIELD)
 *		call pthread_yield in middle of critical section
 */

int SortedList_delete( SortedListElement_t *element) {

}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {

}

int SortedList_length(SortedList_t *list) {

}