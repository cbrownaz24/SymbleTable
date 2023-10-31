/*--------------------------------------------------------------------*/
/* symtablelist.c                                                     */
/* Author: Connor Brown                                               */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "symtable.h"

/*--------------------------------------------------------------------*/

/* Each item stored in a SymTable. SymTableNodes are linked to form a 
   list. */

struct SymTableNode {
  /* The string key. */
  char *pcKey; 

  /* The generic value. */
  void *pvValue; 

  /* A reference to the next node in the list. */
  struct SymTableNode *psNextNode; 
};

/*--------------------------------------------------------------------*/

/* A SymTable structure is a "manager" structure which tracks the first
   SymTableNode in the linked list and the length of the list. */

struct SymTable {
  /* A reference to the first node in the list. */
  struct SymTableNode *psFirstNode;

  /* Length of the list. */
  size_t uLength; 
};

/*--------------------------------------------------------------------*/

SymTable_T SymTable_new(void) {
  /* Allocate memory for SymTable struct, but store/pass a 
     reference to it, not a copy. */
  SymTable_T oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));

  /* Check that memory allocation was successful. */
  if (oSymTable == NULL) {
    return NULL;
  }

  /* Initialize values of the new, empty SymTable. */
  oSymTable->psFirstNode = NULL;
  oSymTable->uLength = 0;

  return oSymTable;
}

/*--------------------------------------------------------------------*/

void SymTable_free(SymTable_T oSymTable) {
  /* Reference to current node to free. */
  struct SymTableNode *psCurrentNode; 

  /* Reference to the next node to free. */
  struct SymTableNode *psNextNode;

  assert(oSymTable != NULL);

  /* Iterate through linked list and free all nodes and their 
     defensive key copies. */
  for (psCurrentNode = oSymTable->psFirstNode;
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    /* Update to the next node in list. */
    psNextNode = psCurrentNode->psNextNode;

    /* Free the defensive copy of the the current node's key and free
       the node itself. */
    free(psCurrentNode->pcKey);
    free(psCurrentNode);
  }

  /* Free the "manager" structure for the SymTable ADT. */
  free(oSymTable);
}

/*--------------------------------------------------------------------*/

size_t SymTable_getLength(SymTable_T oSymTable) {
  assert(oSymTable != NULL);

  /* Return the length field tracked by the "manager" SymTable 
     structure. */
  return oSymTable->uLength;
}

/*--------------------------------------------------------------------*/

int SymTable_put(SymTable_T oSymTable, const char *pcKey, 
  const void *pvValue) 
{
  /* The new node to add to linked list. */
  struct SymTableNode *psNewNode;

  /* A defensive copy of the new node's key. */
  char *pcKeyCopy;

  assert(oSymTable != NULL);
  assert(pcKey != NULL);

  /* If a binding with the same key already exists in the SymTable, do
     nothing to SymTable. */
  if (SymTable_contains(oSymTable, pcKey) == 1)
    return 0;

  /* Allocate memory for new node, but store/pass by reference. */
  psNewNode = (struct SymTableNode*)malloc(sizeof(struct SymTableNode));

  /* Check that memory allocation for new node was successful. If not,
     there's nothing to put. */
  if (psNewNode == NULL) 
    return 0;

  /* Allocate memory for defensive key copy with same size as the 
     original key passed into function. */
  pcKeyCopy = (char *) malloc(sizeof(char) * (strlen(pcKey) + 1)); 

  /* Check that memory allocation for defensie key copy was 
     successful. If not, can't put in the new node. */
  if (pcKeyCopy == NULL) {
    free(psNewNode);
    return 0;
  }

  /* Make the defensive copy of key. */
  pcKeyCopy = strcpy(pcKeyCopy, pcKey);

  /* Initialize key/value of new node. */
  psNewNode->pcKey = pcKeyCopy;
  psNewNode->pvValue = (void *) pvValue;

  /* Insert new node at the start of the linked list. */
  psNewNode->psNextNode = oSymTable->psFirstNode;
  oSymTable->psFirstNode = psNewNode;

  /* Update the size of the SymTable. */
  oSymTable->uLength++;

  /* Put was successful. */
  return 1;
}

/*--------------------------------------------------------------------*/

void *SymTable_replace(SymTable_T oSymTable,
  const char *pcKey, const void *pvValue) 
{
  /* Current node being compared to target. */
  struct SymTableNode *psCurrentNode;

  /* Store the binding's previous value to return it. */
  void *pvOldValue;

  assert(oSymTable != NULL);
  assert(pcKey != NULL);

  /* Iterate through linked list and stop if target is found or end of
    list reached. */
  for (psCurrentNode = oSymTable->psFirstNode;
    psCurrentNode != NULL;
    psCurrentNode = psCurrentNode->psNextNode)
  {
    /* Target is found when keys match. */
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
      /* Store the binding's old value to be returned at end of
         function. */
      pvOldValue = psCurrentNode->pvValue;

      /* Update target binding's value. */
      psCurrentNode->pvValue = (void*)pvValue;

      /* Function was successful, return binding's old value. */
      return pvOldValue;
    }
  }

  /* Function was unsuccessful (target doesn't exist in SymTable). */
  return NULL;
}

/*--------------------------------------------------------------------*/

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
  /* Current node being compared to target. */
  struct SymTableNode *psCurrentNode;

  assert(oSymTable != NULL);
  assert(pcKey != NULL);

  /* Iterate through linked list and stop if target is found or end of
    list reached. */
  for (psCurrentNode = oSymTable->psFirstNode;
    psCurrentNode != NULL;
    psCurrentNode = psCurrentNode->psNextNode)
  {
    /* Target is found when keys match. */
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
      /* Successful target hit, return TRUE (1). */
      return 1;
  }

  /* Target was not found before the end of linked list, return FALSE 
     (0). */
  return 0;
}

/*--------------------------------------------------------------------*/

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
  /* Current node being compared to target. */
  struct SymTableNode *psCurrentNode;

  /* Track the next node in linked list to look at. */
  struct SymTableNode *psNextNode;

  assert(oSymTable != NULL);
  assert(pcKey != NULL);

  /* Iterate through linked list and stop if target is found or end of
    list reached. */
  for (psCurrentNode = oSymTable->psFirstNode;
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    /* Target is found when keys match. */
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
      /* Return the value of target binding. */
      return psCurrentNode->pvValue;
    
    /* Keep searching through linked list by jumping to next node. */
    psNextNode = psCurrentNode->psNextNode;
  }

  /* Target was not found before the end of linked list. No value to 
     return. */
  return NULL;
}

/*--------------------------------------------------------------------*/

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
  /* The node being compared to target. */
  struct SymTableNode *psCurrentNode;

  /* The node placed before the target node. */
  struct SymTableNode *psPreviousNode;
  
  /* Store the target binding's value. */
  void *pvReturnValue;

  assert(oSymTable != NULL);
  assert(pcKey != NULL);

  /* Iterate through linked list until end of list is reached or the
     target node is found. Track the previous node accordingly. */
  for (psCurrentNode = oSymTable->psFirstNode, psPreviousNode = NULL;
       psCurrentNode != NULL && 
       strcmp(psCurrentNode->pcKey, pcKey) != 0;
       psPreviousNode = psCurrentNode, 
       psCurrentNode = psCurrentNode->psNextNode);
  
  /* If the end of the list was reached, there's nothing to remove. */
  if (psCurrentNode == NULL) {
    return NULL;
  }
  /* If the node before the target is NULL, then the target was at the 
     start of the linked list. */
  else if (psPreviousNode == NULL) {
    /* Remove target node by replacing the first node in the list with 
       the second node in list. */
    oSymTable->psFirstNode = psCurrentNode->psNextNode;
  } 
  /* The target node was in the middle of the list. */
  else {
    /* Remove target node by connecting the node before the target node 
       to the node after the target node. */
    psPreviousNode->psNextNode = psCurrentNode->psNextNode;
  }

  /* Update the return value to be the target binding's value. */
  pvReturnValue = psCurrentNode->pvValue;

  /* Free the target node's defensive key copy, then free the target 
     node itself. */
  free(psCurrentNode->pcKey);
  free(psCurrentNode);

  /* Update the length of the linked list accordingly. */
  oSymTable->uLength--;

  /* Return the value of the binding which was removed. */
  return pvReturnValue;
}

/*--------------------------------------------------------------------*/

void SymTable_map(SymTable_T oSymTable,
  void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
  const void *pvExtra) 
{
  /* The current binding to apply pfApply to. */
  struct SymTableNode *psCurrentNode;

  assert(oSymTable != NULL);
  assert(pfApply != NULL);

  /* Iterate through linked list and apply pfApply to each node's 
    key/value with pvExtra. */
  for (psCurrentNode = oSymTable->psFirstNode;
       psCurrentNode != NULL;
       psCurrentNode = psCurrentNode->psNextNode)
  {
    /* Apply pfApply to the current node. */
    pfApply(psCurrentNode->pcKey, psCurrentNode->pvValue, 
      (void * ) pvExtra);
  }
}