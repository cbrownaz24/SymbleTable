#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "symtable.h"

struct SymTableNode {
  char *pcKey;
  void *pvValue;
  struct SymTableNode *psNextNode;
};

struct SymTable {
  struct SymTableNode *psFirstNode;
  size_t uLength;
};

SymTable_T SymTable_new(void) {
  SymTable_T oSymTable = (SymTable_T)malloc(sizeof(struct SymTable));
  if (oSymTable == NULL) {
    return NULL;
  }

  oSymTable->psFirstNode = NULL;
  oSymTable->uLength = 0;
  return oSymTable;
}

void SymTable_free(SymTable_T oSymTable) {
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;

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
  struct SymTableNode *psNewNode;
  char *pcKeyCopy;

  assert(oSymTable != NULL);

  if (SymTable_contains(oSymTable, pcKey) == 1)
    return 0;

  psNewNode = (struct SymTableNode*)malloc(sizeof(struct SymTableNode));
  pcKeyCopy = (char*) malloc(sizeof(char) * (strlen(pcKey) + 1)); 
  if (psNewNode == NULL || pcKeyCopy == NULL)
    return 0;
  
  pcKeyCopy = strcpy(pcKeyCopy, pcKey);

  psNewNode->pcKey = pcKeyCopy;
  psNewNode->pvValue = (void*)pvValue;
  psNewNode->psNextNode = oSymTable->psFirstNode;
  oSymTable->psFirstNode = psNewNode;

  oSymTable->uLength++;

  return 1;
}

void *SymTable_replace(SymTable_T oSymTable,
  const char *pcKey, const void *pvValue) 
{
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;
  void *pvOldValue;

  assert(oSymTable != NULL);

  for (psCurrentNode = oSymTable->psFirstNode;
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

int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;

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
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;

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
  struct SymTableNode *psPreviousNode;
  struct SymTableNode *psCurrentNode;
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

void SymTable_map(SymTable_T oSymTable,
  void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
  const void *pvExtra) 
{
  struct SymTableNode *psCurrentNode;
  struct SymTableNode *psNextNode;

  assert(oSymTable != NULL);

  for (psCurrentNode = oSymTable->psFirstNode;
    psCurrentNode != NULL;
    psCurrentNode = psNextNode)
  {
    pfApply(psCurrentNode->pcKey, 
            psCurrentNode->pvValue, 
            (void*)pvExtra);
    psNextNode = psCurrentNode->psNextNode;
  }
}