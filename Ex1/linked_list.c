#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"

/*Definition of the node struct*/
typedef struct node{
  struct node * next;
  int val;
  print_ptr print_list;
} node;

void print_item(node* current); // The declaration is need so that it can be used in add function

/* Initialization of the head node */
node* head = NULL;

/*Function used to add an item to the list.
The item will be added at the end of the list*/
void add(int val){
	node* current = malloc(sizeof(node));
	if(head == NULL){                  //If this is the first node set it as head
		current->next = NULL;
		current->val = val;
		current->print_list = print_item;
		head = current;
	}
	else{
		current = head;
		while(current->next!=NULL){
			current = current->next;   //Iterate over the list till the last item
		}
		current->next = malloc(sizeof(node));
		current->next->val = val;
		current->next->next = NULL;
		current->next->print_list = print_item;
	}
}

/*Function used to print the list*/
void print(){
	node* current = head;
	if(current == NULL){
		printf("The list has no items.\n");
	}
	else{
		while(current!=NULL){
			current->print_list(current);
			current = current->next;
		}
	}
}

/* Function used to print individual items*/
void print_item(node* current){
	if(current!=NULL){
		printf("%d\n",current->val);
	}
	else{
		printf("The current item is NULL.");
	}
}

/* Function used to delete a specific item*/
void delete(int val){
	node* current = NULL;
	node* temp = NULL;

	if(head == NULL){
		return;                //Check for empty list
	}

	if(head->val==val){
		temp = head;
		head = head->next;
		free(temp);
	}
	else if((head->val != val) && (head->next == NULL)){   //Mange the case when the value is not in the list and 
		printf("The value was not found in the list.\n");  // the list has only 1 node
	}
	else{
		current = head;
		while((current->next->next!=NULL)&&(current->next->val!=val)){
			current = current->next;
		}
		if((current->next->next==NULL)&&(current->next->val!=val)){
			printf("The value was not found in the list.\n");
		}
		else{
			temp = current->next;
			current->next = temp->next;
			free(temp);
		}
	}
}

/*Functions used to sort the list.*/
void sort(){
	node* i;
	node* j;
	int temp;

	if(head==NULL){
		return;                       //Check for empty list
	}

	for(i=head;i->next!=NULL;i=i->next){
		for(j=i->next;j!=NULL;j=j->next){
			if(i->val > j->val){
				temp = i->val;
				i->val = j->val;
				j->val = temp;
			}
		}
	}
}

/*Function used to delete the last entry in the list*/
void pop(){
	node* current = head;
	node* temp = NULL;

	if(head == NULL){
		return;
	}
	if(current->next == NULL){
		temp = head;
		head = NULL;
		free(temp);
	}
	while(current->next->next != NULL){
		current = current->next;
	}
	temp = current->next;
	current->next = NULL;
	free(temp);
}
/*Function used to flush the list*/
void flush(){
	while(head->next != NULL){
		pop();
	}
	free(head);
}