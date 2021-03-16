#include "pch.h"
#include "LabelManager.h"

CLabelTableRefs g_labelRefs;    // таблица ссылок на метки, локальные и глобальные.
CLabelTable g_labelGlobalDefs;  // таблица глобальных меток
CLabelTable g_labelLocalDefs;   // таблица локальных меток, должна очищаться каждый раз при добавлении метки в таблицу глобальных меток
//#pragma warning(disable:4996)

#if (DEBUG_LABEL_MANAGER)
// /-----------------------debug---------------------------
FILE *dbgF = nullptr;
void DebugInit()
{
	dbgF = _wfopen(L"debug_lbl.log", L"wt");
}
// /-----------------------debug---------------------------
#endif
/*
Добавление метки в таблицу локальных меток
*/
bool AddLocalLabel(CBKToken *token, int value, uint32_t lt)
{
	bool bRet = g_labelLocalDefs.AddLabel(token, value, lt);
#if (DEBUG_LABEL_MANAGER)

	// /-----------------------debug---------------------------
	if (bRet)
	{
        fprintf(dbgF, L"\nAdd Local %06o: '%s'\t(%d)\n", value, token->getName().c_str(), g_labelLocalDefs.getSize());
	}
	else
	{
        fprintf(dbgF, L"\n!Add Local %06o: '%s'\t (%d) Already defined.\n", value, token->getName().c_str(), g_labelLocalDefs.getSize());
	}

	// /-----------------------debug---------------------------
#endif
	return bRet;
}

/*
Добавление метки в таблицу глобальных меток
*/
bool AddGlobalLabel(CBKToken *token, int value, uint32_t lt)
{
	bool bRet = false;

    bRet = g_labelGlobalDefs.AddLabel(token, value, lt);
    if (bRet)
	{
		g_labelLocalDefs.Clear();
	}

#if (DEBUG_LABEL_MANAGER)

	// /-----------------------debug---------------------------
	if (bRet)
	{
        fprintf(dbgF, L"\nAdd Global %06o: '%s'\t(%d)\n", value, token->getName().c_str(), g_labelGlobalDefs.getSize());
        fprintf(dbgF, L"Clear Local defs. (%d)\n", g_labelLocalDefs.getSize());
	}
	else
	{
        fprintf(dbgF, L"\n!Add Global %06o: '%s'\t(%d) Already defined.\n", value, token->getName().c_str(), g_labelGlobalDefs.getSize());
        fprintf(dbgF, L"!Not Clear Local defs. (%d)\n", g_labelLocalDefs.getSize());
	}

	// /-----------------------debug---------------------------
#endif
	return bRet;
}

/*
Добавление ссылки на метку в таблицу ссылок
*/
bool AddlLabelReference(CBKToken *token, int value, uint32_t lt)
{
	bool bRet = g_labelRefs.AddLabel(token, value, lt);
#if (DEBUG_LABEL_MANAGER)

	// /-----------------------debug---------------------------
	if (bRet)
	{
        fprintf(dbgF, L"\nAdd reference %06o: '%s' (%d)\n", value, token->getName().c_str(), g_labelRefs.getSize());
	}
	else
	{
        fprintf(dbgF, L"\nReferences. Unknown error.\n");
	}

	// /-----------------------debug---------------------------
#endif
	return bRet;
}

bool SearchLabelInTables(CBKToken *token, int &value, uint32_t &lt)
{
	value = 0;
#if (DEBUG_LABEL_MANAGER)
	// /-----------------------debug---------------------------
    fprintf(dbgF, L"\tSearch label: '%s'\n", token->getName().c_str());
	// /-----------------------debug---------------------------
#endif
	int n = g_labelLocalDefs.SearchLabel(token);

	if (n >= 0)
	{
		value = g_labelLocalDefs.GetValue(n);
		lt = g_labelLocalDefs.GetType(n);
#if (DEBUG_LABEL_MANAGER)
		// /-----------------------debug---------------------------
        fprintf(dbgF, L"\tFound in Local Table: %06o\n", value);
		// /-----------------------debug---------------------------
#endif
	}
	else
	{
		n = g_labelGlobalDefs.SearchLabel(token);

		if (n >= 0)
		{
			value = g_labelGlobalDefs.GetValue(n);
			lt = g_labelGlobalDefs.GetType(n);
#if (DEBUG_LABEL_MANAGER)
			// /-----------------------debug---------------------------
            fprintf(dbgF, L"\tFound in Global Table: %06o\n", value);
			// /-----------------------debug---------------------------
#endif
		}
	}

#if (DEBUG_LABEL_MANAGER)

	// /-----------------------debug---------------------------
	if (n < 0)
	{
        fprintf(dbgF, L"\tLabel not found.\n");
	}

	// /-----------------------debug---------------------------
#endif
	return (n >= 0);
}
