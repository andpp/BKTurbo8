#include "pch.h"

#include "Globals.h"
#include "Assemble.h"
#include "BKToken.h"
#include "LabelManager.h"
#include "Parser.h"
#include "Listing.h"
#include "ErrorManager.h"

CPUCommandStruct g_pCPUCommands[] =
{
	{ L"HALT",   0000000,    CPUCmdGroup::NOOPS }, // op
	{ L"WAIT",   0000001,    CPUCmdGroup::NOOPS },
	{ L"RTI",    0000002,    CPUCmdGroup::NOOPS },
	{ L"BPT",    0000003,    CPUCmdGroup::NOOPS },
	{ L"IOT",    0000004,    CPUCmdGroup::NOOPS },
	{ L"RESET",  0000005,    CPUCmdGroup::NOOPS },
	{ L"RTT",    0000006,    CPUCmdGroup::NOOPS },
	{ L"START",  0000012,    CPUCmdGroup::NOOPS },
	{ L"STEP",   0000016,    CPUCmdGroup::NOOPS },
	{ L"NOP",    0000240,    CPUCmdGroup::NOOPS },
	{ L"CLC",    0000241,    CPUCmdGroup::NOOPS },
	{ L"CLV",    0000242,    CPUCmdGroup::NOOPS },
	{ L"CLVC",   0000243,    CPUCmdGroup::NOOPS },
	{ L"CLZ",    0000244,    CPUCmdGroup::NOOPS },
	{ L"CLZC",   0000245,    CPUCmdGroup::NOOPS },
	{ L"CLZV",   0000246,    CPUCmdGroup::NOOPS },
	{ L"CLZVC",  0000247,    CPUCmdGroup::NOOPS },
	{ L"CLN",    0000250,    CPUCmdGroup::NOOPS },
	{ L"CLNC",   0000251,    CPUCmdGroup::NOOPS },
	{ L"CLNV",   0000252,    CPUCmdGroup::NOOPS },
	{ L"CLNVC",  0000253,    CPUCmdGroup::NOOPS },
	{ L"CLNZ",   0000254,    CPUCmdGroup::NOOPS },
	{ L"CLNZC",  0000255,    CPUCmdGroup::NOOPS },
	{ L"CLNZV",  0000256,    CPUCmdGroup::NOOPS },
	{ L"CCC",    0000257,    CPUCmdGroup::NOOPS },
	{ L"SEC",    0000261,    CPUCmdGroup::NOOPS },
	{ L"SEV",    0000262,    CPUCmdGroup::NOOPS },
	{ L"SEVC",   0000263,    CPUCmdGroup::NOOPS },
	{ L"SEZ",    0000264,    CPUCmdGroup::NOOPS },
	{ L"SEZC",   0000265,    CPUCmdGroup::NOOPS },
	{ L"SEZV",   0000266,    CPUCmdGroup::NOOPS },
	{ L"SEZVC",  0000267,    CPUCmdGroup::NOOPS },
	{ L"SEN",    0000270,    CPUCmdGroup::NOOPS },
	{ L"SENC",   0000271,    CPUCmdGroup::NOOPS },
	{ L"SENV",   0000272,    CPUCmdGroup::NOOPS },
	{ L"SENVC",  0000273,    CPUCmdGroup::NOOPS },
	{ L"SENZ",   0000274,    CPUCmdGroup::NOOPS },
	{ L"SENZC",  0000275,    CPUCmdGroup::NOOPS },
	{ L"SENZV",  0000276,    CPUCmdGroup::NOOPS },
	{ L"SCC",    0000277,    CPUCmdGroup::NOOPS },
	{ L"RET",    0000207,    CPUCmdGroup::NOOPS },
	{ L"RETURN", 0000207,    CPUCmdGroup::NOOPS },

	{ L"BR",     0000400,    CPUCmdGroup::CBRANCH }, // op lll
	{ L"BNE",    0001000,    CPUCmdGroup::CBRANCH },
	{ L"BEQ",    0001400,    CPUCmdGroup::CBRANCH },
	{ L"BGE",    0002000,    CPUCmdGroup::CBRANCH },
	{ L"BLT",    0002400,    CPUCmdGroup::CBRANCH },
	{ L"BGT",    0003000,    CPUCmdGroup::CBRANCH },
	{ L"BLE",    0003400,    CPUCmdGroup::CBRANCH },
	{ L"BPL",    0100000,    CPUCmdGroup::CBRANCH },
	{ L"BMI",    0100400,    CPUCmdGroup::CBRANCH },
	{ L"BHI",    0101000,    CPUCmdGroup::CBRANCH },
	{ L"BLOS",   0101400,    CPUCmdGroup::CBRANCH },
	{ L"BVC",    0102000,    CPUCmdGroup::CBRANCH },
	{ L"BVS",    0102400,    CPUCmdGroup::CBRANCH },
	{ L"BCC",    0103000,    CPUCmdGroup::CBRANCH },
	{ L"BCS",    0103400,    CPUCmdGroup::CBRANCH },
	{ L"BHIS",   0103000,    CPUCmdGroup::CBRANCH },
	{ L"BLO",    0103400,    CPUCmdGroup::CBRANCH },

	{ L"MUL",    0070000,    CPUCmdGroup::EIS }, // op ss,r
	{ L"DIV",    0071000,    CPUCmdGroup::EIS },
	{ L"ASH",    0072000,    CPUCmdGroup::EIS },
	{ L"ASHC",   0073000,    CPUCmdGroup::EIS },

	{ L"EMT",    0104000,    CPUCmdGroup::TRAP }, // op nnn
	{ L"TRAP",   0104400,    CPUCmdGroup::TRAP },

	{ L"SOB",    0077000,    CPUCmdGroup::SOB }, // op r,ll

	{ L"MARK",   0006400,    CPUCmdGroup::MARK }, // op nn

	{ L"JSR",    0004000,    CPUCmdGroup::TWOOPREG }, // op r,dd
	{ L"XOR",    0074000,    CPUCmdGroup::TWOOPREG },

	{ L"FADD",   0075000,    CPUCmdGroup::FIS }, // op r
	{ L"FSUB",   0075010,    CPUCmdGroup::FIS },
	{ L"FMUL",   0075020,    CPUCmdGroup::FIS },
	{ L"FDIV",   0075030,    CPUCmdGroup::FIS },
	{ L"RTS",    0000200,    CPUCmdGroup::FIS },

	{ L"PUSH",   0010046,    CPUCmdGroup::PUSH }, // op ss
	{ L"PUSHB",  0110046,    CPUCmdGroup::PUSH },

	{ L"JMP",    0000100,    CPUCmdGroup::ONEOPS }, // op dd
	{ L"SWAB",   0000300,    CPUCmdGroup::ONEOPS },
	{ L"MTPS",   0106400,    CPUCmdGroup::ONEOPS },
	{ L"MFPS",   0106700,    CPUCmdGroup::ONEOPS },
	{ L"MTPD",   0106600,    CPUCmdGroup::ONEOPS },
	{ L"MFPD",   0106500,    CPUCmdGroup::ONEOPS },
	{ L"MTPI",   0006600,    CPUCmdGroup::ONEOPS },
	{ L"MFPI",   0006500,    CPUCmdGroup::ONEOPS },
	{ L"SXT",    0006700,    CPUCmdGroup::ONEOPS },
	{ L"CALL",   0004700,    CPUCmdGroup::ONEOPS },
	{ L"POP",    0012600,    CPUCmdGroup::ONEOPS },
	{ L"POPB",   0112600,    CPUCmdGroup::ONEOPS },
	{ L"CLR",    0005000,    CPUCmdGroup::ONEOPS },
	{ L"COM",    0005100,    CPUCmdGroup::ONEOPS },
	{ L"INC",    0005200,    CPUCmdGroup::ONEOPS },
	{ L"DEC",    0005300,    CPUCmdGroup::ONEOPS },
	{ L"NEG",    0005400,    CPUCmdGroup::ONEOPS },
	{ L"ADC",    0005500,    CPUCmdGroup::ONEOPS },
	{ L"SBC",    0005600,    CPUCmdGroup::ONEOPS },
	{ L"TST",    0005700,    CPUCmdGroup::ONEOPS },
	{ L"ROR",    0006000,    CPUCmdGroup::ONEOPS },
	{ L"ROL",    0006100,    CPUCmdGroup::ONEOPS },
	{ L"ASR",    0006200,    CPUCmdGroup::ONEOPS },
	{ L"ASL",    0006300,    CPUCmdGroup::ONEOPS },
	{ L"CLRB",   0105000,    CPUCmdGroup::ONEOPS },
	{ L"COMB",   0105100,    CPUCmdGroup::ONEOPS },
	{ L"INCB",   0105200,    CPUCmdGroup::ONEOPS },
	{ L"DECB",   0105300,    CPUCmdGroup::ONEOPS },
	{ L"NEGB",   0105400,    CPUCmdGroup::ONEOPS },
	{ L"ADCB",   0105500,    CPUCmdGroup::ONEOPS },
	{ L"SBCB",   0105600,    CPUCmdGroup::ONEOPS },
	{ L"TSTB",   0105700,    CPUCmdGroup::ONEOPS },
	{ L"RORB",   0106000,    CPUCmdGroup::ONEOPS },
	{ L"ROLB",   0106100,    CPUCmdGroup::ONEOPS },
	{ L"ASRB",   0106200,    CPUCmdGroup::ONEOPS },
	{ L"ASLB",   0106300,    CPUCmdGroup::ONEOPS },

	{ L"MOV",    0010000,    CPUCmdGroup::TWOOPS }, // op ss, dd
	{ L"CMP",    0020000,    CPUCmdGroup::TWOOPS },
	{ L"BIT",    0030000,    CPUCmdGroup::TWOOPS },
	{ L"BIC",    0040000,    CPUCmdGroup::TWOOPS },
	{ L"BIS",    0050000,    CPUCmdGroup::TWOOPS },
	{ L"MOVB",   0110000,    CPUCmdGroup::TWOOPS },
	{ L"CMPB",   0120000,    CPUCmdGroup::TWOOPS },
	{ L"BITB",   0130000,    CPUCmdGroup::TWOOPS },
	{ L"BICB",   0140000,    CPUCmdGroup::TWOOPS },
	{ L"BISB",   0150000,    CPUCmdGroup::TWOOPS },
	{ L"ADD",    0060000,    CPUCmdGroup::TWOOPS },
	{ L"SUB",    0160000,    CPUCmdGroup::TWOOPS },

	{ L"",       0,          CPUCmdGroup::NOOPS } // конец массива.
};

PseudoCommandStruct PseudoCommands[] =
{
	{ L"ADDR",   true,   PSC_addr },
	{ L"LA",     true,   PSC_la },
	{ L"PRINT",  true,   PSC_print },
	{ L"BLKW",   true,   PSC_blkw },
	{ L"WORD",   true,   PSC_word },
	{ L"RAD50",  true,   PSC_rad50 },
	{ L"END",    false,  PSC_end },
	{ L"EVEN",   false,  PSC_even },
	{ L"BLKB",   false,  PSC_blkb },
	{ L"BYTE",   false,  PSC_byte },
	{ L"ASCII",  false,  PSC_ascii },
	{ L"ASCIZ",  false,  PSC_asciz },
	{ L"ORG",    false,  PSC_org },
	{ L"FLT2",   true,   PSC_flt2 },
	{ L"FLT4",   true,   PSC_flt4 },
	{ L"INCLUDE", false,  PSC_Include },
	{ L"",       false,  nullptr }
};

// вход: token - текущая прочитанная инструкция, или по крайней мере что-то похожее.
//		cp - текущий временный PC, на выходе должен указывать на слово за командой.
// выход: true - нормально
//      false - ошибка
bool AssembleCPUInstruction(CBKToken *token, int &cp, wchar_t &ch)
{
	if (cp & 1)
	{
		OutError(ERROR_120);    // Нечётный адрес команды.
		g_Memory.b[cp++] = 0;   // выровняем и будем компилировать дальше.
	}

	g_GlobalParameters.bOperandType = false; // считаем, что по умолчанию - приёмник
	g_GlobalParameters.nAriphmType = 1;     // арифметические операции внутри ассемблерной инструкции
	int i = 0;

	while (!g_pCPUCommands[i].strName.empty()) // пока не конец таблицы
	{
		if (token->calcHash(g_pCPUCommands[i].strName) == token->getHash()) // если нашли мнемонику
		{
#ifdef _DEBUG

			// проверка на ложное срабатывание.
			if (g_pCPUCommands[i].strName != token->getName())
			{
                printf(L"HASH Error: %s:%#zX ~~ %s:%#zX\n", g_pCPUCommands[i].strName.c_str(), token->calcHash(g_pCPUCommands[i].strName), token->getName().c_str(), token->getHash());
				assert(false);
			}

#endif
			// если нашли нужную мнемонику
			// нужно взять опкод.
			g_Memory.w[cp / 2] = g_pCPUCommands[i].nOpcode;
			g_Memory.w[cp / 2 + 1] = 0; // следующие два слова заранее обнулим
			g_Memory.w[cp / 2 + 2] = 0;
			bool bRet = false;
			Lst_AddPrepareLine(cp, ListType::LT_INSTRUCTION);

			// и обработать операнды в соответствии с типом группы, к которой принадлежит опкод.
			switch (g_pCPUCommands[i].nGroup)
			{
				case CPUCmdGroup::NOOPS: // ничего делать не надо.
					bRet = true;
					break;

				case CPUCmdGroup::CBRANCH:
					bRet = assembleBR(cp, ch);
					break;

				case CPUCmdGroup::EIS:
					bRet = assemble2ROP(cp, ch);
					break;

				case CPUCmdGroup::TRAP:
					bRet = assembleTRAP(cp, ch);
					break;

				case CPUCmdGroup::SOB:
					bRet = assembleSOB(cp, ch);
					break;

				case CPUCmdGroup::MARK:
					bRet = assembleMARK(cp, ch);
					break;

				case CPUCmdGroup::TWOOPREG:
					bRet = assemble2OPR(cp, ch);
					break;

				case CPUCmdGroup::FIS:
					bRet = assemble1OPR(cp, ch);
					break;

				case CPUCmdGroup::PUSH:
					g_GlobalParameters.bOperandType = true; // меняем тип операнда

				// и переходим к выполнению CPUCmdGroup::ONEOPS
				case CPUCmdGroup::ONEOPS:
					bRet = assemble1OP(cp, ch);
					break;

				case CPUCmdGroup::TWOOPS:
					bRet = assemble2OP(cp, ch);
					break;
			}

			cp += 2;
			return bRet;
		}

		i++;
	}

	// ошибка - неопознанная команда.
	OutError(ERROR_107); // Неправильная команда.
	return false;
}

// сборка двухоперандных команд
bool assemble2OP(int &cp, wchar_t &ch)
{
	g_GlobalParameters.bOperandType = true; // сперва ss

	if (!assemble1OP(cp, ch))   // обработаем один операнд
	{
		return false;
	}

	if (!needChar(L',', ch))    // проверим наличие второго
	{
		OutError(ERROR_128);    // Ошибка в команде - нет второго операнда.
		return false;
	}

	g_GlobalParameters.bOperandType = false; // теперь dd
	ch = g_pReader->readChar(true); // пропускаем запятую
	return assemble1OP(cp, ch);     // обработаем второй операнд
}

// сборка обычных однооперандных команд
bool assemble1OP(int &cp, wchar_t &ch)
{
	cp += 2; // двигаем указатель на местоположение операнда
	return Operand_analyse(cp, ch); // разбираемся, что там такое
}

// сборка однооперандных команд, где операнд - регистр
bool assemble1OPR(int &cp, wchar_t &ch)
{
    (void)cp;
	g_pReader->SkipWhitespaces(ch);

	if (ReadRegName(ch))
	{
		return true;
	}

	OutError(ERROR_113); // Ошибка в имени регистра.
	return false;
}

// сборка двухоперандных команд, где первый операнд - регистр
bool assemble2OPR(int &cp, wchar_t &ch)
{
	g_GlobalParameters.bOperandType = true; // сперва ss

	if (!assemble1OPR(cp, ch))  // обрабатываем регистр
	{
		return false;
	}

	if (!needChar(L',', ch))    // проверим наличие второго
	{
		OutError(ERROR_128);    // Ошибка в команде - нет второго операнда.
		return false;
	}

	g_GlobalParameters.bOperandType = false; // теперь dd
	ch = g_pReader->readChar(true); // пропускаем запятую
	return assemble1OP(cp, ch);     // обработаем второй операнд
}

// сборка двухоперандных команд, где второй операнд - регистр
// Это EIS команды, у которых  при записи мнемоники почему-то поменяны местами
// источник и приёмник, как у x86
bool assemble2ROP(int &cp, wchar_t &ch)
{
	g_GlobalParameters.bOperandType = false; // сперва dd

	if (!assemble1OP(cp, ch))   // обработаем первый операнд
	{
		return false;
	}

	if (!needChar(L',', ch))    // проверим наличие второго
	{
		OutError(ERROR_128);    // Ошибка в команде - нет второго операнда.
		return false;
	}

	g_GlobalParameters.bOperandType = true; // теперь ss
	ch = g_pReader->readChar(true); // пропускаем запятую
	return assemble1OPR(cp, ch);    // обработаем второй операнд
}

// сборка команд ветвления
bool assembleBR(int &cp, wchar_t &ch)
{
	int result = 0;
	CBKToken token;
	bool bRet = false;
#if (DEBUG_LABEL_MANAGER)
	// /-----------------------debug---------------------------
    fprintf(dbgF, L"Assemble branch %06o\n", cp);
	// /-----------------------debug---------------------------
#endif

	if (trp202(&token, result, ch)) // читаем имя метки, и если оно нашлось в таблице
	{
#if (DEBUG_LABEL_MANAGER)
		// /-----------------------debug---------------------------
        fprintf(dbgF, L"\tbranch label found '%s'.\n", token.getName().c_str());
		// /-----------------------debug---------------------------
#endif
		int temp = result;  //то продолжим разбирать арифметическое выражение, если оно есть
		bRet = trp242(cp, result, ch);
		result += temp;
	}
	else
	{
#if (DEBUG_LABEL_MANAGER)
		// /-----------------------debug---------------------------
        fprintf(dbgF, L"\tbranch label not found '%s'.\n", token.getName().c_str());
		// /-----------------------debug---------------------------
#endif
		// если метка не нашлась, то добавим её в таблицу ипродолжим разбор арифметического выражения
		bRet = trp240(&token, cp, result, ch, BRANCH_LABEL);
		result += cp + 2;
	}

	// судя по всему, если арифметическое выражение будет начинаться с числа, то будет облом.
#if (DEBUG_LABEL_MANAGER)
	// /-----------------------debug---------------------------
    fprintf(dbgF, L"\tCommand Address %06o, Target Address: %06o\n", cp, result);
	// /-----------------------debug---------------------------
#endif

	if (!bRet) // если при разборе получились ошибки
	{
		return false;   // выходим
	}

	if (BranchVerify(cp, result))
	{
#if (DEBUG_LABEL_MANAGER)
		// /-----------------------debug---------------------------
        fprintf(dbgF, L"\tbranch offset %06o\n", result);
		// /-----------------------debug---------------------------
#endif
		g_Memory.b[cp] = (result & 0xff); // задаём финальное смещение
		return true;
	}
	else
	{
#if (DEBUG_LABEL_MANAGER)
		// /-----------------------debug---------------------------
        fprintf(dbgF, L"\tbranch offset %06o\n", result);
		// /-----------------------debug---------------------------
#endif
		OutError(ERROR_110); // Ошибка длины перехода по оператору ветвления.
		return false;
	}
}

bool assembleSOB(int &cp, wchar_t &ch)
{
	g_GlobalParameters.bOperandType = true;     // сперва ss

	if (!assemble1OPR(cp, ch))  // обработаем имя регистра
	{
		return false;
	}

	if (!needChar(L',', ch))    // проверим наличие второго
	{
		OutError(ERROR_124);    // Ошибка в команде.
		return false;
	}

	g_GlobalParameters.bOperandType = false;    // теперь dd
	ch = g_pReader->readChar(true); // пропускаем запятую
	g_pReader->SkipWhitespaces(ch); // обработаем второй операнд
	int tres = 0;
	CBKToken token;

	if (trp202(&token, tres, ch))   // читаем имя метки, и если оно нашлось в таблице
	{
		int result = tres;      //то продолжим разбирать арифметическое выражение, если оно есть
		trp242(cp, tres, ch);
		result += tres - cp - 2;    // смещение

		if (result < 0)     // если смещение < 0,  то всё верно
		{
			result = -result;   // вычисляем код смещения
			result /= 2;

			if (result <= 077)  // если он влазит
			{
				g_Memory.w[cp / 2] |= uint16_t(result); // формируем опкод
				return true;
			}
		}
	}

	OutError(ERROR_102); // Ошибка длины или направления перехода в команде SOB.
	return false;
}

bool assembleTRAP(int &cp, wchar_t &ch)
{
	int nOp = g_Memory.w[cp / 2]; // код опкода нужен, чтобы текстошибки правильный вывести
	int result = 0;
	g_GlobalParameters.nAriphmType = -1;    // неопределённые метки не допускаются

	if (AriphmParser(cp, result, ch))
	{
		if (result < 0377)
		{
			g_Memory.w[cp / 2] |= uint16_t(result);
			return true;
		}
	}

	OutError((nOp == 0104400) ? ERROR_125 : ERROR_126); // Ошибка аргумента TRAP, EMT.
	return false;
}

bool assembleMARK(int &cp, wchar_t &ch)
{
	int result = 0;
	g_GlobalParameters.nAriphmType = -1;    // неопределённые метки не допускаются

	if (AriphmParser(cp, result, ch))
	{
		if (result < 077)
		{
			g_Memory.w[cp / 2] |= uint16_t(result);
			return true;
		}
	}

	OutError(ERROR_112); // Ошибка аргумента MARK
	return false;
}


// вход: token - текущая прочитанная псевдокоманда, или по крайней мере что-то похожее.
//		cp - текущий временный PC, на выходе должен указывать на слово за командой.
// выход: true - нормально
//      false - ошибка
bool PseudoCommandExec(CBKToken *token, int &cp, wchar_t &ch)
{
	g_GlobalParameters.bOperandType = false;
	int i = 0;

	while (!PseudoCommands[i].strName.empty())
	{
		if (token->calcHash(PseudoCommands[i].strName) == token->getHash()) // если нашли подходящую псевдокоманду
		{
#ifdef _DEBUG

			if (PseudoCommands[i].strName != token->getName())
			{
                printf(L"HASH Error: %s:%#zX ~~ %s:%#zX\n", PseudoCommands[i].strName.c_str(), token->calcHash(PseudoCommands[i].strName), token->getName().c_str(), token->getHash());
				assert(false);
			}

#endif

			// если команда допускает только чётные адреса
			if (PseudoCommands[i].bEvenAddr)
			{
				// нужна доп проверка на чётность адреса
				if (cp & 1)
				{
					OutError(ERROR_120); // Нечётный адрес команды.
					g_Memory.b[cp++] = 0;
					return false;
				}
			}

			g_pReader->SkipWhitespaces(ch);
			g_Memory.w[cp / 2 + 1] = 0;
			g_Memory.w[cp / 2 + 2] = 0;
			return PseudoCommands[i].PSCFunction(cp, ch);
		}

		i++;
	}

	OutError(ERROR_106); // Неправильная псевдокоманда.
	return false;
}


bool PSC_addr(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_ADDR);
	g_Memory.w[cp / 2] = 0;

	if (!ReadRegName(ch))
	{
		OutError(ERROR_113); // Ошибка в имени регистра.
		return false;
	}

	uint16_t t = g_Memory.w[cp / 2];  // t - номер регистра
	g_Memory.w[cp / 2] |= 010700; // формируем 1070R
	cp += 2;
	g_Memory.w[cp / 2] = 062700 | t; // формируем 6270R
	cp += 2;

	if (needChar(L',', ch))
	{
		ch = g_pReader->readChar(true);
		g_pReader->SkipWhitespaces(ch);
		CBKToken token;

		if (g_pReader->readToken(&token, ch))
		{
			int result;
			bool bret = trp240(&token, cp, result, ch, OFFSET_LABEL);
			result += 4;
			g_Memory.w[cp / 2] = result; cp += 2; // формируем смещение
			return bret;
		}
	}

	cp += 2;
	OutError(ERROR_128); // Ошибка в псевдокоманде.
	return false;
}

bool PSC_la(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_LA);

	if (cp == BASE_ADDRESS)
	{
		int result = 0;

		if (AriphmParser(cp, result, ch))
		{
			SetStartAddress(result);
			return true;
		}
		else
		{
			return false;
		}
	}

	OutError(ERROR_101); // Псевдокоманда .LA должна быть первой в тексте.
	return false;
}

bool PSC_print(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_PRINT);

	if (ch == L'#')
	{
		ch = g_pReader->readChar();

		if (assemble1OP(cp, ch))
		{
			g_Memory.w[cp / 2 - 1] = 012701; cp += 2;
			g_Memory.w[cp / 2] = 05002; cp += 2;
			g_Memory.w[cp / 2] = 0104020; cp += 2;
			return true;
		}
	}

	// выполним условие - что на выходе cp должен указывать на адрес за командой
	cp += 8; g_Memory.w[cp / 2 - 1] = 0;
	OutError(ERROR_114); // Ошибка в псевдокоманде.
	return false;
}

bool PSC_blkw(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_BLKW);
	int result;
	g_GlobalParameters.nAriphmType = -1;

	if (AriphmParser(cp, result, ch))
	{
		result *= 2;

		if (cp + result >= HIGH_BOUND)
		{
			OutError(ERROR_118); // Ошибка в псевдокоманде.
			return false;
		}

		for (int i = 0; i < result; ++i)
		{
			g_Memory.b[cp++] = 0;
		}

		return true;
	}

	// здесь, при ошибке, условие - что на выходе cp должен указывать на адрес за командой, невыполнимо
	OutError(ERROR_114); // Ошибка в псевдокоманде.
	return false;
}

bool PSC_blkb(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_BLKB);
	int result;
	g_GlobalParameters.nAriphmType = -1;

	if (AriphmParser(cp, result, ch))
	{
		if (cp + result >= HIGH_BOUND)
		{
			OutError(ERROR_117); // Ошибка в псевдокоманде.
			return false;
		}

		for (int i = 0; i < result; ++i)
		{
			g_Memory.b[cp++] = 0;
		}

		return true;
	}

	// здесь, при ошибке, условие - что на выходе cp должен указывать на адрес за командой, невыполнимо
	OutError(ERROR_114); // Ошибка в псевдокоманде.
	return false;
}

// .ORG n - выравнивание по адресу.
bool PSC_org(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_LA);
	int result = 0;
	g_GlobalParameters.nAriphmType = -1;

	if (AriphmParser(cp, result, ch)) // получим аргумент
	{
		result = CorrectOffset(result);

		if (result >= HIGH_BOUND)
		{
			OutError(ERROR_119);    // Ошибка в псевдокоманде.
			return false;
		}

		while (cp < result)         // выравниваем адрес до аргумента
		{
			g_Memory.b[cp++] = 0;
		}

		return true;
	}

	// здесь, при ошибке, условие - что на выходе cp должен указывать на адрес за командой, невыполнимо
	OutError(ERROR_114); // Ошибка в псевдокоманде.
	return false;
}

bool PSC_flt2(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_FLT2);
	uint16_t flt[2];

	for (;;)
	{
		if (parse_float(ch, 2, flt))
		{
			g_Memory.w[cp / 2] = flt[0]; cp += 2;
			g_Memory.w[cp / 2] = flt[1]; cp += 2;
		}
		else
		{
			OutError(ERROR_131);// ошибка парсинга flt2
			return false;
		}

		if (!needChar(L',', ch))
		{
			break;
		}

		ch = g_pReader->readChar(true);
	}

	return true;
}

bool PSC_flt4(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_FLT4);
	uint16_t flt[4];

	for (;;)
	{
		if (parse_float(ch, 4, flt))
		{
			g_Memory.w[cp / 2] = flt[0]; cp += 2;
			g_Memory.w[cp / 2] = flt[1]; cp += 2;
			g_Memory.w[cp / 2] = flt[2]; cp += 2;
			g_Memory.w[cp / 2] = flt[3]; cp += 2;
		}
		else
		{
			OutError(ERROR_131);// ошибка парсинга flt4
			return false;
		}

		if (!needChar(L',', ch))
		{
			break;
		}

		ch = g_pReader->readChar(true);
	}

	return true;
}

bool PSC_word(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_WORD);
	g_GlobalParameters.nAriphmType = 1;
	int result;

	for (;;)
	{
		if (ch == L'@')
		{
			CBKToken label;
			ch = g_pReader->readChar(); // пропустим "@"

			if (g_pReader->readToken(&label, ch))
			{
				if (!trp240(&label, cp, result, ch, OFFSET_LABEL))
				{
					return false;
				}

				result += 2;
			}
			else
			{
				OutError(ERROR_114); // Ошибка в псевдокоманде.
				return false;
			}
		}
		else if (!AriphmParser(cp, result, ch))
		{
			g_Memory.w[cp / 2] = 0; cp += 2;
			return false;
		}

		g_Memory.w[cp / 2] = result; cp += 2;

		if (!needChar(L',', ch))
		{
			break;
		}

		ch = g_pReader->readChar(true);
	}

	return true;
}

bool PSC_byte(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_BYTE);
	g_GlobalParameters.nAriphmType = -1;
	int result;

	for (;;)
	{
		if (!AriphmParser(cp, result, ch))
		{
			g_Memory.b[cp++] = 0;
			return false;
		}

		if (result > 0377)
		{
			OutError(ERROR_129); // Ошибка или отсутствие числового аргумента.
			break;
		}

		g_Memory.b[cp++] = uint8_t(result & 0xff);

		if (!needChar(L',', ch))
		{
			break;
		}

		ch = g_pReader->readChar(true);
	}

	return true;
}

bool PSC_end(int &cp, wchar_t &ch)
{
    (void)ch;
	Lst_AddPrepareLine(cp, ListType::LT_PSC_END);
	g_GlobalParameters.bENDReached = true;
	g_GlobalParameters.nProgramLength = cp - BASE_ADDRESS;

	if (g_GlobalParameters.nModeLinkage >= 0)
	{
		g_GlobalParameters.nModeLinkage = 0;
	}

	LabelLinking();
	g_labelLocalDefs.Clear();
	return true;
}

bool PSC_even(int &cp, wchar_t &ch)
{
    (void)ch;
	Lst_AddPrepareLine(cp, ListType::LT_PSC_EVEN);

	if (cp & 1)
	{
		g_Memory.b[cp++] = 0;
	}

	return true;
}

bool StrQuotes(wchar_t ch)
{
	// первым квотируемым символом считаем любой символ пунктуации и т.п., забыл как группа этих символов называется
	// вообще, по правилам MACRO, квотируемым считается вообще любой символ, кроме <, но это уж как-то слишком
	// усложняет понимание вида текстовой строки, поэтому ограничим себя
	if ((L'!' <= ch && ch <= L'/') || (ch == L':' || ch == L'=' || ch == L'?' || ch == L'@') || (0133 <= ch && ch <= 137) || (0173 <= ch && ch <= 177))
	{
		return true;
	}

	return false;
}

bool PSC_rad50(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_RAD50);

	for (;;)
	{
		uint16_t w = 0; // текущее слово в radix50
		int nCnt = 0; // счётчик символов в слове
		int nMultipler = 03100;

		if (StrQuotes(ch))
		{
			wchar_t chQuoter = ch;
			g_GlobalParameters.bInString = true;

			for (;;)
			{
				ch = g_pReader->readChar(); // берём очередной символ

				if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LN)
				{
					OutError(ERROR_130); // Ошибка в псевдокоманде.
					return false;
				}

				if (ch == chQuoter) // строка закончилась
				{
					if (nCnt) // если что-то было ещё принято
					{
						g_Memory.w[cp / 2] = w; // сохраним
						cp += 2;
					}

					ch = g_pReader->readChar(true); // пропускаем закрывающую кавычку
					break;
				}

				// теперь поищем символ в таблице
				uint16_t nChCode = 0;   // код текущего символа по умолчанию, если символ не будет найден в таблице, будет значение по умолчанию
				ch = toupper(ch);

				for (int n = 0; n < 050; ++n)
				{
					if (RADIX50[n] == ch)
					{
						nChCode = n;    // нашли
						break;
					}
				}

				w += nChCode * nMultipler; // упаковываем очередной символ
				nMultipler /= 050;

				if (++nCnt >= 3)    // если пора сохранять полное слово
				{
					nCnt = 0; nMultipler = 03100;
					g_Memory.w[cp / 2] = w; // сохраним
					cp += 2;
					w = 0;
				}
			}

			g_GlobalParameters.bInString = false;
		}
		else if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::SPACES)
		{
			if (g_pReader->SkipWhitespaces(ch))
			{
				break;
			}
		}
		else if (CheckComment(ch))  // если комментарий - то выходим, его тут не обработать
		{
			break;
		}
		else if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LN)
		{
			break;
		}
		else
		{
			OutError(ERROR_114); // Ошибка в псевдокоманде.
			return false;
		}
	}

	return true;
}

bool PSC_ascii(int &cp, wchar_t &ch)
{
	Lst_AddPrepareLine(cp, ListType::LT_PSC_ASCII);
	g_GlobalParameters.nAriphmType = -1;

	for (;;)
	{
		if (StrQuotes(ch)) // если квотируемый символ, считаем что это кавычка
		{
			// и принимаем строку
			wchar_t chQuoter = ch;
			g_GlobalParameters.bInString = true;

			for (;;)
			{
				ch = g_pReader->readChar();

				if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LN)
				{
					OutError(ERROR_130); // Ошибка в псевдокоманде - обрыв строки.
					return false;
				}

				if (ch == chQuoter)
				{
					ch = g_pReader->readChar(true); // пропускаем закрывающую кавычку
					break;
				}

				g_Memory.b[cp++] = UNICODEtoBK_ch(ch);
			}

			g_GlobalParameters.bInString = false;
		}
		else if (ch == L'<') // если левая скобка, то принимаем число
		{
			ch = g_pReader->readChar(true); // пропустим скобку
			int result = 0;
			bool bRes = AriphmParser(cp, result, ch); // посчитаем арифм. выражение

			if (g_pReader->SkipWhitespaces(ch)) // пропускаем пробелы
			{
				//если внезапный конец строки
				g_Memory.b[cp++] = 0;
				OutError(ERROR_133); // Ошибка в аргументах псевдокоманды.
				return false;
			}

			if (ch == L'>') // если нашлась закрывающая скобка
			{
				ch = g_pReader->readChar(true); // пропустим скобку

				if (bRes && (result < 0377)) // если аргумент верный
				{
					g_Memory.b[cp++] = uint8_t(result & 0xff); // то его сохраняем
				}
				else
				{
					g_Memory.b[cp++] = 0;
					OutError(ERROR_129); // Ошибка или отсутствие числового аргумента.
					return false;
				}
			}
			else
			{
				g_Memory.b[cp++] = 0;
				OutError(ERROR_133); // Ошибка в аргументах псевдокоманды.
				return false;
			}
		}
		else if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::SPACES) // если пробел
		{
			if (g_pReader->SkipWhitespaces(ch)) // то пропустим пробелы
			{
				break; // если конец строки - то выходим
			}
		}
		else if (CheckComment(ch))  // если комментарий - то выходим, его тут не обработать
		{
			break;
		}
		else if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LN)    // если конец строки - то выходим
		{
			break;
		}
		else
		{
			OutError(ERROR_114); // Ошибка в псевдокоманде.
			return false;
		}
	}

	return true;
}

bool PSC_asciz(int &cp, wchar_t &ch)
{
	bool bRet = PSC_ascii(cp, ch);
	g_Memory.b[cp++] = 0;
	return bRet;
}

// проверка комментария, выход:
// true - встретили комментарий,
// false - не комментарий, а что-то другое
bool CheckComment(wchar_t &ch)
{
	if (ch == L';' ||
	        (ch == L'/' && (g_pReader->getNextChar() == L'/' || g_pReader->getNextChar() == L'*')))
	{
		return true;
	}

	return false;
}


// проверка длины перехода по ветвлению
// вход: nTargetAddr - адрес метки
//      nCommandAddr - адрес команды. откуда переход
//  важное условие - в команде уже подставлено значение
// выход: nTargetAddr - корректное смещение
//      true - всё нормально
//      false - ошибка длины переход
bool BranchVerify(int nCommandAddr, int &nTargetAddr)
{
	nTargetAddr -= 2;
	auto nOffset = int(char(g_Memory.b[nCommandAddr])); // берём младший байт команды с расширением знака
	g_Memory.b[nCommandAddr] = 0; // смещение пока обнулим
	nTargetAddr += (nOffset * 2 - nCommandAddr);
	nTargetAddr /= 2;

	if (nTargetAddr > 0)
	{
		if (nTargetAddr < 0200)
		{
			return true;
		}
	}
	else
	{
		nTargetAddr = -nTargetAddr;

		if (nTargetAddr <= 0200)
		{
			nTargetAddr -= 0400;
			nTargetAddr = -nTargetAddr;
			return true;
		}
	}

	return false;
}

// выполнение .include filename
bool PSC_Include(int &cp, wchar_t &ch)
{
    (void)cp;
	Lst_AddPrepareLine(0, ListType::LT_COMMENTARY);
	g_GlobalParameters.bInString = true;
    std::wstring strFileName;
	wchar_t chQuoter = 0;

	if (StrQuotes(ch)) // если квотируемый символ, считаем что это кавычка
	{
		chQuoter = ch;
		ch = g_pReader->readChar();
	}

	// и принимаем строку
	for (;;)
	{
		if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LN)
		{
			break;
		}
		else if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::SPACES && chQuoter == 0)
		{
			break; // если имя не в кавычках и встретили пробел - то конец имени
		}
		else if (CheckComment(ch))  // если комментарий - то выходим, его тут не обработать
		{
			break;
		}

		if (ch == chQuoter)
		{
			ch = g_pReader->readChar();
			chQuoter = 0;
			break;
		}

		strFileName.push_back(ch);
		ch = g_pReader->readChar();
	}

	g_GlobalParameters.bInString = false;

	if (chQuoter)
	{
		// если есть открывающая ковычка, но нет закрывающей - ошибка
		OutError(ERROR_130);
		return false;
	}

	if (!strFileName.empty())
	{
        auto pReader = new CReader(wstring_to_utf8(strFileName), Charset);

		if (pReader)
		{
			if (pReader->GetFileCharset() == CReader::FILE_CHARSET::FILEERROR)
			{
				delete pReader;
				OutError(ERROR_132); // Ошибка в псевдокоманде.
				return false;
			}

			// поищем, вдруг уже добаваляли эту инклуду
			bool bFound = false;

			for (auto &rdr : v_Reader)
			{
                if (rdr && rdr->GetFileName() == wstring_to_utf8(strFileName))
				{
					bFound = true; // добавляли
					break;
				}
			}

			if (bFound) // если уже есть среди добавленных
			{
				delete pReader; // не будем ещё раз добавлять
			}
			else
			{
				// добавляем
				v_Reader.emplace_back(g_pReader);
				g_pReader = pReader;
				g_GlobalParameters.bStepInclude = true;
				return true;
			}
		}
	}
	else
	{
		// если нет имени файла - ошибка
		OutError(ERROR_114);
		return false;
	}

	return false;
}


/*
линковка меток
*/
void LabelLinking()
{
	std::vector<size_t> vInt;
	size_t sz = g_labelRefs.getSize();

	// компонуем метки
	for (size_t i = 0; i < sz; ++i) // для всех ссылок на метки
	{
		uint32_t lti = g_labelRefs.GetType(i);

		// если это переход (к обработке обязательно) или окончательная обработка (после команды .END)
		if (((lti & LABEL_REFERENCE_MASK) == BRANCH_LABEL) || (g_GlobalParameters.nModeLinkage == 0))
		{
			// то ищем метку в таблице меток
			CBKToken *token = g_labelRefs.GetLabel(i);
#if (DEBUG_LABEL_MANAGER)

			// /-----------------------debug---------------------------
			if ((lti & LABEL_REFERENCE_MASK) == BRANCH_LABEL)
			{
                fprintf(dbgF, L"Try link branch label '%s'\n", token->getName().c_str());
			}

			// /-----------------------debug---------------------------
#endif

			if ((lti & LABEL_REFERENCE_MASK) == CURRENTPC_LABEL) // если особая метка - ссылка на РС
			{
				int addr = g_labelRefs.GetValue(i);
				std::wstring strCPC = token->getName(); // имя метки используем как текущий PC.
				strCPC = strCPC.substr(1); // убираем первый символ
				int pc = std::stoi(strCPC, nullptr, 8);
				int res = CorrectAddress(pc);

				if (lti & NEGATIVE_FLAG)
				{
					res = -res;
				}

				if (lti & INVERSE_FLAG)
				{
					res = ~res;
				}

				if (lti & HALFLABEL_FLAG)
				{
					res = res / 2;
				}

				g_Memory.w[addr / 2] += res;
				// потом надо будет удалить найденную метку
				vInt.push_back(i); // Список найденных меток
			}
			else
			{
				uint32_t lt;
				int result = 0;

				if (SearchLabelInTables(token, result, lt)) // если нашли, то обрабатываем
				{
					int addr = g_labelRefs.GetValue(i);

					switch (lti & LABEL_REFERENCE_MASK)
					{
						case BRANCH_LABEL:
						{
#if (DEBUG_LABEL_MANAGER)
							// /-----------------------debug---------------------------
                            fprintf(dbgF, "\tCommand Address %06o, Target Address: %06o\n", addr, result);
							// /-----------------------debug---------------------------
#endif
							// проверка длины перехода по ветвлению
							bool bBV = BranchVerify(addr, result);
#if (DEBUG_LABEL_MANAGER)
							// /-----------------------debug---------------------------
                            fprintf(dbgF, "\tbranch offset %06o, verify: %s\n", result, (bBV ? "OK" : L"Fail"));
							// /-----------------------debug---------------------------
#endif

							if (bBV)
							{
								g_Memory.b[addr] = uint8_t(result & 0xff);
							}
							else
							{
								// !!! вот тут может быть неправильно выведена строка
								OutError(ERROR_110, true, addr); // Ошибка длины перехода по оператору ветвления.
							}
						}
						break;

						case OFFSET_LABEL:
							result = result - addr - 2;

							if ((lt & LABEL_DEFINITE_MASK) == CONSTANT_LABEL)
							{
								result = CorrectOffset(result);
							}

							// смещение не может быть ни уполовиненным ни отрицательным, оба этих действия бессмысленны
							g_Memory.w[addr / 2] += result;
							break;

						case RELATIVE_LABEL:
							if ((lt & LABEL_DEFINITE_MASK) != CONSTANT_LABEL)
							{
								result = CorrectAddress(result);
							}

							if (lti & NEGATIVE_FLAG)
							{
								result = -result;
							}

							if (lti & INVERSE_FLAG)
							{
								result = ~result;
							}

							if (lti & HALFLABEL_FLAG)
							{
								result = result / 2;
							}

							g_Memory.w[addr / 2] += result;
							break;
					}

					// потом надо будет удалить найденную метку
					vInt.push_back(i); // Список найденных меток
				}
			}
		}
	}

	// удаляем всё что обработали
#if (DEBUG_LABEL_MANAGER)
	// /-----------------------debug---------------------------
	size_t n = vInt.size();
	// /-----------------------debug---------------------------
#endif

	while (!vInt.empty())
	{
		size_t d = vInt.back();
		g_labelRefs.DeleteLabel(d);
		vInt.pop_back();
	}

#if (DEBUG_LABEL_MANAGER)

	// /-----------------------debug---------------------------
	if (n)
	{
        fprintf(dbgF, "Delete %d references, new refs size. (%d)\n", n, g_labelRefs.getSize());
	}

	// /-----------------------debug---------------------------
#endif
}
