/*--------------------------------------------------------------------*/
/* symtable.h                                                         */
/* Author: Connor Brown                                               */
/*--------------------------------------------------------------------*/

#include <stddef.h>

#ifndef SYMTABLE_INCLUDED
#define SYMTABLE_INCLUDED

/* A SymTable_T object is a collection of bindings with string keys and
   generic values. */

typedef struct SymTable *SymTable_T;

/*--------------------------------------------------------------------*/

/* Return a new SymTable_T object, or NULL if insufficient memory is
   available. */

SymTable_T SymTable_new(void);

/*--------------------------------------------------------------------*/

/* Free oSymTable. */

void SymTable_free(SymTable_T oSymTable);

/*--------------------------------------------------------------------*/

/* Returns the total number of bindings in oSymTable. */

size_t SymTable_getLength(SymTable_T oSymTable);

/*--------------------------------------------------------------------*/

/* Put a new binding, with pcKey as its key and pvValue as its value,
   into oSymTable. Returns 1 if put was successful, or 0 if insufficient
   memory is available. */

int SymTable_put(SymTable_T oSymTable,
  const char *pcKey, const void *pvValue);

/*--------------------------------------------------------------------*/

/* Finds a binding that exists in oSymTable which has the key pcKey and
   replaces its value with pvValue. Returns the binding's previous value
   before replacement, or NULL if the binding does not exist in 
   oSymTable. */

void *SymTable_replace(SymTable_T oSymTable,
  const char *pcKey, const void *pvValue);

/*--------------------------------------------------------------------*/

/* Returns 1 if a binding exists in oSymTable which has the key pcKey, 
   or 0 if no such binding exists in oSymTable. */

int SymTable_contains(SymTable_T oSymTable, const char *pcKey);

/*--------------------------------------------------------------------*/

/* Returns the value of a binding existing in oSymTable which has the 
   key pcKey, or NULL if no such binding exists in oSymTable. */

void *SymTable_get(SymTable_T oSymTable, const char *pcKey);

/*--------------------------------------------------------------------*/

/* Removes the binding in oSymTable which has the key pcKey. Returns the
   value of the binding if it exists in oSymTable before removal, or
   NULL if no such binding exists in oSymTable before attempting 
   removal. */

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey);

/*--------------------------------------------------------------------*/

/* Applies the function pfApply, with an optional paramter pvExtra, to 
   all bindings in oSymTable. */

void SymTable_map(SymTable_T oSymTable,
  void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
  const void *pvExtra);

#endif