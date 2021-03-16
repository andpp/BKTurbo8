#pragma once

#include "BKToken.h"

struct Registers
{
	std::wstring name;
	int nNum;
};

extern const wchar_t RADIX50[050];

bool AriphmParser(int &cp, int &result, wchar_t &ch);
// bool NumberParser(int &result, wchar_t &ch);
bool trp242(int &cp, int &result, wchar_t &ch);
bool trp240(CBKToken *lbl, int &cp, int &result, wchar_t &ch, uint32_t lt);
bool trp202(CBKToken *token, int &result, wchar_t &ch);
bool ReadRegName(wchar_t &ch);
bool ParseRegName(CBKToken *reg);
void SetAddresationRegister(int data);
int GetAddresationRegister();
int CheckReg(wchar_t &ch);
bool Operand_analyse(int &cp, wchar_t &ch);
bool needChar(wchar_t nch, wchar_t &ch);

bool parse_float(wchar_t &ch, int size, uint16_t *flt);
bool AdvancedNumberParser(int &result, wchar_t &ch);
bool Macro11NumberParser(int &result, wchar_t &ch, bool &bInvert);
