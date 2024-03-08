/*Reema Chintala
rechinta
PA3*/

//-----------------------------------------------------------------------------
// List.c
// Implementation file for List ADT
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "List.h"

// structs --------------------------------------------------------------------

// private Node type
typedef struct NodeObj *Node;

// private NodeObj type
typedef struct NodeObj {
    ListElement data;
    Node next;
    Node previous;
} NodeObj;

// private ListObj type
typedef struct ListObj {
    Node front;
    Node back;
    Node cursor;
    int length;
    int cursorIndex;
} ListObj;

// Constructors-Destructors ---------------------------------------------------

// newNode()
// Returns reference to new Node object. Initializes next, previous, and data fields.
Node newNode(ListElement data) {
    Node N = malloc(sizeof(NodeObj));
    assert(N != NULL);
    N->data = data;
    N->next = NULL;
    N->previous = NULL; //double check
    return (N);
}

// freeNode()
// Frees heap memory pointed to by *pN, sets *pN to NULL.
void freeNode(Node *pN) {
    if (pN != NULL && *pN != NULL) {
        free(*pN);
        *pN = NULL;
    }
}

// newList()
// Returns reference to new empty List object.
List newList(void) {
    List L;
    L = malloc(sizeof(ListObj));
    assert(L != NULL);
    L->front = L->back = NULL;
    L->length = 0;
    L->cursor = NULL;
    L->cursorIndex = -1;
    return (L);
}

void clear(List L) {
    //to do: error handling for NULL List
    while (length(L) > 0) {
        deleteBack(L);
    }
}

// freeList()
// Frees all heap memory associated with List *pL, and sets *pL to NULL.
void freeList(List *pL) {

    if (pL != NULL && *pL != NULL) {
        clear(*pL);
    }
    free(*pL);
    *pL = NULL;
}

// Access functions -----------------------------------------------------------

// length()
// Returns the length of L.
int length(List L) {
    if (L == NULL) {
        printf("List Error: calling length() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }
    return (L->length);
}

// Returns index of cursor element if defined, -1 otherwise.
int cur_index(List L) {
    if ((L->cursor) == NULL) {
        return -1;
    }
    return L->cursorIndex;
}

// Returns front element of L. Pre: length()>0
void *front(List L) {
    if (L == NULL) {
        printf("List Error: calling front() on NULL LIST reference\n");
        exit(EXIT_FAILURE);
    }
    if (length(L) <= 0) {
        printf("List Error: calling front() on empty List\n");
        freeList(&L);
        exit(EXIT_FAILURE);
    }
    return L->front->data;
}

// Returns back element of L. Pre: length()>0

void *back(List L) {
    if (L == NULL) {
        printf("List Error: calling back() on NULL LIST reference\n");
        exit(EXIT_FAILURE);
    }
    if (length(L) <= 0) {
        printf("List Error: calling back() on empty List\n");
        freeList(&L);
        exit(EXIT_FAILURE);
    }
    return L->back->data;
}

// Returns cursor element of L. Pre: length()>0, index()>=0
void *get_cursor(List L) {

    if (L == NULL) {
        printf("List Error: calling get() on NULL LIST reference\n");
        exit(EXIT_FAILURE);
    }

    if (length(L) <= 0 || ((L->cursorIndex) < 0)) {
        printf("List Error: calling get() on NULL cursor\n");
        freeList(&L);
        exit(EXIT_FAILURE);
    }

    //printf("check else\n");

    return L->cursor->data;
}

// Manipulation procedures ----------------------------------------------------

// Overwrites the cursor element’s data with x.
// Pre: length()>0, index()>=0
void set(List L, void *x) {

    if (length(L) > 0 && cur_index(L) >= 0) {
        L->cursor->data = x;
    }
}

// If L is non-empty, sets cursor under the front element,
// otherwise does nothing.
void moveFront(List L) {
    if (length(L) > 0) {
        //printf("moving..: \n");
        L->cursorIndex = 0;
        L->cursor = L->front;
        //printf("index is: %d\n", index(L));
    }
}

// If L is non-empty, sets cursor under the back element,
// otherwise does nothing.

//to do: pre conditions list null and list empty
void moveBack(List L) {

    if (length(L) > 0) {
        L->cursorIndex = length(L) - 1;
        L->cursor = L->back;
    }
}

// If cursor is defined and not at front, move cursor one
// step toward the front of L; if cursor is defined and at
// front, cursor becomes undefined; if cursor is undefined
// do nothing
void movePrev(List L) {

    if (L == NULL) {
        printf("List Error: calling movePrev() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    if (cur_index(L) >= 1) {
        L->cursor = L->cursor->previous;
        L->cursorIndex--;
    } else if (cur_index(L) == 0) {
        L->cursor = NULL;
        L->cursorIndex = -1;
    }
}

// If cursor is defined and not at back, move cursor one
// step toward the back of L; if cursor is defined and at
// back, cursor becomes undefined; if cursor is undefined
// do nothing
void moveNext(List L) {

    if (L == NULL) {
        printf("List Error: calling moveNext() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    if (cur_index(L) == length(L) - 1) {
        L->cursor = NULL;
        L->cursorIndex = -1;
    } else if (cur_index(L) < length(L) && (L->cursor != NULL)) {
        L->cursorIndex++;
        L->cursor = L->cursor->next;
    }
}

// Insert new element into L. If L is non-empty,
// insertion takes place before front element.
void prepend(List L, void *x) {

    Node new = newNode(x);

    if (L == NULL) {
        printf("List Error: calling prepend() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }
    if (length(L) > 0) {
        L->front->previous = new;
        new->next = L->front;
    } else {
        L->back = new;
    }
    L->front = new;
    L->length++;
    L->cursorIndex++;
}

// Insert new element into L. If L is non-empty,
// insertion takes place after back element.
void append(List L, void *x) {

    Node new = newNode(x);

    if (L == NULL) {
        printf("List Error: calling append() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }
    if (length(L) > 0) {
        L->back->next = new;
        new->previous = L->back;
        L->back = new;

    } else {
        L->front = L->back = new;
    }
    L->length++;
}

// Insert new element before cursor.
// Pre: length()>0, index()>=0
void insertBefore(List L, void *x) {

    if (L == NULL) {
        printf("List Error: calling insertBefore() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    if (length(L) > 0 && cur_index(L) >= 0) {
        if (cur_index(L) == 0) {
            prepend(L, x);
        } else {

            Node new = newNode(x);
            new->next = L->cursor;
            new->previous = L->cursor->previous;

            L->cursor->previous = new;
            new->previous->next = new;
            L->length++;
            L->cursorIndex++;
        }

    } else {
        if (length(L) < 1) {
            printf("List Error: calling insertBefore() on list length less than 1\n");
            exit(EXIT_FAILURE);
        }
        if (cur_index(L) < 0) {
            printf("List Error: calling insertBefore() on  or index less than zero \n");
            exit(EXIT_FAILURE);
        }
    }
}

// Insert new element after cursor.
// Pre: length()>0, index()>=0
void insertAfter(List L, void *x) {

    if (L == NULL) {
        printf("List Error: calling insertAfter() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    if (length(L) > 0 && cur_index(L) >= 0) {

        if (cur_index(L) == (length(L) - 1)) {
            append(L, x);
        }

        else {

            Node new = newNode(x);
            new->previous = L->cursor;
            new->next = L->cursor->next;

            L->cursor->next = new;
            new->next->previous = new;
            L->length++;
        }
    }
}

// Delete the front element. Pre: length()>0
void deleteFront(List L) {

    if (L == NULL) {
        printf("List Error: calling deleteFront() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    if (L->cursor == L->front) {
        L->cursor = NULL;
        L->cursorIndex = -1;
    }

    Node N = L->front;
    if (length(L) > 1) {
        L->front = L->front->next;
        L->length--;
        freeNode(&N);
        L->front->previous = NULL;
        L->cursorIndex--;
    } else {
        L->length = 0;
        freeNode(&N);
        L->front = NULL;
        L->back = NULL;
        L->cursorIndex = -1;
    }
}
// Delete the back element. Pre: length()>0
void deleteBack(List L) {
    if (L == NULL) {
        printf("List Error: calling deleteBack() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    if (L->cursor == L->back) {
        L->cursor = NULL;
        L->cursorIndex = -1;
    }

    Node N = NULL;
    if (length(L) > 1) {
        N = L->back;
        L->back = L->back->previous;

        L->back->next = NULL;

    } else {

        N = L->back;

        L->front = NULL;
        L->back = NULL;
    }
    freeNode(&N);
    L->length--;
}

// Delete cursor element, making cursor undefined.
void delete (List L) {

    if (L == NULL) {
        printf("List Error: calling delete() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    if (cur_index(L) >= 0 && length(L) > 0) {

        if (cur_index(L) == 0) {
            deleteFront(L);

        } else if (cur_index(L) == length(L) - 1) {
            deleteBack(L);
        } else {

            if (L->cursor != NULL) {
                if (L->cursor->previous != NULL) {
                    L->cursor->previous->next = L->cursor->next;
                }
                if (L->cursor->next != NULL) {
                    L->cursor->next->previous = L->cursor->previous;
                }

                L->cursor = NULL;

                //freeNode(&(L->cursor));
                L->cursorIndex = -1;
                L->length--;
            }
        }
    }
}
