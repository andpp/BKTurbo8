#pragma once

#include "Reader.h"

// тут будут храниться всякие глобальные параметры и флаги
struct GlobalParams
{
    std::string strAppPath;    // домашний путь
	bool bVerbosity;
	bool bInString;             // флаг, что находимся внутри строки "" для ascii, asciz и rad50
	bool bOperandType;          // флаг, какой операнд. false = dd, true = ss, где OPssdd
	bool bENDReached;           // флаг, что встретилась псевдокоманда .end
	bool bStopOnError;          // флаг остановки компиляции при первой встреченной ошибке
	bool bLinkerFirst;          // флаг первого слинкованного модуля
	int nModeLinkage;           // режим компоновки меток: -1 = CL, 1 = CO, 0 = окончательная компоновка после .end
	int nAriphmType;            // тип арифметического выражения, где оно встречается: -1 = константа, 1 - word или ассемблерная команда, 0 - все остальное
	int nStartAddress;          // смещение адреса начала трансляции проги относительно 01000 == по-умолчанию
	int nPC;                    // текущий PC при компиляции, измеряется в байтах.
	int nProgramLength;         // длина компилируемой программы
	int nError;                 // флаг возникновения ошибки, и одновременно счётчик.
	bool bStepInclude;          // флаг, который говорит, чо мы вошли в новую инклуду и надо начать цикл сначала
	bool bInvert;               // флаг оператора ^C, означает, что следующий оператор надо инвертировать
};


extern GlobalParams g_GlobalParameters;

union UMemory
{
	uint16_t w[32768];
	uint8_t  b[65536];
};

extern UMemory g_Memory;
extern CReader *g_pReader;
extern std::vector <CReader *> v_Reader;
extern CReader::FILE_CHARSET Charset;

constexpr auto BASE_ADDRESS = 0;
constexpr auto START_ADDRESS_OFFSET = 01000;
constexpr auto HIGH_BOUND = 0177000;

void InitGlobalParameters(GlobalParams *gp);
void ReInitGlobalParameters(GlobalParams *gp);
void SetStartAddress(int addr);
int GetStartAddress();
int CorrectAddress(int addr);
int CorrectOffset(int addr);

