/**
 * variable to enable diagnositc calls to pthread_yield
 */
extern int opt_yield;
#define	INSERT_YIELD	0x01	// yield in insert critical section
#define	DELETE_YIELD	0x02	// yield in delete critical section
#define	SEARCH_YIELD	0x04	// yield in lookup/length critical section

/*
 * SortedList (and SortedListElement)
 *
 *	A doubly linked list, kept sorted by a specified key.
 *	This structure is used for a list head, and each element
 *	of the list begins with this structure.
 *
 *	The list head is in the list, and an empty list contains
 *	only a list head.  The list head is also recognizable because
 *	it has a NULL key pointer.
 */
struct SortedListElement {
	struct SortedListElement *prev;
	struct SortedListElement *next;
	const char *key;
};
typedef struct SortedListElement SortedList_t;
typedef struct SortedListElement SortedListElement_t;

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
	SortedListElement_t* cur = list->next;
	while(cur != list) {
	  // fprintf(stderr, "cur!=list\n");
	  //	  fprintf(stderr, "elem: %s\tcur: %s\n", element->key, cur->key);
	  //	  fprintf(stderr, "list->key: %s\n", list->key);
	  if(strcmp(element->key, cur->key) <= 0) {
	    
	    //	    	    fprintf(stderr, "element->key: %s, cur->key: %s\n", element->key, cur->key);
	    break;
	  }
	  cur = cur->next;
	}
	//	fprintf(stderr, "past cur!=list\n");
	//at this point, element->key is less than cur->key.
	element->prev = cur->prev;
	element->next = cur;
	element->prev->next = element;
	element->next->prev = element;
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
	if (element->next->prev != element || element->prev->next != element) {
		// make sure next-prev and prev-next both point to this node
	        fprintf(stderr, "Error in delete\n");
		return -1;
	}

    //able to delete
	element->prev->next = element->next;
	element->next->prev = element->prev;

	return 0;
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 *
 * Note: if (opt_yield & SEARCH_YIELD)
 *		call pthread_yield in middle of critical section
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	SortedListElement_t* cur = list->next;
  	while(cur != list) {
	    if(strcmp(cur->key,key) == 0) {
	      	if(opt_yield & SEARCH_YIELD) {
				pthread_yield();
	      	}
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
	SortedListElement_t* cur = list->next;
  	int counter = 0;
	//fprintf(stderr, "in length\n");
  	if(opt_yield & SEARCH_YIELD) {
   	 	pthread_yield();
  	}

  	while(cur != list) {
	  //	fprintf(stderr, "Key: %s\n", cur->key);
    		cur = cur->next;
    		counter++;
  	}
  	return counter;
}
