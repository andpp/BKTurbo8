#include "pch.h"
#include "Globals.h"

GlobalParams g_GlobalParameters;

UMemory g_Memory;

CReader *g_pReader = nullptr; // текущий обрабатываемый файл
std::vector <CReader *> v_Reader; // вектор, куда помещаются реадеры
CReader::FILE_CHARSET Charset = CReader::FILE_CHARSET::UNDEFINE;

void InitGlobalParameters(GlobalParams *gp)
{
	gp->bVerbosity = false;
	gp->bInString  = false;         // флаг, что находимся внутри строки "" для ascii, asciz и rad50
	gp->bOperandType = false;       // флаг, какой операнд. 0 = dd, !0 = ss, где OPssdd
	gp->bENDReached = false;        // флаг, что встретилась псевдокоманда .end
	gp->bStopOnError = false;       // флаг остановки компиляции при первой встреченной ошибке
	gp->bLinkerFirst = true;        // флаг первого слинкованного модуля
	gp->nModeLinkage = 0;           // режим компоновки меток: -1 = CL, 1 = CO, 0 = окончательная компоновка после .end
	gp->nAriphmType = 0;            // тип арифметического выражения, где оно встречается: -1 = константа, 1 - word или ассемблерная команда, 0 - все остальное
	gp->nStartAddress = START_ADDRESS_OFFSET;     // смещение адреса начала трансляции проги относительно START_ADDRESS, 0 == по-умолчанию
	gp->nPC = BASE_ADDRESS;         // текущий PC при компиляции
	gp->nProgramLength = 0;         // длина компилируемой программы
	gp->nError = 0;
	gp->bStepInclude = false;
	gp->bInvert = false;
	memset(&(g_Memory.b), 0, 65536);
}
// переинициализация. некоторых параметров.
void ReInitGlobalParameters(GlobalParams *gp)
{
	gp->bInString = false;          // флаг, что находимся внутри строки "" для ascii, asciz и rad50
	gp->bOperandType = false;       // флаг, какой операнд. 0 = dd, !0 = ss, где OPssdd
	gp->bENDReached = false;        // флаг, что встретилась псевдокоманда .end
	gp->bStopOnError = false;       // флаг остановки компиляции при первой встреченной ошибке
	gp->bLinkerFirst = true;        // флаг первого слинкованного модуля
	gp->nModeLinkage = 0;           // режим компоновки меток: -1 = CL, 1 = CO, 0 = окончательная компоновка после .end
	gp->nAriphmType = 0;            // тип арифметического выражения, где оно встречается: -1 = константа, 1 - word или ассемблерная команда, 0 - все остальное
	gp->nPC = BASE_ADDRESS;         // текущий PC при компиляции
	gp->nProgramLength = 0;         // длина компилируемой программы
	gp->nError = 0;
	gp->bInvert = false;
	memset(&(g_Memory.b), 0, 65536);
}

void SetStartAddress(int addr)
{
	g_GlobalParameters.nStartAddress = addr;
}
int GetStartAddress()
{
	return g_GlobalParameters.nStartAddress;
}

int CorrectAddress(int addr)
{
	return addr - BASE_ADDRESS + g_GlobalParameters.nStartAddress;
}
int CorrectOffset(int addr)
{
	return addr - BASE_ADDRESS - g_GlobalParameters.nStartAddress;
}
