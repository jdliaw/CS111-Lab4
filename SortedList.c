#include "SortedList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int opt_yield;

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

	//base case empty
	if(cur == NULL) {
	  list->next = element;
	  element->prev = list;
	  element->next = NULL;
	  return;
	}

	//insert beginning
	if(strcmp(element->key, cur->key) <= 0) {
	  element->next = cur;
	  element->prev = list;
	  list->next = element;
	  element->next->prev = element;
	  return;
	}

	while(cur->next != NULL) {
	  //element is now finally smaller than cur, so break. we want to insert before cur.
	  if(strcmp(element->key, cur->next->key) <= 0) {     
	    break;
	  }
	  cur = cur->next;
	}
	//reached the end
	if(cur->next == NULL) {
	  cur->next = element;
	  element->prev = cur;
	  element->next = NULL;
	  return;
	}
	
	//found
     	element->prev = cur->next->prev;
	element->next = cur->next;
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

int SortedList_delete(SortedListElement_t *element) {
	if (element->next->prev != element || element->prev->next != element) {
		// make sure next-prev and prev-next both point to this node
		return -1;
	}
	// last element
	if (element->next == NULL) {
		element->prev = NULL;
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
  SortedListElement_t* cur = list->next;
  while(cur != NULL) {
    if(strcmp(cur->key,key) == 0) {
      if(opt_yield & SEARCH_YIELD) {
	pthread_yield();
      }
      fprintf(stderr, "Found: %s\n", cur->key);
      return cur;
    }
    cur = cur->next;
  }
  fprintf(stderr, "Not found\n");
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

  if(opt_yield & SEARCH_YIELD) {
    pthread_yield();
  }
  while(cur != NULL) {
    fprintf(stderr, "Key: %s\n", cur->key);
    cur = cur->next;
    counter++;
  }
  fprintf(stderr, "Length: %d\n", counter);
  return counter;
}

int main1() {
  SortedList_t *head;
  head = malloc(sizeof(SortedListElement_t));
  char*val = "test";
  head->next = NULL;

  SortedListElement_t *node1 = malloc(sizeof(SortedListElement_t));
  char* val1 = "100";
  node1->key = val1;
  node1->next = NULL;

  SortedListElement_t *node2 = malloc(sizeof(SortedListElement_t));
  char*val2 = "10000";
  node2->key = val2;
  node2->next = NULL;

  SortedListElement_t *node3 = malloc(sizeof(SortedListElement_t));
  char* val3 = "10";
  node3->key = val3;
  node3->next = NULL;

  SortedListElement_t *node4 = malloc(sizeof(SortedListElement_t));
  char* val4 = "1";
  node4->key = val4;
  node4->next = NULL;

  SortedListElement_t *node5 = malloc(sizeof(SortedListElement_t));
  char* val5 = "1000";
  node5->key = val5;
  node5->next = NULL;

  SortedListElement_t *node6 = malloc(sizeof(SortedListElement_t));
  char* val6 = "188";
  node6->key = val6;
  node6->next = NULL;

  SortedListElement_t *node7 = malloc(sizeof(SortedListElement_t));
  char* val7 = "11";
  node7->key = val7;
  node7->next = NULL;
  
  fprintf(stderr, "start insert\n");
  SortedList_insert(head, node1);
  SortedList_insert(head, node2);
  SortedList_insert(head, node3);
  SortedList_insert(head, node4);
    SortedList_insert(head, node5);
   SortedList_insert(head, node6);
   SortedList_insert(head, node7);
  //  SortedList_length(head);
  SortedListElement_t *itr = head->next;
  while(itr != NULL) {
    fprintf(stderr, "Iteration test: %s\n", itr->key);
    itr = itr->next;
  }
  itr = head->next;
  while(itr->next != NULL) {
    itr = itr->next;
  }
  while(itr != head) {
    fprintf(stderr,"Backwards: %s\n", itr->key);
    itr = itr->prev;
  }
  /*  SortedList_lookup(head, val1);
  SortedList_lookup(head, val2);
  SortedList_lookup(head, val3);
  SortedList_lookup(head, val4);*/
  return 0;
}
