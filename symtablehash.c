#include <stddef.h>
#include <assert.h>
#include "symtable.h"

struct SymTableNode {
  const char *pcKey;
  const void *pvValue;
  struct SymTableNode *psNextNode;
};

struct SymTable {
  SymTableNode_T *psaNodeChains[509];
  size_t uLength;
};

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

SymTable_T SymTable_new(void) {
  SymTable_T oSymTable;

  oSymTable = (SymTable_T) malloc(sizeof(struct SymTable));
  if (oSymTable == NULL) {
    return NULL;
  }

  int i;
  for (i = 0; i < 509; i++) {
    oSymTable->psaNodeChains[i] = NULL;
  }

  oSymTable->uLength = 0;

  return oSymTable;
}

void SymTable_free(SymTable_T oSymTable) {
  SymTableNode *psCurrentNode;
  SymTableNode *psNextNode;
  int i;

  assert(oSymTable != NULL);

  for(i = 0; i < 509; i++) {
    for (psCurrentNode = oSymTable->psaNodeChains[i];
      psCurrentNode != NULL;
      psCurrentNode = psNextNode)
    {
      psNextNode = psCurrentNode->psNextNode;
      free(psCurrentNode->pcKey);
      free(psCurrentNode);
    }
  }

  free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable) {
  return oSymTable->uLength;
}

int SymTable_put(SymTable_T oSymTable,
  const char *pcKey, const void *pvValue) 
{
  size_t uHashValue;
  char *pcKeyCopy;

  SymTableNode *psCurrentNode;
  SymTableNode *psNextNode;

  assert(oSymTable != NULL);

  uHashValue = SymTable_hash(pcKey, 509);
  for (psCurrentNode = oSymTable->psaNodeChains[uHashValue];
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
      return 0;
    psNextNode = psCurrentNode->psNextNode;
  }

  SymTableNode *psNewNode;
  SymTableNode *psPreviousFirst;
  char *pcKeyCopy;

  psNewNode = (struct SymTableNode*) malloc(size_of(struct SymTableNode));
  pcKeyCopy = (char*) malloc(size_of(char) * (strlen(pcKey) + 1)); 
  if (psNewNode == NULL || pcKeyCopy == NULL)
    return 0;
  
  pcKeyCopy = strcpy(pcKeyCopy, pcKey);

  psPreviousFirst = oSymTable->psaNodeChains[uHashValue];
  oSymTable->psaNodeChains[uHashValue] = psNewNode;
  psNewNode->psNextNode = psPreviousFirst;
  psNewNode->pcKey = pcKeyCopy;
  psNewNode->pvValue = pvValue;

  oSymTable->uLength++;

  return 1;
}

void *SymTable_replace(SymTable_T oSymTable,
  const char *pcKey, const void *pvValue) 
{
  size_t uHashValue;
  char *pcKeyCopy;

  SymTableNode *psCurrentNode;
  SymTableNode *psNextNode;

  assert(oSymTable != NULL);

  uHashValue = SymTable_hash(pcKey, 509);
  for (psCurrentNode = oSymTable->psaNodeChains[uHashValue];
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
      pvOldValue = psCurrentNode->pvValue;
      psCurrentNode-pvValue = pvValue;
      return pvOldValue;
    }
    psNextNode = psCurrentNode->psNextNode;
  }
  return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
  size_t uHashValue;
  char *pcKeyCopy;

  SymTableNode *psCurrentNode;
  SymTableNode *psNextNode;

  assert(oSymTable != NULL);

  uHashValue = SymTable_hash(pcKey, 509);
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

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
  size_t uHashValue;
  SymTableNode *psCurrentNode;
  SymTableNode *psNextNode;

  assert(oSymTable != NULL);

  uHashValue = SymTable_hash(pcKey, 509);
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

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
  size_t uHashValue;
  SymTableNode *psPreviousNode;
  SymTableNode *psCurrentNode;
  SymTableNode *psNextNode;
  void *pvReturnValue;

  assert(oSymTable != NULL);

  uHashValue = SymTable_hash(pcKey, 509);
  psCurrentNode = oSymTable->psaNodeChains[uHashValue];
  if (psCurrentNode == NULL)
    return NULL;
  else if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
    oSymTable->psFirstNode = psCurrentNode->psNextNode;
    pvReturnValue = psCurrentNode->pvValue;
    free(psCurrentNode->pcKey);
    free(psCurrentNode);
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

void SymTable_map(SymTable_T oSymTable,
  void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
  const void *pvExtra)
{
  SymTableNode *psCurrentNode;
  SymTableNode *psNextNode;
  int i;

  assert(oSymTable != NULL);

  for(i = 0; i < 509; i++) {
    for (psCurrentNode = oSymTable->psaNodeChains[i];
      psCurrentNode != NULL;
      psCurrentNode = psNextNode)
    {
      pfApply(psCurrentNode->pcKey, psCurrentNode->pvValue, pvExtra);
      psNextNode = psCurrentNode->psNextNode;
    }
  }
}