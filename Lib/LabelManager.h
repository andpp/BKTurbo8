#pragma once
#include "LabelTable.h"

#define DEBUG_LABEL_MANAGER 0

extern CLabelTableRefs g_labelRefs;     // таблица ссылок на метки, локальные и глобальные.
extern CLabelTable g_labelGlobalDefs;   // таблица глобальных меток
extern CLabelTable g_labelLocalDefs;    // таблица локальных меток
class CReader;

bool AddLocalLabel(CBKToken *token, int value, uint32_t lt);
bool AddGlobalLabel(CBKToken *token, int value, uint32_t lt);
bool AddlLabelReference(CBKToken *token, int value, uint32_t lt);
// поиск метки в таблицах
bool SearchLabelInTables(CBKToken *token, int &value, uint32_t &lt);


#if (DEBUG_LABEL_MANAGER)
// /-----------------------debug---------------------------
extern FILE *dbgF;
void DebugInit();
// /-----------------------debug---------------------------
#endif
