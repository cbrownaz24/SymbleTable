#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

struct SymTableNode {
  char *pcKey;
  void *pvValue;
  struct SymTableNode *psNextNode;
};

struct SymTable {
  struct SymTableNode **psaNodeChains;
  size_t puBucketSizes[8];
  int iBucketSizeIndex;
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

static void SymTable_resizeIfNecessary(SymTable_T oSymTable) {
  struct SymTableNode **psNewBucketList;
  size_t uCurrentBucketCount, uNewBucketCount;
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;
  size_t uHashValue;
  int iBucketSizeIndex;
  size_t i;

  iBucketSizeIndex = oSymTable->iBucketSizeIndex;
  uCurrentBucketCount = oSymTable->puBucketSizes[iBucketSizeIndex];
  if (SymTable_getLength(oSymTable) != uCurrentBucketCount || 
      iBucketSizeIndex == 7) 
    return;

  
  uNewBucketCount = oSymTable->puBucketSizes[++iBucketSizeIndex];
  psNewBucketList = malloc(sizeof(struct SymTableNode *) * 
                           uNewBucketCount);
  if (psNewBucketList == NULL) {
    return;
  }

  for (i = 0; i < uNewBucketCount; i++) 
    psNewBucketList[i] = NULL;

  
  for (i = 0; i < uCurrentBucketCount; i++) {
    for (psCurrentNode = oSymTable->psaNodeChains[i]; 
         psCurrentNode != NULL; 
         psCurrentNode = psNextNode) 
    {
      psNextNode = psCurrentNode->psNextNode;
      uHashValue = SymTable_hash(psCurrentNode->pcKey, uNewBucketCount);

      psCurrentNode->psNextNode = psNewBucketList[uHashValue];
      psNewBucketList[uHashValue] = psCurrentNode;
    }
  }

  
    free(oSymTable->psaNodeChains);
    oSymTable->psaNodeChains = psNewBucketList;
    oSymTable->iBucketSizeIndex++;
}


SymTable_T SymTable_new(void) {
  SymTable_T oSymTable;
  int i;

  oSymTable = (SymTable_T) malloc(sizeof(struct SymTable));
  if (oSymTable == NULL) {
    return NULL;
  }

  oSymTable->psaNodeChains = malloc(sizeof(struct SymTableNode *)*509);
  if (oSymTable->psaNodeChains == NULL) {
    return NULL;
  }

  for (i = 0; i < 509; i++) {
    oSymTable->psaNodeChains[i] = NULL;
  }

  oSymTable->uLength = 0;
  oSymTable->iBucketSizeIndex = 0;

  oSymTable->puBucketSizes[0] = 509;
  oSymTable->puBucketSizes[1] = 1021;
  oSymTable->puBucketSizes[2] = 2039;
  oSymTable->puBucketSizes[3] = 4093;
  oSymTable->puBucketSizes[4] = 8191;
  oSymTable->puBucketSizes[5] = 16381;
  oSymTable->puBucketSizes[6] = 32749;
  oSymTable->puBucketSizes[7] = 65521;

  return oSymTable;
}

/*--------------------------------------------------------------------*/

void SymTable_free(SymTable_T oSymTable) {
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;
  int iBucketSizeIndex;
  size_t i;

  assert(oSymTable != NULL);

  iBucketSizeIndex = oSymTable->iBucketSizeIndex;
  for(i = 0; i < oSymTable->puBucketSizes[iBucketSizeIndex]; i++) {
    for (psCurrentNode = oSymTable->psaNodeChains[i];
      psCurrentNode != NULL;
      psCurrentNode = psNextNode)
    {
      psNextNode = psCurrentNode->psNextNode;
      free(psCurrentNode->pcKey);
      free(psCurrentNode);
    }
  }
  free(oSymTable->psaNodeChains);
  free(oSymTable);
}

/*--------------------------------------------------------------------*/

size_t SymTable_getLength(SymTable_T oSymTable) {
  return oSymTable->uLength;
}

/*--------------------------------------------------------------------*/

int SymTable_put(SymTable_T oSymTable, const char *pcKey, 
  const void *pvValue) 
{
  size_t uHashValue;
  char *pcKeyCopy;

  struct SymTableNode *psNewNode;
  struct SymTableNode *psPreviousFirst;

  int iBucketSizeIndex;

  assert(oSymTable != NULL);

  if (SymTable_contains(oSymTable, pcKey) == 1)
    return 0;

  iBucketSizeIndex = oSymTable->iBucketSizeIndex;
  uHashValue = SymTable_hash(pcKey, 
                            oSymTable->puBucketSizes[iBucketSizeIndex]);

  psNewNode = (struct SymTableNode*) malloc(sizeof(struct SymTableNode));
  pcKeyCopy = (char*) malloc(sizeof(char) * (strlen(pcKey) + 1)); 
  if (psNewNode == NULL || pcKeyCopy == NULL)
    return 0;
  
  pcKeyCopy = strcpy(pcKeyCopy, pcKey);

  psPreviousFirst = oSymTable->psaNodeChains[uHashValue];
  oSymTable->psaNodeChains[uHashValue] = psNewNode;
  psNewNode->psNextNode = psPreviousFirst;
  psNewNode->pcKey = pcKeyCopy;
  psNewNode->pvValue = (void*)pvValue;

  oSymTable->uLength++;

  SymTable_resizeIfNecessary(oSymTable);

  return 1;
}

/*--------------------------------------------------------------------*/

void *SymTable_replace(SymTable_T oSymTable,
  const char *pcKey, const void *pvValue) 
{
  size_t uHashValue;

  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;
  void *pvOldValue;
  int iBucketSizeIndex;

  assert(oSymTable != NULL);

  iBucketSizeIndex = oSymTable->iBucketSizeIndex;
  uHashValue = SymTable_hash(pcKey, 
                            oSymTable->puBucketSizes[iBucketSizeIndex]);
  for (psCurrentNode = oSymTable->psaNodeChains[uHashValue];
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
      pvOldValue = psCurrentNode->pvValue;
      psCurrentNode->pvValue = (void*)pvValue;
      return pvOldValue;
    }
    psNextNode = psCurrentNode->psNextNode;
  }
  return NULL;
}

/*--------------------------------------------------------------------*/

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
  size_t uHashValue;

  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;

  int iBucketSizeIndex;

  assert(oSymTable != NULL);

  iBucketSizeIndex = oSymTable->iBucketSizeIndex;
  uHashValue = SymTable_hash(pcKey, 
                            oSymTable->puBucketSizes[iBucketSizeIndex]);
  for (psCurrentNode = oSymTable->psaNodeChains[uHashValue];
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0) 
      return 1;
    psNextNode = psCurrentNode->psNextNode;
  }
  return 0;
}

/*--------------------------------------------------------------------*/

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
  size_t uHashValue;
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;

  int iBucketSizeIndex;

  assert(oSymTable != NULL);

  iBucketSizeIndex = oSymTable->iBucketSizeIndex;
  uHashValue = SymTable_hash(pcKey, 
                            oSymTable->puBucketSizes[iBucketSizeIndex]);
  for (psCurrentNode = oSymTable->psaNodeChains[uHashValue];
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0) 
      return psCurrentNode->pvValue;
    psNextNode = psCurrentNode->psNextNode;
  }
  return NULL;
}

/*--------------------------------------------------------------------*/

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
  size_t uHashValue;
  struct SymTableNode *psPreviousNode;
  struct SymTableNode *psCurrentNode;
  void *pvReturnValue;

  int iBucketSizeIndex;

  assert(oSymTable != NULL);

  iBucketSizeIndex = oSymTable->iBucketSizeIndex;
  uHashValue = SymTable_hash(pcKey, 
                            oSymTable->puBucketSizes[iBucketSizeIndex]);
  psCurrentNode = oSymTable->psaNodeChains[uHashValue];
  if (psCurrentNode == NULL)
    return NULL;
  else if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
    oSymTable->psaNodeChains[uHashValue] = psCurrentNode->psNextNode;
    pvReturnValue = psCurrentNode->pvValue;
    free(psCurrentNode->pcKey);
    free(psCurrentNode);
    oSymTable->uLength--;

    return pvReturnValue;
  }

  psPreviousNode = psCurrentNode;
  psCurrentNode = psCurrentNode->psNextNode;
  while (psCurrentNode != NULL) {
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
      psPreviousNode->psNextNode = psCurrentNode->psNextNode;
      pvReturnValue = psCurrentNode->pvValue;
      free(psCurrentNode->pcKey);
      free(psCurrentNode);

      oSymTable->uLength--;

      return pvReturnValue;
    }
    psPreviousNode = psCurrentNode;
    psCurrentNode = psCurrentNode->psNextNode;
  }
  return NULL;
}

/*--------------------------------------------------------------------*/

void SymTable_map(SymTable_T oSymTable,
  void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
  const void *pvExtra)
{
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;
  size_t i;

  assert(oSymTable != NULL);

  for(i = 0; i < 509; i++) {
    for (psCurrentNode = oSymTable->psaNodeChains[i];
      psCurrentNode != NULL;
      psCurrentNode = psNextNode)
    {
      pfApply(psCurrentNode->pcKey, 
              psCurrentNode->pvValue, 
              (void*)pvExtra);
      psNextNode = psCurrentNode->psNextNode;
    }
  }
}