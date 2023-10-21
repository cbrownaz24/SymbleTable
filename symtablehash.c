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
  size_t uLength;
};

enum HashTableSize {
  XXS = 509, 
  XS = 1021, 
  S = 2039, 
  M = 4093, 
  L = 8191, 
  XL = 16381, 
  XXL = 32749, 
  XXXL = 65521
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

static size_t SymTable_bucketCount(SymTable_T oSymTable) {
  return sizeof(*(oSymTable->psaNodeChains)) /
          sizeof(struct SymTableNode *);
}

/*--------------------------------------------------------------------*/

static size_t SymTable_sizeNeeded(SymTable_T oSymTable) {
  size_t uLength = SymTable_getLength(oSymTable);
  size_t uBucketCount = SymTable_bucketCount(oSymTable);

  if (uLength <= XXS && uBucketCount == XS) {
    return XXS;
  } else if (uLength >= XS && uBucketCount == XXS) {
    return XS;
  } else if (uLength <= XS && uBucketCount == S) {
    return XS;
  } else if (uLength >= S && uBucketCount == XS) {
    return S;
  } else if (uLength <= S && uBucketCount == M) {
    return S;
  } else if (uLength >= M && uBucketCount == S) {
    return M;
  } else if (uLength <= M && uBucketCount == L) {
    return M;
  } else if (uLength >= L && uBucketCount == M) {
    return L;
  } else if (uLength <= L && uBucketCount == XL) {
    return L;
  } else if (uLength >= XL && uBucketCount == L) {
    return XL;
  } else if (uLength <= XL && uBucketCount == XXL) {
    return XL;
  } else if (uLength >= XXL && uBucketCount == XL) {
    return XXL;
  } else if (uLength <= XXL && uBucketCount == XXXL) {
    return XXL;
  } else if (uLength >= XXXL && uBucketCount == XXL) {
    return XXXL;
  } else {
    return uBucketCount;
  }
}

/*--------------------------------------------------------------------*/

static SymTable_T SymTable_resized(SymTable_T oSymTable, 
  size_t uOriginalSize, size_t uNewSize) 
  {
  SymTable_T oNewSymTable;
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;
  size_t i;

  oNewSymTable = (SymTable_T) malloc(sizeof(struct SymTable));
  if (oNewSymTable == NULL) {
    return oSymTable;
  }

  oNewSymTable->psaNodeChains = 
    malloc(sizeof(struct SymTableNode) * uNewSize);
  if (oNewSymTable->psaNodeChains == NULL) {
    free(oNewSymTable);
    return oSymTable;
  }

  for (i = 0; i < uOriginalSize; i++) {
    for (psCurrentNode = *oSymTable->psaNodeChains;
         psCurrentNode != NULL;
         psCurrentNode = psNextNode) 
    {
      int putStatus = SymTable_put(oNewSymTable, 
                                   psCurrentNode->pcKey, 
                                   psCurrentNode->pvValue);
      if (putStatus == 0) {
        SymTable_free(oNewSymTable);
        return oSymTable;
      }
      psNextNode = psCurrentNode->psNextNode;
    }
    oSymTable->psaNodeChains++;
  }

  oNewSymTable->uLength = SymTable_getLength(oSymTable);

  return oNewSymTable;
}

/*--------------------------------------------------------------------*/

static void SymTable_resizeIfNeeded(SymTable_T oSymTable) {
  SymTable_T oResizedSymTable;
  size_t uSizeNeeded, uCurrentBucketCount;

  uSizeNeeded = SymTable_sizeNeeded(oSymTable);
  uCurrentBucketCount = SymTable_bucketCount(oSymTable);
  if (uSizeNeeded != uCurrentBucketCount) {
    oResizedSymTable = SymTable_resized(oSymTable, 
                                        uCurrentBucketCount, 
                                        uSizeNeeded);
    if (oResizedSymTable != oSymTable) {
      SymTable_T temp = oSymTable;
      oSymTable = oResizedSymTable;
      SymTable_free(temp);
    }
  }
}

/*--------------------------------------------------------------------*/

SymTable_T SymTable_new(void) {
  SymTable_T oSymTable;
  int i;

  oSymTable = (SymTable_T) malloc(sizeof(struct SymTable));
  if (oSymTable == NULL) {
    return NULL;
  }

  oSymTable->psaNodeChains = malloc(sizeof(struct SymTableNode) * XXS);
  if (oSymTable->psaNodeChains == NULL) {
    return NULL;
  }

  for (i = 0; i < XXS; i++) {
    oSymTable->psaNodeChains[i] = NULL;
  }

  oSymTable->uLength = 0;

  return oSymTable;
}

/*--------------------------------------------------------------------*/

void SymTable_free(SymTable_T oSymTable) {
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;
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
  free(oSymTable->psaNodeChains);
  free(oSymTable);
}

/*--------------------------------------------------------------------*/

size_t SymTable_getLength(SymTable_T oSymTable) {
  return oSymTable->uLength;
}

/*--------------------------------------------------------------------*/

int SymTable_put(SymTable_T oSymTable,
  const char *pcKey, const void *pvValue) 
{
  size_t uHashValue;
  char *pcKeyCopy;

  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;

  struct SymTableNode *psNewNode;
  struct SymTableNode *psPreviousFirst;

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

  SymTable_resizeIfNeeded(oSymTable);

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

  assert(oSymTable != NULL);

  uHashValue = SymTable_hash(pcKey, 509);
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

/*--------------------------------------------------------------------*/

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
  size_t uHashValue;
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;

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

/*--------------------------------------------------------------------*/

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
  size_t uHashValue;
  struct SymTableNode *psPreviousNode;
  struct SymTableNode *psCurrentNode;
  void *pvReturnValue;

  assert(oSymTable != NULL);

  uHashValue = SymTable_hash(pcKey, 509);
  psCurrentNode = oSymTable->psaNodeChains[uHashValue];
  if (psCurrentNode == NULL)
    return NULL;
  else if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
    oSymTable->psaNodeChains[uHashValue] = psCurrentNode->psNextNode;
    pvReturnValue = psCurrentNode->pvValue;
    free(psCurrentNode->pcKey);
    free(psCurrentNode);
    oSymTable->uLength--;

    SymTable_resizeIfNeeded(oSymTable);

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

      SymTable_resizeIfNeeded(oSymTable);

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
  int i;

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