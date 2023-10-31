/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
/* Author: Connor Brown                                               */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

/*--------------------------------------------------------------------*/

/* The different bucket counts that the hash table can have as it 
   grows. */
static const size_t auBucketCounts[8] = {509, 1021, 2039, 4093, 8191, 
                                         16381, 32749, 65521};

/*--------------------------------------------------------------------*/

/* The total number of bucket sizes in hash table. */
static const size_t numBucketCounts = sizeof(auBucketCounts) / 
                                      sizeof(auBucketCounts[0]);

/*--------------------------------------------------------------------*/

/* Each item stored in a SymTable. SymTableNodes are linked to form a 
   chain connected to a bucket element in an array of buckets. */

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
   SymTableNode in each chain associated with each bucket; it stores an 
   index for accessing the current bucket size from auBucketCounts; it 
   tracks the total number of elements (length) in the SymTable. */

struct SymTable {
  /* Array of "buckets" which each have an associated chain of nodes. */
  struct SymTableNode **psaNodeChains;

  /* The index of the current bucket size in auBucketCounts. */
  size_t iBucketSizeIndex;

  /* The total number of bindings in SymTable. */
  size_t uLength;
};

/*--------------------------------------------------------------------*/

/* Return a hash code for pcKey that is between 0 and uBucketCount-1,
  inclusive. */
static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
   const size_t HASH_MULTIPLIER = 65599;
   size_t u;
   size_t uHash = 0;

   assert(pcKey != NULL);

   for (u = 0; pcKey[u] != '\0'; u++)
      uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

   return uHash % uBucketCount;
}

/*--------------------------------------------------------------------*/

/* Resizes the hash table associated with the SymTable ADT referenced 
   by oSymTable. If the hash table is already maximally sized (maxed 
   number of buckets), then no resize will occur. */
static void SymTable_resizeIfNecessary(SymTable_T oSymTable) {
  /* The resized buckets array. */
  struct SymTableNode **psNewBucketList;

  /* The current bucket count and new/expanded bucket count. */
  size_t uCurrentBucketCount, uNewBucketCount;

  /* The current node being rehashed into new buckets array and the next
     node to be rehashed into new buckets array. */
  struct SymTableNode *psCurrentNode, *psNextNode;

  /* The new hash index of the current node. */
  size_t uHashValue;

  /* The index correpsonding to current and next bucket size in 
     SymTable's bucket size array. */
  size_t iBucketSizeIndex; 

  /* Iterator variable to access each bucket in both the original and 
     new bucket arrays. */
  size_t i;

  assert(oSymTable != NULL);

  /* Determine the current bucket count. */
  iBucketSizeIndex = oSymTable->iBucketSizeIndex;
  uCurrentBucketCount = auBucketCounts[iBucketSizeIndex];

  /* Proceed with the resize iff the number of bindings equals the 
     current number of buckets or the bucket count is maxed. Otherwise, 
     stop the function call. */
  if (SymTable_getLength(oSymTable) != uCurrentBucketCount || 
      iBucketSizeIndex == numBucketCounts - 1) 
    return;

  /* Determine the new bucket count by incrementing to the next highest 
     bucket count. */
  uNewBucketCount = auBucketCounts[++iBucketSizeIndex];

  /* Allocate memory for the new buckets array according to the new 
     bucket count. */
  psNewBucketList = calloc(uNewBucketCount, 
                      sizeof(struct SymTableNode *));

  /* Check that memory was allocated successfully. */
  if (psNewBucketList == NULL) {
    return;
  }

  /* Iterate through all bindings by iterating through each of the 
     original buckets, then iterating through each of those buckets' 
     node chains. Rehash each of the nodes and place them into the new 
     buckets array accordingly. */
  for (i = 0; i < uCurrentBucketCount; i++) {
    for (psCurrentNode = oSymTable->psaNodeChains[i]; 
         psCurrentNode != NULL; 
         psCurrentNode = psNextNode) 
    {
      /* Update the next node to be rehashed.  */
      psNextNode = psCurrentNode->psNextNode;

      /* Calculate the new hash value of the current node using the 
         resized bucket count. */
      uHashValue = SymTable_hash(psCurrentNode->pcKey, uNewBucketCount);

      /* Insert the node into the new buckets array by placing it
         at the start of its rehashed buckets' node chain. */
      psCurrentNode->psNextNode = psNewBucketList[uHashValue];
      psNewBucketList[uHashValue] = psCurrentNode;
    }
  }

    /* Free the previous buckets array and update the SymTable's buckets
       array to be the new one. */
    free(oSymTable->psaNodeChains);
    oSymTable->psaNodeChains = psNewBucketList;

    /* Increment the bucket size of SymTable to be the next greatest 
       size allowed. */
    oSymTable->iBucketSizeIndex++;
}


SymTable_T SymTable_new(void) {
  /* Reference to struct SymTable "manager" of given SymTable 
     instance. */
  SymTable_T oSymTable;

  /* Allocate memory for SymTable "manager". */
  oSymTable = (SymTable_T) malloc(sizeof(struct SymTable));

  /* Check that memory allocation was successful. */
  if (oSymTable == NULL) {
    return NULL;
  }

  /* Allocate memory for buckets array and initialize buckets count to 
     be the smallest possible bucket count. */
  oSymTable->psaNodeChains = calloc(sizeof(struct SymTableNode *), 
                                    auBucketCounts[0]);

  /* Check that memory allocation for buckets array was successful. If 
     not, SymTable cannot be created either. */
  if (oSymTable->psaNodeChains == NULL) {
    free(oSymTable);
    return NULL;
  }

  /* Initialize the length of the new, empty SymTable to be 0. */
  oSymTable->uLength = 0;

  /* Initialize the current size of the bucket count to be the smallest
  possible. */
  oSymTable->iBucketSizeIndex = 0;

  return oSymTable;
}

/*--------------------------------------------------------------------*/

void SymTable_free(SymTable_T oSymTable) {
  /* Reference to current node to free and to the next node to free. */
  struct SymTableNode *psCurrentNode, *psNextNode;

  /* Record the current number of buckets in SymTable. */
  size_t iBucketSizeIndex;

  /* Incrementor to iterate through each bucket to free each node 
     chain. */
  size_t i;

  assert(oSymTable != NULL);

  /* Get the index corresponding to bucket count. */
  iBucketSizeIndex = oSymTable->iBucketSizeIndex;

  /* Iterate through each bucket to free each node chain. */
  for(i = 0; i < auBucketCounts[iBucketSizeIndex]; i++) {
    /* Iterate through the current bucket's node chain to free the 
       chain. */
    for (psCurrentNode = oSymTable->psaNodeChains[i];
      psCurrentNode != NULL;
      psCurrentNode = psNextNode)
    {
      /* Find the next node to free. */
      psNextNode = psCurrentNode->psNextNode;

      /* Free node's defensive key copy and node itself. */
      free(psCurrentNode->pcKey);
      free(psCurrentNode);
    }
  }

  /* All nodes/bindings are freed, so free the buckets and free the 
     "mananger" struct. */
  free(oSymTable->psaNodeChains);
  free(oSymTable);
}

/*--------------------------------------------------------------------*/

size_t SymTable_getLength(SymTable_T oSymTable) {
  assert(oSymTable != NULL);

  /* Return the uLength field tracked by the "manager" SymTable 
     structure. */
  return oSymTable->uLength;
}

/*--------------------------------------------------------------------*/

int SymTable_put(SymTable_T oSymTable, const char *pcKey, 
  const void *pvValue) 
{
  /* The hash value corresponding to which bucket to add node to. */
  size_t uHashValue;

  /* Defensive copy of the new binding's key. */
  char *pcKeyCopy;

  /* The node corresponding to the new binding. */
  struct SymTableNode *psNewNode;

  /* Record the current number of buckets in SymTable. */
  size_t iBucketSizeIndex;

  assert(oSymTable != NULL);
  assert(pcKey != NULL);

  /* Check if a binding with the same key already exists in the 
     SymTable. */
  if (SymTable_contains(oSymTable, pcKey) == 1)
    /* If a binding with the same key does exist, put fails and SymTable
       is unchanged. */
    return 0;

  /* Get the index corresponding to bucket count. */
  iBucketSizeIndex = oSymTable->iBucketSizeIndex;

  /* Calculate which bucket to add node to. */
  uHashValue = SymTable_hash(pcKey, auBucketCounts[iBucketSizeIndex]);

  /* Allocate memory for new node. */
  psNewNode = (struct SymTableNode*)malloc(sizeof(struct SymTableNode));

  /* Check that memory allocation was successful. If not, cannot add 
     node and put fails. */
  if (psNewNode == NULL)
    return 0;

  /* Allocate memory for defensive key copy. */
  pcKeyCopy = (char *) malloc(sizeof(char) * (strlen(pcKey) + 1)); 

  /* Check that memory allocation was successful. If not, then unable to
     create node, so free the node from memory.*/
  if (pcKeyCopy == NULL) {
    free(psNewNode);
    return 0;
  }

  /* Add new node to front of node chain in its bucket. */
  psNewNode->psNextNode = oSymTable->psaNodeChains[uHashValue];
  oSymTable->psaNodeChains[uHashValue] = psNewNode;

  /* Add defensive key copy to new node. */
  pcKeyCopy = strcpy(pcKeyCopy, pcKey);
  psNewNode->pcKey = pcKeyCopy;

  /* Initialize its value accordingly. */
  psNewNode->pvValue = (void *) pvValue;

  /* Update the total number of bindings in SymTable. */
  oSymTable->uLength++;

  /* Resize the hash table accordingly. */
  SymTable_resizeIfNecessary(oSymTable);

  /* Put was successful. */
  return 1;
}

/*--------------------------------------------------------------------*/

void *SymTable_replace(SymTable_T oSymTable,
  const char *pcKey, const void *pvValue) 
{
  /* The hash value corresponding to which bucket to add node to. */
  size_t uHashValue;

  /* The node being compared to the target. */
  struct SymTableNode *psCurrentNode;

  /* The previous value of the target binding before replacing. */
  void *pvOldValue;

  /* Record the current number of buckets in SymTable. */
  size_t iBucketSizeIndex;

  assert(oSymTable != NULL);
  assert(pcKey != NULL);

  /* The bucket size is tracked in the SymTable "manager" struct. */
  iBucketSizeIndex = oSymTable->iBucketSizeIndex;

  /* Calculate which bucket to search for target in. */
  uHashValue = SymTable_hash(pcKey, auBucketCounts[iBucketSizeIndex]);
  
  /* Iterate only through the one node chain which the target can be 
     found in. */
  for (psCurrentNode = oSymTable->psaNodeChains[uHashValue];
    psCurrentNode != NULL;
    psCurrentNode = psCurrentNode->psNextNode)
  {
    /* Target hit success when keys match. */
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
      /* Store the binding's previous value. */
      pvOldValue = psCurrentNode->pvValue;

      /* Replace the binding's value. */
      psCurrentNode->pvValue = (void*)pvValue;

      /* Return the previous value that was replaced. */
      return pvOldValue;
    }
  }

  /* Target does not exist in SymTable, no value to replace. */
  return NULL;
}

/*--------------------------------------------------------------------*/

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
  /* The hash value corresponding to which bucket to add node to. */
  size_t uHashValue;

  /* The node being compared to the target. */
  struct SymTableNode *psCurrentNode;

  /* Record the current number of buckets in SymTable. */
  size_t iBucketSizeIndex;

  assert(oSymTable != NULL);
  assert(pcKey != NULL);

  /* The bucket size is tracked in the SymTable "manager" struct. */
  iBucketSizeIndex = oSymTable->iBucketSizeIndex;

  /* Calculate which bucket to search for target in. */
  uHashValue = SymTable_hash(pcKey, auBucketCounts[iBucketSizeIndex]);
  
  /* Iterate only through the one node chain which the target can be 
     found in. */
  for (psCurrentNode = oSymTable->psaNodeChains[uHashValue];
    psCurrentNode != NULL;
    psCurrentNode = psCurrentNode->psNextNode)
  {
    /* Target hit success when keys match. */
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0) 
      /* Target hit success is TRUE (1). */
      return 1;
  }

  /* Target hit fail, return FALSE (0). */
  return 0;
}

/*--------------------------------------------------------------------*/

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
  /* The hash value corresponding to which bucket to add node to. */
  size_t uHashValue;

  /* The node being compared to the target. */
  struct SymTableNode *psCurrentNode;

  /* Record the current number of buckets in SymTable. */
  size_t iBucketSizeIndex;

  assert(oSymTable != NULL);
  assert(pcKey != NULL);

  /* The bucket size is tracked in the SymTable "manager" struct. */
  iBucketSizeIndex = oSymTable->iBucketSizeIndex;

  /* Calculate which bucket to search for target in. */
  uHashValue = SymTable_hash(pcKey, auBucketCounts[iBucketSizeIndex]);

  /* Iterate only through the one node chain which the target can be 
     found in. */
  for (psCurrentNode = oSymTable->psaNodeChains[uHashValue];
    psCurrentNode != NULL;
    psCurrentNode = psCurrentNode->psNextNode)
  {
    /* Target hit success when keys match. */
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0) 
      /* Give the value of the target binding. */
      return psCurrentNode->pvValue;
  }

  /* Target does not exist in SymTable, nothing to return. */
  return NULL;
}

/*--------------------------------------------------------------------*/

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
  /* The hash value corresponding to which bucket to add node to. */
  size_t uHashValue;

  /* The node right before the current node being looked at. */
  struct SymTableNode *psPreviousNode;

  /* The node being compared to the target. */
  struct SymTableNode *psCurrentNode;

  /* The value of the target binding before removing. */
  void *pvReturnValue;

  /* Record the current number of buckets in SymTable. */
  size_t iBucketSizeIndex;

  assert(oSymTable != NULL);
  assert(pcKey != NULL);

  /* The bucket size is tracked in the SymTable "manager" struct. */
  iBucketSizeIndex = oSymTable->iBucketSizeIndex;

  /* Calculate which bucket to search for target in. */
  uHashValue = SymTable_hash(pcKey, auBucketCounts[iBucketSizeIndex]);

  /* Iterate through node chain until end of chain is reached or the
     target node is found. Track the previous node accordingly. */
  for (psCurrentNode = oSymTable->psaNodeChains[uHashValue], 
       psPreviousNode = NULL; 
       psCurrentNode != NULL && 
       strcmp(psCurrentNode->pcKey, pcKey) != 0;
       psPreviousNode = psCurrentNode, 
       psCurrentNode = psCurrentNode->psNextNode);

  /* If the end of the list was reached, there's nothing to remove. */
  if (psCurrentNode == NULL) {
    return NULL;
  }
  /* If the node before the target is NULL, then the target was at the 
     start of the node chain. */
  else if (psPreviousNode == NULL) {
    /* Remove target node by replacing the first node in the node chain 
       with the second node in chain. */
    oSymTable->psaNodeChains[uHashValue] = psCurrentNode->psNextNode;
  } 
  /* The target node was in the middle of the chain. */
  else {
    /* Remove target node by connecting the node before the target node 
       to the node after the target node. */
    psPreviousNode->psNextNode = psCurrentNode->psNextNode;
  }

  /* Update the return value to be the target binding's value. */
  pvReturnValue = psCurrentNode->pvValue;

  /* Free the target node's defnesive key copy, then free the target 
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

  /* Record the current number of buckets in SymTable. */
  size_t iBucketSizeIndex;

  /* Incrementor to iterate over all buckets/node chains in SymTable. */
  size_t i; 

  assert(oSymTable != NULL);
  assert(pfApply != NULL);

  /* The bucket size is tracked in the SymTable "manager" struct. */
  iBucketSizeIndex = oSymTable->iBucketSizeIndex;

  /* Iterate through each bucket and each node in each buckets' node 
     chain and apply pfApply to each node's key/value with pvExtra. */
  for(i = 0; i < auBucketCounts[iBucketSizeIndex]; i++) {
    for (psCurrentNode = oSymTable->psaNodeChains[i];
      psCurrentNode != NULL;
      psCurrentNode = psCurrentNode->psNextNode)
    {
      /* Apply pfApply to the current node. */
      pfApply(psCurrentNode->pcKey, 
              psCurrentNode->pvValue, 
              (void*)pvExtra);
    }
  }
}