#pragma once

enum class ListType : int
{
	LT_BLANKLINE,
	LT_COMMENTARY,  // комментарий
	LT_ASSIGN,      // присваивание
	LT_LABEL,       // метка
	LT_INSTRUCTION, // обычная ассемблерная инструкция
	LT_PSC_ADDR,    // псевдокоманда ADDR
	LT_PSC_LA,      // псевдокоманда LA
	LT_PSC_PRINT,   // псевдокоманда PRINT
	LT_PSC_BLKW,    // псевдокоманда BLKW
	LT_PSC_BLKB,    // псевдокоманда BKLB
	LT_PSC_WORD,    // псевдокоманда WORD
	LT_PSC_BYTE,    // псевдокоманда BYTE
	LT_PSC_END,     // псевдокоманда END
	LT_PSC_EVEN,    // псевдокоманда EVEN
	LT_PSC_RAD50,   // псевдокоманда RAD50
	LT_PSC_ASCII,   // псевдокоманда ASCII,ASCIIZ
	LT_PSC_FLT2,    // псевдокоманда FLT2
	LT_PSC_FLT4,    // псевдокоманда FLT4
};

struct ListingCmdType
{
	int nPC;        // адрес с которого начинаются бинарные данные
	int nEndAdr;    // адрес на котором кончаются бинарные данные
	ListType LT;    // тип команды в строке
};


struct ListingLine
{
	int nLineNum;           // номер строки
    std::wstring line;           // сама строка
    std::vector<std::wstring> errors; // список ошибок.
	std::vector <ListingCmdType> vCMD;

	ListingLine() : nLineNum(0)
	{}
};

extern std::vector <ListingLine> g_Listing;

void Lst_PrepareLine(int cp);
void Lst_AddPrepareLine(int cp, ListType lt);
void Lst_AddPrepareLine2(int len);
void MakeListing(const std::string &strName, const std::string &strExt);
/*
Генерация листинга делается в 2 этапа
1. при первом проходе собирается информация о строках, и их содержимом.

2. если ошибок нет и мы делаем окончательную компоновку, генерируется
листинг, данные берутся из массива скомпиленных данных и выводятся
в отформатированном виде в зависимости от типа команды в строке.

*/
