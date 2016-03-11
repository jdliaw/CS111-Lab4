#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

	while (cur->next != NULL && strcmp(cur->key,element->key) <= 0) {
		// find where it belongs in SortedList
		cur = cur->next;
	}
	if (cur->next == NULL) {
		// insert at tail of list
		cur->next = element;
		if (opt_yield & INSERT_YIELD)
			pthread_yield();
		element->prev = cur;
		element->next = NULL;
	}
	else {
		// insert in middle of list
		SortedListElement_t* temp = cur->next;
		cur->next = element;
		element->prev = cur;
		if (opt_yield & INSERT_YIELD)
			pthread_yield();
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

int SortedList_delete(SortedListElement_t *element) {
	if (element->next->prev != element || element->prev->next != element) {
		// make sure next-prev and prev-next both point to this node
		return -1;
	}
	// last element
	if (element->next == NULL) {
		element->prev = NULL;
		if (opt_yield & DELETE_YIELD)
			pthread_yield();
		element->prev->next = NULL;

	}
	// middle element
	else {
		element->prev->next = element->next;
		if (opt_yield & DELETE_YIELD)
			pthread_yield();
		element->next->prev = element->prev;
		element->next = NULL;
		element->prev = NULL;
	}
	return 0;
}


/**
 * SortedList_lookup ... search sorted list for a key
 *
 *The specified list will be searched for an
 *element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 *
 * Note: if (opt_yield & SEARCH_YIELD)
 *call pthread_yield in middle of critical section
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
  SortedListElement_t* cur = list;
  while(cur != NULL) {
    if(cur->key == key) {
      return cur;
    }
    cur = cur->next;
  }
  return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 *
 * Note: if (opt_yield & SEARCH_YIELD)
 *		call pthread_yield in middle of critical section
 */
int SortedList_length(SortedList_t *list) {
  SortedListElement_t* cur = list;
  int counter = 0;
  while(cur != NULL) {
    cur = cur->next;
    counter++;
  }
  return counter;
}

int main() {
  return 0;
}
