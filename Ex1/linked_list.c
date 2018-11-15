#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"


struct node;
typedef void (*print_ptr)(struct node*);

typedef struct node {
  struct node * next;
  int val;
  print_ptr print_list;
} node;

static void print_item(node* current); // The declaration is need so that it can be used in add function

/* Initialization of the head node */
static node* head = NULL;

/*Function used to add an item to the list.
The item will be added at the end of the list*/
void add(int val) {
	node* current = malloc(sizeof(node));
	if (head == NULL) {                  //If this is the first node set it as head
		current->next = NULL;
		current->val = val;
		current->print_list = print_item;
		head = current;
	}
	else {
		current = head;
		while (current->next != NULL) {
			current = current->next;   //Iterate over the list till the last item
		}
		current->next = malloc(sizeof(node));
		current->next->val = val;
		current->next->next = NULL;
		current->next->print_list = print_item;
	}
}

/*Function used to print the list*/
void print() {
	node* current = head;
	if (current == NULL) {
		printf("The list has no items.\n");
	}
	else{
		while (current != NULL) {
			current->print_list(current);
			current = current->next;
		}
	}
}

/* Function used to print individual items*/
static void print_item(node* current) {
	if (current != NULL) {
		printf("%d\n",current->val);
	}
	else {
		printf("The current item is NULL.");
	}
}

/* Function used to delete a specific item*/
void delete(int val) {
	node* current = NULL;
	node* temp = NULL;

	if (head == NULL) {
		return;                //Check for empty list
	}

	if (head->val == val) {
		temp = head;
		head = head->next;
		free(temp);
	}
	else if ((head->val != val) && (head->next == NULL)) {   //Mange the case when the value is not in the list and 
		printf("The value was not found in the list.\n");  // the list has only 1 node
	}
	else {
		current = head;
		while ((current->next->next != NULL) && (current->next->val != val)) {
			current = current->next;
		}
		if ((current->next->next == NULL) && (current->next->val != val)) {
			printf("The value was not found in the list.\n");
		}
		else {
			temp = current->next;
			current->next = temp->next;
			free(temp);
		}
	}
}

/*Function used to delete the last entry in the list*/
 void pop(){
	node* current = head;
	node* temp = NULL;

	if (head == NULL) {
		return;
	}
	if (current->next == NULL) {
		temp = head;
		head = NULL;
		free(temp);
	}
	while (current->next->next != NULL) {
		current = current->next;
	}
	temp = current->next;
	current->next = NULL;
	free(temp);
}

/*Function used to flush the list*/
void flush(){
	while (head->next != NULL) {
		pop();
	}
	free(head);
}

/* Splits the current list into two lists. The first part will point to the initial head while the 
second part will point to the mid element (if the list is long enough). If the number of elements 
is odd the first list will have the extra item*/
static void split(node* first_item, node** first_part, node** second_part) {
	node* slow;
	node* fast;

	if (first_item == NULL || first_item->next == NULL) {
		*first_part = first_item;
		*second_part = NULL;
	}
	else {
		slow = first_item;
		fast = first_item->next;

		while (fast != NULL) {
			fast = fast->next;

			if (fast != NULL) {
				slow = slow->next;
				fast = fast->next;
			}
		}

	*first_part = first_item;
	*second_part = slow->next;
	slow->next = NULL;
	}
}

/* Returns a merged list based on the pointers received. This function does the actual sorting based on 
the value of each item.*/
static node* mergeLists(node* first_part,node* second_part) {
	
	node* mergedList = NULL;

	if (first_part == NULL) {
		return second_part;
	}
	else if (second_part == NULL) {
		return first_part;
	}
	if (first_part->val <= second_part->val) {
		mergedList = first_part;
		mergedList->next = mergeLists(first_part->next,second_part);
	}
	else {
		mergedList = second_part;
		mergedList->next = mergeLists(first_part,second_part->next);
	}

	return mergedList;
}

/* Starts with the initial list and breaks it until it gets to a list with a single item.
Calls the merging function to merge the items until the list is recreated.*/
static void mergeSort(node** first_item){

	node* head_ref = *first_item;
	node* first_part = NULL;
	node* second_part  = NULL;

	if (head_ref == NULL || head_ref->next == NULL) {
		return;
	}

	split(head_ref,&first_part,&second_part);

	mergeSort(&first_part);
	mergeSort(&second_part);

	*first_item = mergeLists(first_part,second_part);
}

/*Call to mergeSort using the head of the list in order to avoid the passing of the head in main.*/
void sort() {
	mergeSort(&head);
}