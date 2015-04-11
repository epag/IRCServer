
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LinkedList.h"

//
// Initialize a linked list
//
void llist_init(LinkedList * list)
{
    list->head = NULL;
}

//
// It prints the elements in the list in the form:
// 4, 6, 2, 3, 8,7
//
void llist_print(LinkedList * list) {

    ListNode * e;

    if (list->head == NULL) {
        printf("{EMPTY}\n");
        return;
    }

    printf("{");

    e = list->head;
    while (e != NULL) {
        printf("%d", e->value);
        e = e->next;
        if (e!=NULL) {
            printf(", ");
        }
    }
    printf("}\n");
}

//
// Appends a new node with this value at the beginning of the list
//
void llist_add(LinkedList * list, int value) {
    // Create new node
    ListNode * n = (ListNode *) malloc(sizeof(ListNode));
    n->value = value;

    // Add at the beginning of the list
    n->next = list->head;
    list->head = n;
}

//
// Returns true if the value exists in the list.
//
int llist_exists(LinkedList * list, int value) {
    ListNode * p = list -> head;
    while ((p = p -> next) != NULL) {
        if (p->value == value) {
            return 1;
        }
    }
    return 0;
}

//
// It removes the entry with that value in the list.
//
int llist_remove(LinkedList * list, int value) {
    ListNode * p = list -> head;
    ListNode * prev = NULL;
    while (p != NULL) {
        if (p->value == value) {
            break;
        }
        prev = p;
        p = p -> next;
    }
    if (p == NULL) {
        return 0;
    }
    if (prev == NULL) {
        list->head = p->next;
    }
    else {
        prev->next = p->next;
    }
    return 1;
}

//
// It stores in *value the value that correspond to the ith entry.
// It returns 1 if success or 0 if there is no ith entry.
//
int llist_get_ith(LinkedList * list, int ith, int * value) {
    ListNode * p = list -> head;
    int i;
    for (i = 0; i < ith; i++) {
        if (p -> next == NULL) {
            return 0;
        }
        p = p -> next;
    }
    *value = p -> value;
    return 1;
}

//
// It removes the ith entry from the list.
// It returns 1 if success or 0 if there is no ith entry.
//
int llist_remove_ith(LinkedList * list, int ith) {
    ListNode * p = list -> head;
    ListNode * prev = NULL;
    int i;
    for (i = 0; i < ith; i++) {
        if (p->next == NULL) {
            return 0;
        }
        prev = p;
        p = p -> next;
    }
    if (p == NULL) {
        return 0;
    }
    if (prev == NULL) {
        list->head = p->next;
    }
    else {
        prev->next = p->next;
    }
    return 1;
}

//
// It returns the number of elements in the list.
//
int llist_number_elements(LinkedList * list) {
    ListNode * p = list->head;
    int counter = 1;
    while ((p = p->next) != NULL) {
        counter++;
    }
    return counter;
}


//
// It saves the list in a file called file_name. The format of the
// file is as follows:
//
// value1\n
// value2\n
// ...
//
int llist_save(LinkedList * list, char * file_name) {
    ListNode * p = list->head;
    FILE * fp;
    fp = fopen(file_name, "w+");

    if (fp == NULL) {
        return 0;
    }
    while (p != NULL) {
        fprintf(fp, "%d\n", p->value);
        p = p->next;
    }
    fclose(fp);
    return(0);
}

//
// It reads the list from the file_name indicated. If the list already has entries, 
// it will clear the entries.
//
int llist_read(LinkedList * list, char * file_name) {
    int i;
    FILE * fp;
    fp = fopen(file_name, "r");
    llist_init(list);
    if (fp == NULL) {
        return 0;
    }
    else {
        while (fscanf(fp, "%d", &i) != EOF){
            llist_add(list, i);
        }
    }
    fclose(fp);
    return 1;
}


//
// It sorts the list. The parameter ascending determines if the
// order si ascending (1) or descending(0).
//
void llist_sort(LinkedList * list, int ascending) {
    int sorted = 0;
    int temp, i;
    ListNode * p = list->head;
    
    if (p == NULL) {
        return;
    }

    else {
    if (ascending == 1) {
        while (sorted == 0) {
            sorted = 1;
            while((p->next) != NULL) {
            
            if(p->value > (p->next)->value) {
                temp = p->value;
                p->value = (p->next)->value;
                (p->next)->value = temp;
                sorted = 0;
            }
            p = p->next;
        }
        p = list->head;
        }
    }

    if (ascending == 0){
        p = list->head;
        while (sorted == 0) {
            sorted = 1;
            while((p->next) != NULL) {
            
            if(p->value < (p->next)->value) {
                temp = p->value;
                p->value = (p->next)->value;
                (p->next)->value = temp;
                sorted = 0;
            }
            p = p->next;
        }
        p = list->head;
        }
    }
    }
}

//
// It removes the first entry in the list and puts value in *value.
// It also frees memory allocated for the node
//
int llist_remove_first(LinkedList * list, int * value) {
    ListNode * p = list->head;
    if (p == NULL) 
        return 0;
     *value = p->value;
     list->head = p->next;
     free(p);
    return 1;
}

//
// It removes the last entry in the list and puts value in *value/
// It also frees memory allocated for node.
//
int llist_remove_last(LinkedList * list, int *value) {
   ListNode * p = list->head;
   ListNode * prev;

    if (p == NULL)
        return 0;

    while (p->next != NULL) {
        prev = p;
        p = p->next;
    }
    *value = p->value;
    prev->next = NULL;
    free(p);
    return 1;
}

//
// Insert a value at the beginning of the list.
// There is no check if the value exists. The entry is added
// at the beginning of the list.
//
void llist_insert_first(LinkedList * list, int value) {
    llist_add(list, value);
}

//
// Insert a value at the end of the list.
// There is no check if the name already exists. The entry is added
// at the end of the list.
//
void llist_insert_last(LinkedList * list, int value) {
   ListNode * p = list->head;
   ListNode * n = (ListNode *) malloc(sizeof(ListNode));
   if (list-> head == NULL) {
        list->head = (ListNode *) malloc(sizeof(ListNode));
        list->head->value = value;
        list->head->next = NULL;
        return;
   }
   while (p->next != NULL) {
        p = p->next;
    }
    p->next = n;
    n->value = value;
    n->next = NULL;
}

//
// Clear all elements in the list and free the nodes
//
void llist_clear(LinkedList *list)
{
    ListNode * n = list->head;
    ListNode * next;

    while (n != NULL) {
        next = n->next;
        free(n);
        n = next;
    }
    list->head = NULL;
}
