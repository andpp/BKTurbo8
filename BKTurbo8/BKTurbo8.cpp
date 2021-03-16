// BKTurbo8.cpp: определяет точку входа для консольного приложения.
//

#include "pch.h"
#include "BKTurbo8.h"
#include "Globals.h"
#include "ErrorManager.h"
#include "BKToken.h"
#include "Assemble.h"
#include "LabelManager.h"
#include "Parser.h"
#include "Listing.h"
#include "Object.h"

//#include <io.h>
#include <clocale>
#include "getopt.h"
//#pragma warning(disable:4996)
#include <dirent.h>


constexpr auto EXT_OBJ = 0;
constexpr auto EXT_LST = 1;
constexpr auto EXT_BIN = 2;
constexpr auto EXT_RAW = 3;
static std::string m_strExt[] =
{
    ".obj",
    ".lst",
    ".bin",
    ".raw"
};

std::string strListingFilename, strObjectFileName, strTableFileName, strResultFile;
OP_MODE operation = OP_MODE::UNKN;

int nStartAddress = -1;
bool m_bMakeListing = false;
bool m_bMakeObject = false;
bool m_bMakeTable = false;
bool m_bMakeRaw = false;

using namespace std;

int main(int argc, char *argv[])
{
//	_wsetlocale(LC_ALL, L"Russian");
    std::setlocale(LC_ALL, "");
	InitGlobalParameters(&g_GlobalParameters);
    char curDir[_MAX_PATH] = { 0 };
    getcwd(curDir, _MAX_PATH);
    g_GlobalParameters.strAppPath = std::string(curDir);
	static struct option long_options[] =
	{
        { "help",        ARG_NONE, nullptr, '?' },
        { "verbose",     ARG_NONE, nullptr, 'v' },
        { "raw",         ARG_NONE, nullptr, 'r' },
        { "input",       ARG_REQ,  nullptr, 'i' },
        { "listing",     ARG_OPT,  nullptr, 'l' },
        { "object",      ARG_OPT,  nullptr, 'o' },
        { "table",       ARG_OPT,  nullptr, 't' },
        { "address",     ARG_REQ,  nullptr, 's' },
        { nullptr,       ARG_NULL, nullptr, ARG_NULL }
	};
    static char optstring[] = "?vri:l::o::t::s:";
	bool bShowHelp = false;
	int option_index = 0;
	int c;

	while ((c = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1)
	{
		// Handle options
		c = tolower(c);

		switch (c)
		{
			case L'?':
				bShowHelp = true;
				break;

			case L'v':
				g_GlobalParameters.bVerbosity = true;
				break;

			case L'i': // задаём кодировку входного файла
			{
                std::string strTmp = (optarg) ? std::string(optarg) : "a";

				if (!strTmp.empty())
				{
					wchar_t wch = tolower(strTmp.at(0)); // оставим только первую букву

					switch (wch)
					{
						case L'k':
							Charset = CReader::FILE_CHARSET::KOI8;
							break;

						case L'o':
							Charset = CReader::FILE_CHARSET::CP866;
							break;

						case L'w':
							Charset = CReader::FILE_CHARSET::CP1251;
							break;

						case L'8':
							Charset = CReader::FILE_CHARSET::UTF8;
							break;

						case L'u':
							Charset = CReader::FILE_CHARSET::UTF16LE;
							break;

						case L'a':
						default:
							Charset = CReader::FILE_CHARSET::UNDEFINE;
							break;
					}
				}
			}
			break;

			case L'r':
				m_bMakeRaw = true;
				break;

			case L'l':
				m_bMakeListing = true;

				if (optarg)
				{
                    strListingFilename =  std::string(optarg);
				}

				break;

			case L'o':
				m_bMakeObject = true;

				if (optarg)
				{
                    strObjectFileName = std::string(optarg);
				}

				break;

			case L't':
				m_bMakeTable = true;

				if (optarg)
				{
                    strTableFileName = std::string(optarg);
				}

				break;

			case L's':
			{
                int nTmp = stol(optarg, nullptr, 8) & 0xffff;

				if (0 <= nTmp && nTmp <= HIGH_BOUND)
				{
					nStartAddress = nTmp;
				}
			}
			break;
		}

		if (bShowHelp)
		{
			Usage();
			return 0;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1)
	{
        printf("Ошибка в командной строке: отсутствует команда.\n\n");
		Usage();
	}
	else
	{
        std::string strCommand = strToLower(std::string(*argv).substr(0, 2));

        if (strCommand == "li")
		{
			operation = OP_MODE::LINK;
		}
        else if (strCommand == "co")
		{
			operation = OP_MODE::COMP;
		}
        else if (strCommand == "cl")
		{
			operation = OP_MODE::CL;
		}
		else
		{
            printf("Ошибка в командной строке: неопознанная команда.\n\n");
			Usage();
			return 0;
		}

		argc--;
		argv++;

		if (argc < 1)
		{
            printf("Ошибка в командной строке: не хватает аргументов после команды.\n\n");
			Usage();
			return 0;
		}

		if (operation == OP_MODE::LINK)
		{
            strResultFile = std::string(*argv);
			argc--;
			argv++;

			if (argc < 1)
			{
                printf("Ошибка в командной строке: не хватает аргументов после команды LI.\n\n");
				Usage();
				return 0;
			}

            printf("Линковка...\nСоздаётся файл: %s\n", strResultFile.c_str());

			if (nStartAddress != -1) // если в командной строке был задан адрес загрузки
			{
				// то применим его
				SetStartAddress(nStartAddress);
				g_GlobalParameters.bLinkerFirst = false; // а адреса объектных модулей будем игнорировать
			}
		}
		else if (operation == OP_MODE::COMP)
		{
			if (nStartAddress != -1) // если в командной строке был задан адрес загрузки
			{
				// то применим его
				SetStartAddress(nStartAddress);
			}
		}

//		struct _wfinddatai64_t f_file;
//		intptr_t hFile;

		bool bRes = true;
        struct stat f_file;

		for (; argc > 0; argc--, argv++) // теперь, пока аргументы не кончатся - обрабатывать
		{
			// тут теряется относительный путь в маске поиска. и как это исправить, пока не ясно.
//			if ((hFile = _wfindfirsti64(*argv, &f_file)) == -1L)
            if (stat(*argv, &f_file) == -1)
			{
                printf("Файл(ы) '%s' не найден(ы)\n", *argv);
			}
			else
			{
//				do
                {
//					if (f_file.attrib & _A_SUBDIR) // если директория
                    if ((f_file.st_mode & S_IFREG) == 0)
                    {
                        // Ignore all except regular files

//						if ((strcmp(f_file.name, "..") == 0) || (strcmp(f_file.name, ".") == 0))
//						{
//							continue;
//						}

                        // остальные директории игнорируем
                    }
                    else
					{
                        std::string strFilePath;
                        char strDrive[_MAX_DIR];
                        char strPath[_MAX_PATH];
						// выделим путь из исходного шаблона
//						_tsplitpath_s(*argv, strDrive, _MAX_DIR, strPath, _MAX_PATH, nullptr, 0, nullptr, 0);

//                        if (!std::string(strDrive).empty()) // если задан привод, то в маске был полный путь
//						{
//							strFilePath = std::string(strDrive) + std::string(strPath);
//						}
//						else
//						{
//                            char CurrDir[_MAX_PATH];
//                            getcwd(CurrDir, _MAX_PATH); // текущая директория
//                            strFilePath = std::string(CurrDir) + "/" + std::string(strPath);
//						}

						if (operation == OP_MODE::LINK)
						{
                            printf("Линкуется объектный файл: %s ", *argv);
                            strFilePath = std::string(*argv);
//                            bRes = LoadObj(strFilePath + std::string(*argv));
                            bRes = LoadObj(strFilePath);

							if (bRes)
							{
                                printf("- OK\n");
								LabelLinking();
							}
							else
							{
                                printf("- Ошибка\n");
								break;
							}
						}
						else if (operation == OP_MODE::COMP || operation == OP_MODE::CL)
						{
//                            workCycle(strFilePath + std::string(*argv));
                            strFilePath = std::string(*argv);
                            workCycle(strFilePath);
							ReInitGlobalParameters(&g_GlobalParameters);
							// чистим все векторы.
							g_labelRefs.Clear();
							g_labelGlobalDefs.Clear();
							g_labelLocalDefs.Clear();
							g_Listing.clear();
						}
					}
				}
//				while (_wfindnexti64(hFile, &f_file) == 0);

//				_findclose(hFile);
			}

			if (!bRes)
			{
				break;
			}
		}

		if (operation == OP_MODE::LINK && bRes)
		{
            printf("Адрес компоновки = %#6o\n", GetStartAddress());
            printf("Размер исполняемого файла = %#6o\n", g_GlobalParameters.nProgramLength);
			SaveFile(strResultFile);

			if (m_bMakeTable)
			{
//				char pName[_MAX_FNAME] = { 0 };
//				_wsplitpath_s((strTableFileName.empty() ? strResultFile : strTableFileName).c_str(), nullptr, 0, nullptr, 0, pName, _MAX_FNAME, nullptr, 0);
//				std::string name = std::string(pName) + "_tbl";
                std::string name = std::string(basename((strTableFileName.empty() ? strResultFile : strTableFileName).c_str())) + "_tbl";
				MakeObj(name, m_strExt[EXT_OBJ], true);
			}

			if (m_bMakeObject)
			{
				MakeObj(strObjectFileName.empty() ? strResultFile : strObjectFileName, m_strExt[EXT_OBJ]);
			}

			if (m_bMakeListing)
			{
				MakeListing(strListingFilename.empty() ? strResultFile : strListingFilename, m_strExt[EXT_LST]);
			}

			PrintLabelTable(strListingFilename.empty() ? strResultFile : strListingFilename, m_strExt[EXT_LST]);
		}
	}

	return 0;
}

int workCycle(std::string &strInFileName)
{
	v_Reader.emplace_back(nullptr); // условие выхода из цикла
	g_pReader = new CReader(strInFileName, Charset);
#if (DEBUG_LABEL_MANAGER)
	DebugInit();
#endif

	if (!g_pReader)
	{
        printf("Недостаточно памяти.\n");
		return -1;
	}

	if (operation == OP_MODE::COMP)
	{
        printf("Компиляция исполняемого файла...\n");
		// режим компиляции CO
		g_GlobalParameters.nModeLinkage = 1;
	}
	else if (operation == OP_MODE::CL)
	{
        printf("Компиляция объектного файла...\n");
		// режим компиляции CL
		g_GlobalParameters.nModeLinkage = -1;
	}

    printf("Файл: %s\nАвтоопределение кодировки: ", strInFileName.c_str());

	switch (g_pReader->GetFileCharset())
	{
		case CReader::FILE_CHARSET::KOI8:
            printf("КОИ8-Р\n");
			break;

		case CReader::FILE_CHARSET::CP866:
            printf("OEM CP-866\n");
			break;

		case CReader::FILE_CHARSET::CP1251:
            printf("ANSI CP-1251\n");
			break;

		case CReader::FILE_CHARSET::UTF8:
            printf("UTF-8\n");
			break;

		case CReader::FILE_CHARSET::UTF16LE:
            printf("UTF-16LE(Unicode)\n");
			break;

		case CReader::FILE_CHARSET::UNDEFINE:
            printf("Не определена. Возможно, это не текстовый файл.\n");
			return -2;

		default:
            printf("\rОшибка открытия файла.     \n");
			return -3;
	}

	do
	{
		// первый проход
		while (!g_GlobalParameters.bENDReached && !g_pReader->isEOF())
		{
			ParseLine();

			if (g_GlobalParameters.nPC >= HIGH_BOUND)
			{
                printf("Критическая ошибка:: Достигнут предел свободной памяти!\n");
				break;
			}
		}

		if (!g_GlobalParameters.bENDReached)
		{
			OutError(ERROR_121, false); // Отсутствует .END
		}

		delete g_pReader; // удаляем текущий экземпляр
		g_pReader = nullptr;

		if (!v_Reader.empty()) // если в векторе что-то есть
		{
			g_pReader = v_Reader.back(); // достаём
			v_Reader.pop_back();

			if (g_pReader)
			{
				g_GlobalParameters.bENDReached = false;
			}
		}
	}
	while (g_pReader);   // и продолжаем, пока есть что обрабатывать

	int nErr = IsError();

	if (nErr)
	{
		// если были ошибки - одно действие
        printf("Количество ошибок: %d\n", nErr);
	}
	else
	{
		// если не было ошибок - другое
		if (operation == OP_MODE::CL)
		{
			MakeObj(strObjectFileName.empty() ? strInFileName : strObjectFileName, m_strExt[EXT_OBJ]);
		}
		else
		{
            printf("Адрес компоновки = %#6o\n", GetStartAddress());
			SaveFile(strInFileName);

			if (m_bMakeObject)
			{
				MakeObj(strObjectFileName.empty() ? strInFileName : strObjectFileName, m_strExt[EXT_OBJ]);
			}
		}

        printf("Размер исполняемого файла = %#6o\n", g_GlobalParameters.nProgramLength);

		if (m_bMakeTable)
		{
//			wchar_t pName[_MAX_FNAME] = { 0 };
//			_wsplitpath_s((strTableFileName.empty() ? strInFileName : strTableFileName).c_str(), nullptr, 0, nullptr, 0, pName, _MAX_FNAME, nullptr, 0);
//			std::wstring name = std::wstring(pName) + L"_tbl";
            std::string name = std::string(basename((strTableFileName.empty() ? strInFileName : strTableFileName).c_str())) + "_tbl";
            MakeObj(name, m_strExt[EXT_OBJ], true);
		}
	}

	// листинг делаем в любом случае

	if (m_bMakeListing)
	{
		MakeListing(strListingFilename.empty() ? strInFileName : strListingFilename, m_strExt[EXT_LST]);
	}

	PrintLabelTable(strListingFilename.empty() ? strInFileName : strListingFilename, m_strExt[EXT_LST]);
	return 0;
}


/*
Пропускаем остаток строки.
Выход: true - встретился конец файла
false - просто конец строки.
*/
bool SkipTailString(wchar_t &ch)
{
	do
	{
		ch = g_pReader->readChar();
	}
	while (!g_pReader->isEOF() && g_pReader->getCurrCharType() != CReader::CHAR_TYPE::LN);

	if (g_pReader->isEOF())
	{
		return true;
	}

	return false;
}

/*
*Парсим строку, в строке может быть неограниченное число ассемблерных команд.
*Перед командой может быть неограниченное число меток.
*Конец файла тоже считается концом строки, затем надо делать конкретную проверку на именно конец файла.
g_pReader->m_nLineNum
*/
void ParseLine()
{
	CBKToken token;
	int cp = g_GlobalParameters.nPC;    // текущий временный PC, который приращивается во время построения инструкции или псевдокоманды
	Lst_PrepareLine(cp);
	wchar_t ch = g_pReader->readChar(); // читаем первый символ

	for (;;) // крутимся в цикле, пока не достигнем конца строки
	{
		// делаем так, чтобы можно было обрабатывать несколько инструкций в одной строке
		g_GlobalParameters.bInString = false;
		g_GlobalParameters.nAriphmType = 0;
		g_GlobalParameters.nPC = cp; // обычно сюда попадаем перед началом очередной команды, поэтому сохраняем счётчик.

		if (g_pReader->SkipWhitespaces(ch))  // пропустим возможные пробелы
		{
			// если после чтения получился конец строки,
			break;  // то выход из цикла.
		}

		// обрабатываем многострочный комментарий /*...*/
		// нормально парсится инлайн коммент, за которым может быть инструкция.
		// комментарий не может быть внутри инструкции, т.к. такой парсинг не реализован
		if (ch == L'/' && g_pReader->getNextChar() == L'*')
		{
			bool bex = false;
			Lst_AddPrepareLine(0, ListType::LT_COMMENTARY);

			do
			{
				ch = g_pReader->readChar();

				if (g_pReader->getCurrCharType() == CReader::CHAR_TYPE::LN)
				{
					Lst_PrepareLine(cp);
					Lst_AddPrepareLine(0, ListType::LT_COMMENTARY);
				}

				if (g_pReader->isEOF())
				{
					bex = true;
					break;
				}
			}
			while (!(ch == L'*' && g_pReader->getNextChar() == L'/'));

			if (bex)
			{
				break;
			}
			else
			{
				ch = g_pReader->readChar();     // читаем *
				ch = g_pReader->readChar(true); // читаем /
				continue;
			}
		}

		// теперь обрабатываем строку
		if (ch == L';' || (ch == L'/' && g_pReader->getNextChar() == L'/')) // если комментарий ";" или "// "
		{
			SkipTailString(ch);
			Lst_AddPrepareLine(0, ListType::LT_COMMENTARY);
			break; // выходим из цикла, независимо от результата
		}
		else if (ch == L'.') // если псевдокоманда или точка
		{
			// обработка псевдокоманды
			ch = g_pReader->readChar(); // читаем следующий за точкой символ.

			// присваивание значения точке
			if (needChar(L'=', ch))
			{
				Lst_AddPrepareLine(cp, ListType::LT_ASSIGN);
				ch = g_pReader->readChar(); // читаем следующий символ.
				int result = 0;
				g_GlobalParameters.nAriphmType = -1;

				if (AriphmParser(cp, result, ch))
				{
					cp = CorrectOffset(result);
					g_GlobalParameters.nPC = cp;
					Lst_AddPrepareLine2(cp);
				}
				else
				{
					SkipTailString(ch);
					break;
				}
			}
			else
			{
				if (!g_pReader->readToken(&token, ch)) // и читаем лексему
				{
					// если токен не прочёлся, значит сразу за точкой встретился посторонний символ.
					OutError(ERROR_103);
					SkipTailString(ch);
					break;
				}

				if (!PseudoCommandExec(&token, cp, ch))
				{
					// если встретилась ошибка, пропустим всю строку
					SkipTailString(ch);
				}
				else
				{
					if (g_GlobalParameters.bStepInclude)
					{
						g_GlobalParameters.bStepInclude = false;
						break;
					}
				}

				Lst_AddPrepareLine2(cp);
			}
		}
		else // дальше что-то, либо метка, либо присваивание, либо команда.
		{
			if (!g_pReader->readToken(&token, ch)) // считаем это лексемой
			{
				// если токен не прочёлся, значит сразу встретился посторонний символ.
				OutError(ERROR_103); // Недопустимый символ в строке.
				SkipTailString(ch);
				break;
			}

			// если сразу за лексемой идёт двоеточие
			if (ch == L':') // значит это метка.
			{
				ch = g_pReader->readChar(true);         // пропускаем ':' и пустоту за ним
				wchar_t wch = token.getName().at(0);    // определяем тип метки.
				// если метка начинается с буквы - это глобальная метка
				// если с цифры - то это локальная метка.
				// остальные символы - любые допустимые.
				bool bl;

				if (g_pReader->AnalyseChar(wch) == CReader::CHAR_TYPE::DIGITS)
				{
					// локальная метка
					bl = AddLocalLabel(&token, g_GlobalParameters.nPC, DEFINITE_LABEL);
				}
				else
				{
					// глобальная метка
					bl = AddGlobalLabel(&token, g_GlobalParameters.nPC, DEFINITE_LABEL);
				}

				Lst_AddPrepareLine(cp, ListType::LT_LABEL);

				if (bl) // если метка добавилась в таблицу
				{
					LabelLinking(); // то линкуем
				}
				else
				{
					OutError(ERROR_116); // иначе ошибка - повторное определение метки
				}
			}
			else if (needChar(L'=', ch)) // если за лексемой, возможно через пробелы идёт '=',
			{
				// то это присваивание
				ch = g_pReader->readChar(true); // пропустим =
				g_GlobalParameters.nAriphmType = -1;
				int result = 0;
				Lst_AddPrepareLine(cp, ListType::LT_ASSIGN);

				if (AriphmParser(cp, result, ch)) // если арифметическое выражение распарсилось
				{
					if (!g_labelGlobalDefs.AddLabel(&token, result, CONSTANT_LABEL)) // добавляем метку со значением, без удаления локальных меток
					{
						OutError(ERROR_116); // если не добавилось - ошибка. уже есть такая метка
					}

					Lst_AddPrepareLine2(result);
				}
				else
				{
					SkipTailString(ch);
					break;
				}
			}
			else
			{
				g_pReader->SkipWhitespaces(ch); // пропускаем пробелы - на всякий случай

				// обработка ассемблерной команды.
				if (!AssembleCPUInstruction(&token, cp, ch))
				{
					// assert(false);
					// если встретилась ошибка, пропустим всю строку
					SkipTailString(ch);
				}

				Lst_AddPrepareLine2(cp);
			}
		}
	}
}

void SaveFile(const std::string &strName)
{
    std::string pPath;
    std::string pName;
    std::string pExt;
    wsplitpath_s(strName, pPath, pName, pExt);

	// если получается очень большой файл
	if (g_GlobalParameters.nProgramLength >= HIGH_BOUND)
	{
		m_bMakeRaw = true; // то бин делать никак нельзя
	}

    std::string strTmp = std::string(pPath) + std::string(pName);
	// если у файла стандартное расширение, то его убираем, остальные - оставляем
    std::string strExt = trim(strToLower(std::string(pExt)), ' ');

    if (strExt != ".asm" && strExt != ".obj")
	{
        strTmp += std::string(pExt);
	}

	// сделаем новое имя файла - добавим расширение бин или рав
    std::string outName = strTmp + m_strExt[m_bMakeRaw ? EXT_RAW : EXT_BIN];
    FILE *of = fopen(outName.c_str(), "wb");

	if (of)
	{
		if (!m_bMakeRaw)
		{
			uint16_t bin[2];
			bin[0] = GetStartAddress();
			bin[1] = g_GlobalParameters.nProgramLength;
			fwrite(bin, 1, 4, of);
		}

		fwrite(&g_Memory.b[BASE_ADDRESS], 1, g_GlobalParameters.nProgramLength, of);
		fclose(of);
	}
	else
	{
        printf("Ошибка создания файла %s\n", outName.c_str());
	}
}

// вывод на экран таблицы меток.
void PrintLabelTable(const std::string &strName, const std::string &strExt)
{
    std::string pPath;
    std::string pName;
    std::string pExt;
    wsplitpath_s(strName, pPath, pName, pExt);
    // сделаем новое имя файла
    std::string outName = std::string(pPath) + std::string(pName) + strExt;
	FILE *of = nullptr;

	if (m_bMakeListing)
	{
        of = fopen(outName.c_str(), "a+t");
	}

	// выведем таблицу глобальных меток.
    std::wstring wstr1 = L"\nГлобальные метки\n\n";

	if (g_GlobalParameters.bVerbosity)
	{
        printf("%s", wstring_to_utf8(wstr1).c_str());
	}

    if (of)
    {
//        size_t len = wstr1.length();
//        auto *pbuf = new uint8_t[len + 1];

//        if (pbuf)
//        {
//            UNICODEtoBK(wstr1, pbuf, len, false);
//            pbuf[len] = 0;
//            fprintf(of, "%s", pbuf);
//            delete[] pbuf;
//        }
        fprintf(of, "%s", wstring_to_utf8(wstr1).c_str());
    }

	size_t sz = g_labelGlobalDefs.getSize();
	size_t maxlen = 0;

	// сперва найдём самую длинную метку
	for (size_t i = 0; i < sz; ++i)
	{
		std::wstring name = g_labelGlobalDefs.GetLabel(i)->getName();

		if (name.length() > maxlen)
		{
			maxlen = name.length();
		}
	}

	// теперь сформируем строку формата
    static char buf[32] = { 0 };

	if (maxlen > 16)
	{
		maxlen = 16;
	}

    snprintf(buf, 32, "%%-%ds = %%07o", int(maxlen));
	int n = 84 / (int(maxlen) + 12); // вот столько влазит в экран
	int j = n;

	for (size_t i = 0; i < sz; ++i)
	{
		std::wstring name = g_labelGlobalDefs.GetLabel(i)->getName();
		int value = g_labelGlobalDefs.GetValue(i);

		if ((g_labelGlobalDefs.GetType(i) & LABEL_DEFINITE_MASK) == DEFINITE_LABEL)
		{
			value = CorrectAddress(value);
		}

		if (name.length() > maxlen)
		{
			// если метка очень длинная, то её только одну в строке выведем.
			if (g_GlobalParameters.bVerbosity)
			{
                printf("%-s = %07o", wstring_to_utf8(name).c_str(), value);
			}

			if (of)
			{
                fprintf(of, "%-s = %07o", wstring_to_utf8(name).c_str(), value);
			}

			j = n;

			if (g_GlobalParameters.bVerbosity)
			{
                printf("\n");
			}

			if (of)
			{
                fprintf(of, "\n");
			}
		}
		else
		{
			if (g_GlobalParameters.bVerbosity)
			{
                printf(buf, wstring_to_utf8(name).c_str(), value);
			}

			if (of)
			{
                fprintf(of, buf, wstring_to_utf8(name).c_str(), value);
			}

			if (--j <= 0)
			{
				j = n;

				if (g_GlobalParameters.bVerbosity)
				{
                    printf("\n");
				}

				if (of)
				{
                    fprintf(of, "\n");
				}
			}
			else
			{
				if (g_GlobalParameters.bVerbosity)
				{
                    printf("  ");
				}

				if (of)
				{
                    fprintf(of, "  ");
				}
			}
		}
	}

	// а теперь выведем оставшиеся ссылки на не найденные метки
	sz = g_labelRefs.getSize();

	if (sz)
	{
		wstr1 = L"\n\nСсылки на не определённые метки!\n\n";
		// if (g_GlobalParameters.bVerbosity)
		{
            printf("%s", wstring_to_utf8(wstr1).c_str());
		}

		if (of)
		{
//			size_t len = wstr1.length();
//			auto *pbuf = new uint8_t[len + 1];

//			if (pbuf)
//			{
//				UNICODEtoBK(wstr1, pbuf, len, false);
//				pbuf[len] = 0;
//				fprintf(of, "%s", pbuf);
//				delete[] pbuf;
//			}
            fprintf(of, "%s", wstring_to_utf8(wstr1).c_str());
		}

		// сперва найдём самую длинную метку
		for (size_t i = 0; i < sz; ++i)
		{
			std::wstring name = g_labelRefs.GetLabel(i)->getName();

			if (name.length() > maxlen)
			{
				maxlen = name.length();
			}
		}

		// теперь сформируем строку формата
		if (maxlen > 16)
		{
			maxlen = 16;
		}

        snprintf(buf, 32, "%%07o : %%-%ds", int(maxlen));
		n = 84 / (int(maxlen) + 12); // вот столько влазит в экран
		j = n;

		for (size_t i = 0; i < sz; ++i)
		{
			std::wstring name = g_labelRefs.GetLabel(i)->getName();
			int value = g_labelRefs.GetValue(i);
			value = CorrectAddress(value);

			if (name.length() > maxlen)
			{
				// если метка очень длинная, то её только одну в строке выведем.
				// if (g_GlobalParameters.bVerbosity)
				{
                    printf("%07o : %-s", value, wstring_to_utf8(name).c_str());
				}

				if (of)
				{
                    fprintf(of, "%07o : %-s", value, wstring_to_utf8(name).c_str());
				}

				j = n;
				// if (g_GlobalParameters.bVerbosity)
				{
                    printf("\n");
				}

				if (of)
				{
                    fprintf(of, "\n");
				}
			}
			else
			{
				// if (g_GlobalParameters.bVerbosity)
				{
                    printf(buf, value, wstring_to_utf8(name).c_str());
				}

				if (of)
				{
                    fprintf(of, buf, value, wstring_to_utf8(name).c_str());
				}

				if (--j <= 0)
				{
					j = n;
					// if (g_GlobalParameters.bVerbosity)
					{
                        printf("\n");
					}

					if (of)
					{
                        fprintf(of, "\n");
					}
				}
				else
				{
					// if (g_GlobalParameters.bVerbosity)
					{
                        printf("  ");
					}

					if (of)
					{
                        fprintf(of, "  ");
					}
				}
			}
		}
	}

	// if (g_GlobalParameters.bVerbosity)
	{
        printf("\n");
	}

	if (of)
	{
        fprintf(of, "\n"); fclose(of);
	}
}


void Usage()
{
    printf("Кросс ассемблер Turbo8 для БК0010-БК0011М\n" \
            "(с) 2014-2020 gid\n\n" \
            "Использование:\n" \
            "BKTurbo8 -? (--help)\n" \
            "  Вывод этой справки.\n\n" \
            "Имеется два режима работы. Режим компиляции, с созданием объектных модулей\n" \
            "и исполняемого файла. И режим линковки объектных модулей.\n\n" \
            "1. Режим компиляции.\n" \
            "BKTurbo8 [-i<c>][-v][-r][-l[name]][-o[name]][-s<0addr>] <cmd> <file_1 *[ file_n]>\n" \
            "  -i<c> (--input <c>) - задать кодировку исходного файла.\n" \
            "      Возможные кодировки:\n" \
            "        a - автоопределение (по умолчанию)\n" \
            "        k - KOI8-R\n" \
            "        o - OEM CP866\n" \
            "        w - ANSI CP1251\n" \
            "        8 - UTF8\n" \
            "        u - UNICODE UTF16LE\n" \
            "  Если автоопределение определило кодировку некорректно, необходимо задать\n" \
            "верную кодировку данным ключом.\n" \
            "  -v (--verbose) - вывод большего количества информации на экран.\n" \
            "      На данный момент дополнительно выводится таблица меток программы.\n\n" \
            "  -r (--raw) - создавать просто бинарный массив, не использовать формат .bin.\n\n" \
            "  -l[name] (--listing [name]) - генерировать lst Файл.\n" \
            "      Если имя файла задано, то используется оно для генерации листинга, если\n" \
            "      нет - то берётся имя файла исходного текста.\n\n" \
            "  -o[name] (--object [name]) - генерировать объектный файл.\n" \
            "      Если имя файла задано, то используется оно для генерации листинга, если\n" \
            "      нет - то берётся имя файла исходного текста.\n\n" \
            "  -t[name] (--table [name]) - создавать особый объектный файл, в котором\n" \
            "      содержатся только глобальные метки. (См. документацию)\n\n" \
            "  -s<0addr> (--address <0addr>) - задать начальный адрес компиляции.\n" \
            "      Адрес задаётся в восьмеричном виде.\n\n" \
            "  <cmd> - команда компиляции:\n" \
            "      CO - полная компиляция. В результате при отсутствии ошибок создаётся\n" \
            "           бинарный исполняемый файл и опционально создаются объектные файлы,\n" \
            "           заданные соответствующими ключами.\n" \
            "      CL - компиляция в объектный файл для дальнейшей линковки с другими\n" \
            "           объектными файлами. В результате при отсутствии ошибок всегда\n" \
            "           создаётся объектный файл. Бинарный файл не создаётся.\n" \
            "      Файл листинга создаётся в любом случае. При наличии ошибок код ошибки\n" \
            "      и его текстовое пояснение помещаются перед строкой листинга, вызвавшей\n" \
            "      ошибку.\n" \
            "      В конец файла листинга записывается таблица глобальных меток, а также\n" \
            "      список ссылок на неопределённые метки, если они есть.\n\n" \
            "  <file_1 *[ file_n]> - список исходных файлов, перечисленных через пробел.\n" \
            "      Допускаются маски файлов.\n\n" \
            "2. Режим линковки.\n" \
            "BKTurbo8 [-v][-r][-l[name]][-o[name]][-s<0addr>] LI <outfile> <file_1 *[ file_n]>\n" \
            "  Ключ -i не используется.\n\n" \
            "  Ключи -v, -r, -l, -o, -t и -s имеют тот же смысл, что и в режиме компиляции.\n\n" \
            "  Команда линковки - LI, за командой следует обязательное имя выходного файла\n" \
            "  <outfile>, маска файла не допускается. А затем список файлов объектных\n" \
            "  модулей.\n" \
            "  Листинг при этом не создаётся, потому что не из чего, но если задан ключ -l,\n" \
            "  в файл листинга сохраняется список меток, а так же список ссылок на \n" \
            "  неопределённые метки, если они есть.\n\n" \
            "  <file_1 *[ file_n]> - список файлов объектных модулей, перечисленных через\n" \
            "      пробел. Допускаются маски файлов.\n\n");
}

