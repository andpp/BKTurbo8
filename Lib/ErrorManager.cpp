#include "pch.h"
#include "ErrorManager.h"
#include "Globals.h"
#include "Listing.h"

//#pragma warning(disable:4996)

BKTurboAsm_Error errors[] =
{
    { ERROR_100 + 100, "Нет места для меток." },
    { ERROR_101 + 100, "Псевдокоманда .LA должна быть первой в тексте." },
    { ERROR_102 + 100, "Ошибка длины или направления перехода в команде SOB." },
    { ERROR_103 + 100, "Недопустимый символ в строке." },
    { ERROR_104 + 100, "Отсутствие метки." },
    { ERROR_105 + 100, "Ошибка или отсутствие числового аргумента." },
    { ERROR_106 + 100, "Неправильная псевдокоманда." },
    { ERROR_107 + 100, "Неправильная ассемблерная инструкция." },
    { ERROR_108 + 100, "Отсутствует символ после \'." },
    { ERROR_109 + 100, "Отсутствуют или недостаточно символов после \"." },
    { ERROR_110 + 100, "Ошибка длины перехода по оператору ветвления." },
    { ERROR_111 + 100, "Недопустимое использование имени метки." },
    { ERROR_112 + 100, "Ошибка аргумента MARK." },
    { ERROR_113 + 100, "Ошибка в имени регистра." },
    { ERROR_114 + 100, "Ошибка в псевдокоманде." },
    { ERROR_115 + 100, "Ошибка в команде." },
    { ERROR_116 + 100, "Метка уже определена ранее." },
    { ERROR_117 + 100, "Аргумент .BLKB слишком велик." },
    { ERROR_118 + 100, "Аргумент .BLKW слишком велик." },
    { ERROR_119 + 100, "Аргумент .ORG слишком велик." },
    { ERROR_120 + 100, "Нечётный адрес команды." },
    { ERROR_121 + 100, "Отсутствует .END" },
    { ERROR_122 + 100, "Неопределённая метка в непосредственном выражении." },
    { ERROR_123 + 100, "Любые метки в выражении запрещены." },
    { ERROR_124 + 100, "Отсутствует переход у команды SOB." },
    { ERROR_125 + 100, "Ошибка аргумента TRAP." },
    { ERROR_126 + 100, "Ошибка аргумента EMT." },
    { ERROR_127 + 100, "Ошибка или неверный метод адресации." },
    { ERROR_128 + 100, "Отсутствует второй операнд." },
    { ERROR_129 + 100, "Переполнение байтового аргумента." },
    { ERROR_130 + 100, "Неожиданный конец строкового аргумента." },
    { ERROR_131 + 100, "Ошибка в числе с плавающей точкой." },
    { ERROR_132 + 100, "Невозможно открыть файл include." },
    { ERROR_133 + 100, "Ошибка в аргументах псевдокоманды." },
    { ERROR_134 + 100, "Переполнение словного аргумента." },
    { ERROR_135 + 100, "Ошибка в числовом аргументе." },
};

int IsError()
{
	return g_GlobalParameters.nError;
}

/*
вывод сообщения об ошибке.
n - номер ошибки
boutstr true - вывести строку, в которой случилась ошибка,
*/
char errbuf[65536] = { 0 };
char errbuf2[65536] = { 0 };
void OutError(ERROR_NUMBER n, bool bOutStr, int nAddr)
{
    FILE *errf = fopen("_errors.txt", "a");
	/*
	Тут в общем такая ситуация:
	nAddr введён только для того, чтобы правильно отслеживать ошибку длины перехода
	по ветвлению вперёд.
	Во всех случаях nAddr == -1 == значение по умолчанию.
	и только в одном единственном случае возникновения ошибки перехода по ветвлению вперёд
	nAddr - это адрес команды, которая вызвала ошибку 110

	!!! Если возникнет ещё какая нибудь такая ситуация, придётся всё переделывать.

	*/
	auto pll = g_Listing.end() - 1;

	if (nAddr == -1)
	{
		nAddr = g_GlobalParameters.nPC;
	}
	else if (n == ERROR_110)
	{
		// нужно теперь найти, в какое место вставить текст ошибки в листинге
		bool bEx = false;

		for (auto ll = g_Listing.begin(); ll != g_Listing.end(); ++ll)
		{
			for (auto &lct : ll->vCMD)
			{
				if (lct.nPC == nAddr && lct.LT == ListType::LT_INSTRUCTION)
				{
					pll = ll;
					bEx = true;
					break;
				}
			}

			if (bEx)
			{
				break;
			}
		}
	}

	// и приятный бонус. теперь и тут всё правильно отображается для ошибки 110 по переходу вперёд.
    sprintf(errbuf2, "Error %3d: %s\n", errors[n].num, errors[n].str.c_str());
    sprintf(errbuf, "Line %d (Addr: %07o)", pll->nLineNum, CorrectAddress(nAddr));
    pll->errors.emplace_back(utf8_to_wstring(std::string(errbuf2))); // добавляем в листинг ошибку
    Lst_AddPrepareLine(0, ListType::LT_COMMENTARY);
    printf("%s - %s", errbuf, errbuf2);

	if (errf)
	{
        fprintf(errf, "%s :: %s - %s", g_pReader->GetFileName().c_str(), errbuf, errbuf2);
	}

#if (DEBUG_LABEL_MANAGER)
	// /-----------------------debug---------------------------
    fprintf(dbgF, L"%s", errbuf);
	// /-----------------------debug---------------------------
#endif

	// если нужно - ещё и строку покажем, где ошибка
	if (bOutStr)
	{
        printf("%s\n", wstring_to_utf8(pll->line).c_str());

		if (errf)
		{
            fprintf(errf, "%s\n", wstring_to_utf8(pll->line).c_str());
		}

#if (DEBUG_LABEL_MANAGER)
		// /-----------------------debug---------------------------
        fprintf(dbgF, L"%s\n", pll->line.c_str());
		// /-----------------------debug---------------------------
#endif
	}

	if (errf)
	{
		fclose(errf);
	}

	g_GlobalParameters.nError++;
}

