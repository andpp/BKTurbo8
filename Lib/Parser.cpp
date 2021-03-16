#include "pch.h"
#include "Parser.h"
#include "Globals.h"
#include "LabelTable.h"
#include "LabelManager.h"
#include "ErrorManager.h"
#pragma warning(disable:4996)

Registers RegNames[] =
{
	{L"R0", 0},
	{L"R1", 1},
	{L"R2", 2},
	{L"R3", 3},
	{L"R4", 4},
	{L"R5", 5},
	{L"R6", 6},
	{L"R7", 7},
	{L"SP", 6},
	{L"PC", 7},
};


const wchar_t RADIX50[050] =
{
	// 000..007
	L' ', L'A', L'B', L'C', L'D', L'E', L'F', L'G',
	// 010..017
	L'H', L'I', L'J', L'K', L'L', L'M', L'N', L'O',
	// 020..027
	L'P', L'Q', L'R', L'S', L'T', L'U', L'V', L'W',
	// 030..037
	L'X', L'Y', L'Z', L'$', L'.', L' ', L'0', L'1',
	// 040..047
	L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9'
};


/*
парсер арифметического выражения
вход: cp - текущий РС
выход: result - результат арифметического выражения
true - всё нормально
false - необходимо прекратить компиляцию
*/
bool AriphmParser(int &cp, int &result, wchar_t &ch)
{
	int accum = 0;  // аккумулятор. Тут накапливается полный результат всего арифметического выражения
	result = 0;
	g_GlobalParameters.bInvert = false; // начинаем с нормального положения

	for (;;)
	{
		g_pReader->SkipWhitespaces(ch);
		uint32_t lt = RELATIVE_LABEL;

		// смотрим текущий символ
		if (ch == L'+') // если унарный +
		{
			ch = g_pReader->readChar(true); // то его игнорируем
		}
		else if (ch == L'-') // если унарный -
		{
			ch = g_pReader->readChar(true); // то его игнорируем
			lt |= NEGATIVE_FLAG; // выставим флаг минуса
		}

		// смотрим, какой там следующий символ
		if (ch == L'/') // если /
		{
			ch = g_pReader->readChar(); // то его пропустим
			lt |= HALFLABEL_FLAG;   // выставим флаг деления пополам

			if (g_pReader->getCurrCharType() != CReader::CHAR_TYPE::LETTERS)
			{
				OutError(ERROR_104); // нет переменной
				return false; // стоп компиляция.
			}
		}

		if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LETTERS)
		{
			CBKToken label;
			g_pReader->readToken(&label, ch);

			if (g_GlobalParameters.nAriphmType == 0)
			{
				// все метки запрещены
				OutError(ERROR_123);
				return false; // стоп компиляция.
			}
			else if (g_GlobalParameters.nAriphmType > 0)
			{
				// все метки разрешены. внесём в таблицу без проверки
				if (g_GlobalParameters.bInvert)
				{
					g_GlobalParameters.bInvert = false;
					lt |= INVERSE_FLAG;
				}

				AddlLabelReference(&label, cp, lt);
				goto next_op;
			}
			else
			{
				// в присваивании
				int n = g_labelGlobalDefs.SearchLabel(&label);

				if (n == -1)
				{
					// неопределённые метки запрещены.
					OutError(ERROR_122);
					return false; // стоп компиляция.
				}
				else
				{
					result = g_labelGlobalDefs.GetValue(n);

					// если это метка, то надо скорректировать адрес до реального
					if ((g_labelGlobalDefs.GetType(n) & LABEL_DEFINITE_MASK) == DEFINITE_LABEL)
					{
						result = CorrectAddress(result);
					}

					if (g_GlobalParameters.bInvert)
					{
						g_GlobalParameters.bInvert = false;
						result = ~result;
					}

					if (lt & HALFLABEL_FLAG)
					{
						result = result / 2;
					}
				}
			}
		}
		else if (ch == L'\'')
		{
			// получение одного символа
			g_GlobalParameters.bInString = true;
			ch = g_pReader->readChar();
			g_GlobalParameters.bInString = false;
			result = UNICODEtoBK_ch(ch);

			if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LN)
			{
				OutError(ERROR_108); // Ошибка или отсутствие числового аргумента.
				return false; // стоп компиляция.
			}

			ch = g_pReader->readChar(); // нужно прочитать ещё один раз, чтобы подвинуть курсор
		}
		else if (ch == L'"')
		{
			// получение двух символов
			g_GlobalParameters.bInString = true;
			ch = g_pReader->readChar();
			g_GlobalParameters.bInString = false;
			result = UNICODEtoBK_ch(ch);

			if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LN)
			{
				OutError(ERROR_109); // Ошибка или отсутствие числового аргумента.
				return false; // стоп компиляция.
			}

			g_GlobalParameters.bInString = true;
			ch = g_pReader->readChar();
			g_GlobalParameters.bInString = false;
			result |= (UNICODEtoBK_ch(ch)) << 8;

			if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LN)
			{
				OutError(ERROR_109); // Ошибка или отсутствие числового аргумента.
				return false; // стоп компиляция.
			}

			ch = g_pReader->readChar(); // нужно прочитать ещё один раз, чтобы подвинуть курсор
		}
		else if (ch == L'.') // если текущий символ .
		{
			ch = g_pReader->readChar(); // получаем следующий символ

			if (g_GlobalParameters.nAriphmType < 0)
			{
				// для константы возвращаем текущий РС
				result = CorrectAddress(g_GlobalParameters.nPC);
			}
			else
			{
				/*
				Тут надо сделать, чтобы возвращалось не значение, а в таблицу добавлялась
				особая ссылка на метку
				*/
				wchar_t buf[32] = { 0 };
                swprintf(buf, 32, L"!%06o\0", g_GlobalParameters.nPC);
				CBKToken lbl;
				lbl.setName(std::wstring(buf));
				lt &= ~(LABEL_DEFINITE_MASK | LABEL_REFERENCE_MASK); // нам нужны флаги
				AddlLabelReference(&lbl, cp, CURRENTPC_LABEL | lt);
				goto next_op;
			}
		}
		else if (ch == L'^') // если текущий символ ^
		{
			bool bInvert = false; // флаг команды ^С

			if (!Macro11NumberParser(result, ch, bInvert))
			{
				OutError(ERROR_105); // Ошибка или отсутствие числового аргумента.
				return false; // стоп компиляция.
			}

			// если была команда ^C, то начинаем сначала
			if (bInvert)
			{
				if (g_pReader->SkipWhitespaces(ch)) // пропустим пустые символы
				{
					OutError(ERROR_105); // Ошибка или отсутствие числового аргумента.
					return false; // стоп компиляция.
				}

				continue;
			}
		}
		else if (!AdvancedNumberParser(result, ch))
		{
			// !!! в макро11 отсутсвие числового или вообще любого аргумента интерпретируется значением по умолчанию.
			// а здесь аргумент должен присутствовать.
			OutError(ERROR_105); // Ошибка или отсутствие числового аргумента.
			return false; // стоп компиляция.
		}

		if (lt & NEGATIVE_FLAG)
		{
			result = -result;
		}

		if (g_GlobalParameters.bInvert)
		{
			result = ~result;
		}

		accum += result;
next_op:
		g_GlobalParameters.bInvert = false; // отменяем операцию инвертирования
		// проверим, дальше операции есть?
		bool bln = g_pReader->SkipWhitespaces(ch);

		if (bln || !(ch == L'+' || ch == L'-'))
		{
			result = accum;
			return true; // иначе - конец выражения.
		}
	}
}

// добавление метки и продолжение разбора арифм выражения
// вход: cp - текущий PC
// выход: result - результат разбора арифм выражения
//      true - всё нормально
//      false - ошибка
bool trp240(CBKToken *lbl, int &cp, int &result, wchar_t &ch, uint32_t lt)
{
	AddlLabelReference(lbl, cp, lt);
	return trp242(cp, result, ch);
}

// продолжение разбора арифм выражения
// вход: cp - текущий PC
// выход: result - результат разбора арифм выражения
//      true - всё нормально
//      false - ошибка
bool trp242(int &cp, int &result, wchar_t &ch)
{
	g_pReader->SkipWhitespaces(ch);

	if ((ch == L'+') || (ch == L'-'))
	{
		// если есть, то продолжим
		return AriphmParser(cp, result, ch);
	}

	result = 0;
	return true;
}

/*
чтение имени переменной, метки, константы и поиск ее в таблице
Эта функция используется только для парсинга выражений в ветвлениях и sob
выход: result - адрес в таблице
       token - прочитанное имя
false - не найдено.
*/
bool trp202(CBKToken *token, int &result, wchar_t &ch)
{
	if (ch == L'.') // если текущий символ .
	{
		// возвращаем текущий РС
		result = g_GlobalParameters.nPC;
		ch = g_pReader->readChar(); // получаем следующий символ
		return true;
	}

	g_pReader->readToken(token, ch);
	uint32_t lt;
	return SearchLabelInTables(token, result, lt);
}

// установка в опкоде имени регистра или адресации
void SetAddresationRegister(int data)
{
	if (g_GlobalParameters.bOperandType)
	{
		data <<= 6; // если bOperandType, то формируем источник
	}

	g_Memory.w[g_GlobalParameters.nPC / 2] |= data;
}

int GetAddresationRegister()
{
	int data  = g_Memory.w[g_GlobalParameters.nPC / 2];

	if (g_GlobalParameters.bOperandType)
	{
		data >>= 6; // если bOperandType, то формируем источник
	}

	return data;
}

// прочитать и опознать имя регистра
bool ReadRegName(wchar_t &ch)
{
	CBKToken reg;
	g_pReader->readToken(&reg, ch);

	if (ParseRegName(&reg))
	{
		return true;
	}

	return false;
}

/*
опознать имя регистра
вход: reg - прочитанное имя
выход: true - имя регистра опознано
       false - не опознано
*/
bool ParseRegName(CBKToken *reg)
{
	for (auto &RegName : RegNames)
	{
		if (RegName.name == reg->getName())
		{
			SetAddresationRegister(RegName.nNum);
			return true;
		}
	}

	return false;
}

// проверка (Rn)
// выход:
// 0 - всё нормально
// 1 - не распознанный регистр
// 2 - отсутствует открывающая скобка
// 3 - отсутствует закрывающая скобка
int CheckReg(wchar_t &ch)
{
	if (ch != L'(')
	{
		return 2;
	}

	ch = g_pReader->readChar();

	if (!ReadRegName(ch))
	{
		return 1;
	}

	if (ch != L')')
	{
		return 3;
	}

	ch = g_pReader->readChar();
	return 0;
}


// разбор операнда
bool Operand_analyse(int &cp, wchar_t &ch)
{
	int result = 0;
    int ret;
	g_pReader->SkipWhitespaces(ch); // пропустим на всякий случай пробелы
	CReader::CHAR_TYPE ct = g_pReader->getCurrCharType(); // какой символ у нас текущий?

	if (ct == CReader::CHAR_TYPE::LN) // строка закончилась
	{
		goto err115; // ошибка в команде
	}
	else if (ct == CReader::CHAR_TYPE::LETTERS) // если буква - то это или метка или имя регистра
	{
		goto l_letters;
	}
	else if (ct == CReader::CHAR_TYPE::DIGITS || ch == L'^') // если цифра - то это индекс адресации 6, или число в адресации 67
	{
		goto l_digits;
	}
	else
	{
		if (ch == L'@') // если первый символ - собака, то это относительная адресация
		{
			SetAddresationRegister(010); // фиксируем относительную адресацию
			ch = g_pReader->readChar(); // читаем следующий символ
			ct = g_pReader->getCurrCharType();

			if (ct == CReader::CHAR_TYPE::SPACES || ct == CReader::CHAR_TYPE::LN) // если за собакой пусто или конец строки
			{
err103:
				OutError(ERROR_127); // Неверная адресация.
				return false;
			}
			else if (ct == CReader::CHAR_TYPE::LETTERS) // если за собакой буква - значит это метка или регистр
			{
l_letters:
				CBKToken token;
				g_pReader->readToken(&token, ch); // читаем лексему

				if (ParseRegName(&token)) // если это имя регистра
				{
					cp -= 2; // регистровая операция
					return true;
				}

				// если это метка - надо её обработать и разобрать арифметическое выражение
				// метку добавим в таблицу ссылок позже
				trp242(cp, result, ch); // продолжение разбора арифм выражения
				int ret = CheckReg(ch); // за арифм выражением может быть имя регистра в скобках

				switch (ret)
				{
					case 1:
						goto err113; // открывающая скобка есть, но за ней не имя регистра

					case 2:     // открывающей скобки нет - предполагаем адресацию [10]+67
						AddlLabelReference(&token, cp, OFFSET_LABEL);
						goto l_adr67;

					case 3:
						goto err115; // открывающая скобка есть и регистр есть, а закрывающей - нет
				}

				AddlLabelReference(&token, cp, RELATIVE_LABEL);
				goto l_adr60; // всё есть - регистр в скобках - адресация [10]+6х
			}
			else if (ct == CReader::CHAR_TYPE::DIGITS || ch == L'^') // если за собакой цифра - адрес
			{
l_digits:

				if (!AriphmParser(cp, result, ch)) // разбираем арифметическое выражение
				{
					return false;
				}

                ret = CheckReg(ch); // за арифм выражением может быть имя регистра в скобках

				switch (ret)
				{
					case 1:
err113:                 // открывающая скобка есть, но за ней не имя регистра
						OutError(ERROR_113); // Ошибка в имени регистра.
						return false;

					case 2:
						result -= (cp + 2);
						result = CorrectOffset(result);
l_adr67:                // открывающей скобки нет - предполагаем адресацию 67
						SetAddresationRegister(067);
						g_Memory.w[cp / 2] += result;
						return true;

					case 3:
err115:                 // открывающая скобка есть и регистр есть, а закрывающий нет
						OutError(ERROR_127); // Ошибка в команде.
						return false;
				}

l_adr60:
				SetAddresationRegister(060); // всё есть - регистр в скобках - адресация 7х
				g_Memory.w[cp / 2] += result;
				return true;
			}
		}

		if (ch == L'#') // если текущий символ - решётка, (перед ним может быть собака)
		{
			SetAddresationRegister(027); // значит это непосредственная адресация
			ch = g_pReader->readChar(); // смотрим следующий символ

			if (!AriphmParser(cp, result, ch)) // за решёткой может быть только арифметическое выражение
			{
				return false;
			}

			g_Memory.w[cp / 2] += result; // если это было правильное выражение - зафиксируем
			return true;
		}

		if (ch == L'-') // если текущий символ - минус, то это может быть декремент или арифм выражение
		{
			ct = g_pReader->getNextCharType(); // смотрим символ за минусом

			if (ct != CReader::CHAR_TYPE::OTHERS || g_pReader->getNextChar() == L'^') // если за минусом буква, цифра, пусто или конец строки
			{
				goto l_digits;      // идём обрабатывать арифметическое выражение
			}

			ch = g_pReader->readChar(); // иначе - прочитаем символ за минусом
			int ret = CheckReg(ch); // там ожидается регистр в скобках

			switch (ret)
			{
				case 1:
					goto err113; // открывающая скобка есть, но за ней не имя регистра

				case 2:
					goto err103; // открывающий скобки нет. там какая-то ерунда

				case 3:
					goto err115; // открывающая скобка есть и регистр есть, а закрывающий нет
			}

			SetAddresationRegister(040); // всё нормально - декрементная операция
		}
		else // далее или относительная или инкрементная операции
		{
            ret = CheckReg(ch); // там ожидается регистр в скобках

			switch (ret)
			{
				case 1:
					goto err113; // открывающая скобка есть, но за ней не имя регистра

				case 2:
					goto l_digits; // открывающий скобки нет. там какая-то ерунда

				case 3:
					goto err115; // открывающая скобка есть и регистр есть, а закрывающий нет
			}

			// всё нормально - уточним операцию
			if (ch == L'+') // если сразу за закрывающей скобкой есть +
			{
				ch = g_pReader->readChar(); // его прочитаем
				SetAddresationRegister(020); // и это инкрементная операция
			}
			else
			{
				// если конструкция @(Rn) то ошибка
				// а MACRO-11 интерпретирует её как @0(Rn)
				if (GetAddresationRegister() == 010)
				{
					goto err103; // не допустим этого
				}

				SetAddresationRegister(010); // нет плюса - относительная
			}
		}

		cp -= 2; // регистровая операция
	}

	return true;
}

// поиск заданного символа.
// если найден - то остановка на нём и выход - true
// если не найден - выход - false и остановка на том, что найдено
bool needChar(wchar_t nch, wchar_t &ch)
{
	bool bln = g_pReader->SkipWhitespaces(ch); // пропускаем возможные пробелы

	if (!bln) // если не конец строки
	{
		return (ch == nch); // смотрим, то ли нашли, что нам нужно.
	}

	return false;
}


////////////////////////////////////////////////////////////////////////
// парсер чисел.

bool HexNumberParser(wchar_t &ch, int &ret)
{
	bool bOk = true;
	ret = 0;

	while (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::DIGITS || g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LETTERS)
	{
		int d = 0;

		if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::DIGITS)
		{
			d = (ch - L'0');
		}
		else if (L'A' <= ch && ch <= L'F')
		{
			d = 10 + (ch - L'A');
		}
		else
		{
			bOk = false;
			break;
		}

		ret *= 16;
		ret += d;
		ch = g_pReader->readChar();
	}

	return bOk;
}

bool DecNumberParser(wchar_t &ch, int &ret)
{
	ret = 0;

	while (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::DIGITS)
	{
		ret *= 10;
		ret += (ch - L'0');
		ch = g_pReader->readChar();
	}

	return true;
}

bool OctNumberParser(wchar_t &ch, int &ret)
{
	ret = 0;
	bool bOk = true;

	while (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::DIGITS)
	{
		if (L'0' <= ch && ch <= L'7')
		{
			ret *= 8;
			ret += ch & 7;
		}
		else
		{
			// если встретится 8 или 9 считаем их не цифрами и говорим, что ошибка парсинга
			bOk = false;
			break;
		}

		ch = g_pReader->readChar();
	}

	return bOk;
}

bool BinNumberParser(wchar_t &ch, int &ret)
{
	bool bOk = true;
	ret = 0;

	while (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::DIGITS)
	{
		if (L'0' <= ch && ch <= L'1')
		{
			ret *= 2;
			ret += ch & 1;
		}
		else
		{
			bOk = false;
			break;
		}

		ch = g_pReader->readChar();
	}

	return bOk;
}


/*взято из кросс ассемблера MACRO-11
 Copyright (c) 2001, Richard Krehbiel
 формат внутреннего представления плавающего числа.
 первое слово: бит 15 - знак, затем 8 бит - порядок, затем 7 бит - мантисса
 второе слово: продолжение мантиссы
 третье и 4е слово: продолжение мантиссы

 соответственно точность определяется отбрасыванием 2, 3 и 4 слов мантиссы
 */

/* Parse PDP-11 64-bit floating point format. */
/* Give a pointer to "size" words to receive the result. */
/* Note: there are probably degenerate cases that store incorrect
results.  For example, I think rounding up a FLT2 might cause
exponent overflow.  Sorry. */
/* Note also that the full 49 bits of precision probably aren't
available on the source platform, given the widespread application
of IEEE floating point formats, so expect some differences.  Sorry
again. */
bool parse_float(wchar_t &ch,
                 int size,   // 1 для ^f; 2 для .FLT2; 4 для .FLT4
                 uint16_t *flt) // массив из 1..4х слов, куда сохраняется результат
{
	// сперва надо прочитать в буфер всё число.
	std::wstring str;

	while (g_pReader->getCurrCharType() != CReader::CHAR_TYPE::SPACES && g_pReader->getCurrCharType() != CReader::CHAR_TYPE::LN)
	{
		// вот ещё одна проверка на посторонние символы
		if (ch == L'+' || ch == L'-' || ch == L'E' || ch == L'.' || g_pReader->getCurrCharType() == CReader::CHAR_TYPE::DIGITS)
		{
			if (ch == L'.')
			{
				ch = L','; // меняем точку на запятую, потому что для scanf нужна ','
			}

			str.push_back(ch); // соберём все цифры в строку.
			ch = g_pReader->readChar();
		}
		else
		{
			break;
		}
	}

	for (int i = 0; i < size; ++i)
	{
		flt[i] = 0;  // очищаем результат
	}

	double d = 0;   // значение
	int n = 0;
	int i = swscanf(str.c_str(), L"%lf%n", &d, &n); // парсим число, на выходе i == кол-во сконвертирвоанных полей

	if (i == 0)
	{
		OutError(ERROR_131); // сообщим об этом
		return false;    // не получилось сконвертировать.
	}

	// если обработали меньше символов, чем захватили - то захватили
	// какие-то посторонние символы, которых быть там не должно
	if (size_t(n) < str.length())
	{
		OutError(ERROR_103); // сообщим об этом
	}

	uint16_t uexp = 0;              // экспонента
	uint64_t ufrac = 0;             // мантисса
	uint16_t sign = 0;              // маска знака

	if (d != 0.0)
	{
		int sexp;   // знаковый порядок
		double frac = frexp(d, &sexp);  // отделяем порядок и мантиссу

		if (frac < 0)
		{
			sign = 0100000;             // знак минуса
			frac = -frac;               // корректируем мантиссу
		}

		/* это длинное число - это 2 в степени 56 */
		auto ufrac = uint64_t(frac * 72057594037927936.0);  // выравниваем биты мантиссы

		/* Round from FLT4 to FLT2 */
		if (size < 4)
		{
			if (size < 2)
			{
				ufrac += 0x800000000000;    // округляем для 16-битного представления
			}
			else
			{
				ufrac += 0x80000000;        // округляем для 32-битного представления
			}

			if (ufrac >= 0x100000000000000) // переполнение? (это то же число 2 в степени 56)
			{
				ufrac >>= 1;                // нормализуем
				sexp--;
			}
		}

		if (sexp < -128 || sexp > 127)
		{
			// порядок выходит за пределы?
			OutError(ERROR_131); // сообщим об этом
			return false;
		}

		uint16_t uexp = sexp + 128;     // делаем беззнаковый порядок - 128
		uexp &= 0xff;                   // оставляем только младший байт
	}

	// если d == 0.0, то сохраняем результат - точный 0
	flt[0] = uint16_t((sign | (uexp << 7) | ((ufrac >> 48) & 0x7F))); // первое слово

	if (size > 1)
	{
		flt[1] = uint16_t((ufrac >> 32) & 0xffff); // второе слово

		if (size > 2)
		{
			flt[2] = uint16_t((ufrac >> 16) & 0xffff);  // третье
			flt[3] = uint16_t(ufrac & 0xffff);          // четвёртое
		}
	}

	return true;
}


/*
парсер числа
выход: true - число в result достоверно
false - было переполнение, и число в result недостоверно

Попадая сюда имеем лексему, гарантированно начинающуюся с цифры

префиксная форма - без проблем.
0xabcd - 16ричное число с префиксом
0d9999 - 10чное число с префиксом - даж не знаю, нужно ли.
07777 - 8 ричное число с префиксом
0b1111 - двоичное число с префиксом
0o777 - сделать можно, но бессмысленно
9999. - 10чное число с суффиксом  - без проблем
7777 - 8 ричное число по умолчанию  - без проблем
*/
bool AdvancedNumberParser(int &result, wchar_t &ch)
{
	int nTmp = 0;
	bool bRet = true;
	std::wstring str;
    bool bHas8 = false;

	if (ch == L'0') // если первая цифра 0
	{
		// и если за нулём спец код
		if (g_pReader->getNextChar() == L'x' || g_pReader->getNextChar() == L'X')
		{
			ch = g_pReader->readChar();
			ch = g_pReader->readChar();
			// парсим 16ричное число
			bRet = HexNumberParser(ch, nTmp);
			goto l_aex;
		}
		else if (g_pReader->getNextChar() == L'd' || g_pReader->getNextChar() == L'D')
		{
			ch = g_pReader->readChar();
			ch = g_pReader->readChar();
			// парсим 10чное число
			bRet = DecNumberParser(ch, nTmp);
			goto l_aex;
		}
		else if (g_pReader->getNextChar() == L'b' || g_pReader->getNextChar() == L'B')
		{
			ch = g_pReader->readChar();
			ch = g_pReader->readChar();
			// парсим 2чное число
			bRet = BinNumberParser(ch, nTmp);
			goto l_aex;
		}
	}

	// а иначе, парсим просто число, 8чное или 10чное, зависит от точки на конце.
	// конструкция 0123. допустима, и считается десятичным числом, несмотря на ведущий 0

	while (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::DIGITS)
	{
		str.push_back(ch); // соберём все цифры в строку.

		// заодно проверим, есть ли в числе цифры 8 и 9
		if (ch == L'8' || ch == L'9')
		{
			bHas8 = true;
		}

		ch = g_pReader->readChar();
	}

	if (ch == L'.') // если завершающий символ точка
	{
		ch = g_pReader->readChar(true); // точку тоже захватим
		// парсим 10чное число
		nTmp = wcstol(str.c_str(), nullptr, 10);
	}
	else if (bHas8)
	{
		// ошибка, десятичное число без точки
		nTmp = 0;
		bRet = false;
	}
	else
	{
		// парсим 8чное число
		nTmp = wcstol(str.c_str(), nullptr, 8);
	}

l_aex:
	result = nTmp;

	if (!bRet)
	{
		OutError(ERROR_135); // ошибка в числе
	}

	if (nTmp > 65535)
	{
		result &= 0xffff;
		OutError(ERROR_134); // переполнение слова
		return false;
	}

	return bRet;
}

// если при разборе арифметического выражения встретился символ ^,
// то его читаем и вызываем эту функцию,
// на входе: ch - следующий за ^ символ.
/*
префиксная форма макро11:
^xabcd - 16ричное число с префиксом
^habcd - 16ричное число с префиксом
^d9999 - 10чное число с префиксом
^o7777 - 8 ричное число с префиксом
^b1111 - двоичное число с префиксом
^f12.3 - плавающее число.
^rabc - три символа в коде Radix - 50
^c - следующий операнд надо будет инвертировать
*/
bool Macro11NumberParser(int &result, wchar_t &ch, bool &bInvert)
{
	int nTmp = 0;
	ch = g_pReader->readChar();
	bool bRet = true;

	switch (ch)
	{
		case L'X':
		case L'H':
			ch = g_pReader->readChar();
			// парсим 16ричное число
			bRet = HexNumberParser(ch, nTmp);
			break;

		case L'D':
			ch = g_pReader->readChar();
			// парсим 10чное число
			bRet = DecNumberParser(ch, nTmp);
			break;

		case L'O':
			ch = g_pReader->readChar();
			// парсим 8чное число
			bRet = OctNumberParser(ch, nTmp);
			break;

		case L'B':
			ch = g_pReader->readChar();
			// парсим 2чное число
			bRet = BinNumberParser(ch, nTmp);
			break;

		case L'F':
		{
			ch = g_pReader->readChar();
			uint16_t flt[2];

			// парсим плавающее число
			if (bRet = parse_float(ch, 1, flt))
			{
				nTmp = flt[0];
			}

			break;
		}

		case L'R':
		{
			ch = g_pReader->readChar();
			// парсим радикс
			result = 0;
			int nMultipler = 03100;

			for (int i = 0; i < 3; ++i) // берём три следующих символа
			{
				int nChCode = 0;

				for (int n = 0; n < 050; ++n)
				{
					if (RADIX50[n] == ch)
					{
						nChCode = n;    // нашли
						break;
					}
				}

				// неверные символы радикс просто считаем пробелами.
				result += nChCode * nMultipler; // упаковываем очередной символ
				nMultipler /= 050;
				// следующий символ
				ch = g_pReader->readChar();

				// если , - следующий операнд
				// если CT_LN - то конец строки, значит конец операнда
				// если CT_SPACES - то конец операнда. пробелы в операндах в данной команде не допускаются
				// если ; - то начался комментарий
				// или с-подобный комментарий
				if (ch == L',' || ch == L';' || g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LN /* || g_pReader->getCurrCharType() == CReader::CHAR_TYPE::SPACES*/
				        || (ch == L'/' && (g_pReader->getNextChar() == L'/' || g_pReader->getNextChar() == L'*')))
				{
					// прервёмся
					break;
				}
			}

			return true;
		}

		case L'C':
			ch = g_pReader->readChar();
			bInvert = true;
			g_GlobalParameters.bInvert = !g_GlobalParameters.bInvert; // будем инвертировать
			return true;

		default:
			result = 0; // ошибка - неверный символ в строке.
			return false;
	}

	result = nTmp;

	if (!bRet)
	{
		OutError(ERROR_135); // ошибка в числе
	}

	if (nTmp > 65535)
	{
		result &= 0xffff;
		OutError(ERROR_134); // переполнение слова
		return false;
	}

	return bRet;
}
