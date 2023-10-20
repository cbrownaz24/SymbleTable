#include <stddef.h>
#include <assert.h>

typedef struct SymTableNode_T {
  const char *pcKey;
  const void *pvValue;
  struct SymTableNode_T *psNextNode;
} SymTableNode_T;

typedef struct SymTable_T {
  SymTableNode_T *psFirstNode;
  size_t uLength;
} SymTable_T;

SymTable_T SymTable_new(void) {
  SymTable_T oSymTable = (SymTable_T) malloc(size_of(SymTable_T));
  if (oSymTable == NULL) {
    return NULL;
  } else {
    oSymTable->psFirstNode = NULL;
    oSymTable->uLength = 0;
    return oSymTable;
  }
}

void SymTable_free(SymTable_T oSymTable) {
  SymTableNode_T *psCurrentNode;
  SymTableNode_T *psNextNode;

  assert(oSymTable != NULL);

  for (psCurrentNode = oSymTable->psFirstNode;
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    psNextNode = psCurrentNode->psNextNode;
    free(psCurrentNode->pcKey);
    free(psCurrentNode);
  }

  free(oSymTable);
}

size_t SymTable_getLength(SymTable_T oSymTable) {
  return oSymTable->uLength;
}

int SymTable_put(SymTable_T oSymTable,
  const char *pcKey, const void *pvValue) 
{

  SymTableNode_T *psCurrentNode;
  SymTableNode_T *psNextNode;

  assert(oSymTable != NULL);

  for (psCurrentNode = oSymTable->psFirstNode;
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
      return 0;
    psNextNode = psCurrentNode->psNextNode;
  }

  SymTableNode_T psNewNode;
  char *pcKeyCopy;

  psNewNode = (SymTableNode_T) malloc(size_of(SymTable_T));
  pcKeyCopy = (char *) malloc(size_of(char) * (strlen(pcKey) + 1)); 
  if (psNewNode == NULL || pcKeyCopy == NULL)
    return 0;
  
  pcKeyCopy = strcpy(pcKeyCopy, pcKey);

  psNewNode->pcKey = pcKeyCopy;
  psNewNode->pvValue = pvValue;
  psNewNode->psNextNode = oSymTable->psFirstNode;
  oSymTable->psFirstNode = psNewNode;

  return 1;
}

void *SymTable_replace(SymTable_T oSymTable,
  const char *pcKey, const void *pvValue) 
{
  SymTableNode_T *psCurrentNode;
  SymTableNode_T *psNextNode;
  void *pvOldValue;

  assert(oSymTable != NULL);

  for (psCurrentNode = oSymTable->psFirstNode;
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0)
      pvOldValue = psCurrentNode->pvValue;
      psCurrentNode-pvValue = pvValue;
      return pvOldValue;
    psNextNode = psCurrentNode->psNextNode;
  }
  return NULL;
}

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
  SymTableNode_T *psCurrentNode;
  SymTableNode_T *psNextNode;

  assert(oSymTable != NULL);

  for (psCurrentNode = oSymTable->psFirstNode;
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
  SymTableNode_T *psCurrentNode;
  SymTableNode_T *psNextNode;

  assert(oSymTable != NULL);

  for (psCurrentNode = oSymTable->psFirstNode;
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
  SymTableNode_T *psPreviousNode;
  SymTableNode_T *psCurrentNode;
  SymTableNode_T *psNextNode;
  void *pvReturnValue;

  assert(oSymTable != NULL);

  psCurrentNode = oSymTable->psFirstNode;
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
  psCurrentNode = psCurrentNode -> psNextNode;
  while (psCurrentNode != NULL) {
    if (strcmp(psCurrentNode->pcKey, pcKey) == 0) {
      psPreviousNode->psNextNode = psCurrentNode->psNextNode;
      pvReturnValue = psCurrentNode->pvValue;
      free(psCurrentNode->pcKey);
      free(psCurrentNode);
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
  SymTableNode_T *psCurrentNode;
  SymTableNode_T *psNextNode;

  assert(oSymTable != NULL);

  for (psCurrentNode = oSymTable->psFirstNode;
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    pfApply(psCurrentNode->pcKey, psCurrentNode->pvValue, pvExtra);
    psNextNode = psCurrentNode->psNextNode;
  }
}