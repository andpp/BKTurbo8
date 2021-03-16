#include "pch.h"
#include "Listing.h"
#include "Globals.h"
#include "Reader.h"

std::vector <ListingLine> g_Listing;
//#pragma warning(disable:4996)

int offsetpc = g_GlobalParameters.nStartAddress;

// начальные приготовления
void Lst_PrepareLine(int cp)
{
	ListingLine ll;
	ll.nLineNum = g_pReader->getLineNum();
    ll.line = trim(g_pReader->getCurString(), L'\n');
	ListingCmdType lct;
	lct.nPC = cp;
	lct.LT = ListType::LT_BLANKLINE;
	lct.nEndAdr = 0;
	ll.vCMD.push_back(lct);
	g_Listing.push_back(ll);
}

// добавляем инфу о текущей инструкции
void Lst_AddPrepareLine(int cp, ListType lt)
{
	auto pll = g_Listing.end() - 1;
	auto plct = pll->vCMD.end() - 1;

	if (plct->LT == ListType::LT_BLANKLINE)   // если последняя запись - пустая строка, то она и первая в общем-то
	{
		plct->nPC = cp;
		plct->LT = lt;  // то меняем её
	}
	else if (plct->LT == ListType::LT_LABEL)  // если последняя запись - метка, то тоже заменяем её, если эта запись остаётся,
	{
		// то это была единственная метка в строке.
		plct->nPC = cp;
		plct->LT = lt;  // то меняем её
	}
	else if (lt != ListType::LT_COMMENTARY) // если нет - то добавим новую, если это не комментарий.
	{
		ListingCmdType lct;
		lct.nPC = cp;
		lct.LT = lt;
		lct.nEndAdr = 0;
		pll->vCMD.push_back(lct);
	}
}

// добавляем инфу о текущей инструкции
void Lst_AddPrepareLine2(int len)
{
	auto pll = g_Listing.end() - 1;
	auto plct = pll->vCMD.end() - 1;
	plct->nEndAdr = len;
}

/*
Листинг состоит из 4х полей
1 - номер строки
2 - адрес команды
3 - бинарные данные
4 - текстовая строка
*/


// выводим номер строки
void Lst_OutLineNum(FILE *of, int nOut, int num)
{
	if (nOut == 0)
	{
        fprintf(of, "% 5d ", num);
	}
	else
	{
        fprintf(of, "      ");
	}
}
// выводим адрес
void Lst_OutAddress(FILE *of, int nOutLN, bool bOut, int num, int addr)
{
	Lst_OutLineNum(of, nOutLN, num);

	if (bOut)
	{
        fprintf(of, "%07o: ", addr - BASE_ADDRESS + offsetpc);
	}
	else
	{
        fprintf(of, "         ");
	}
}
// выводим бинарные данные пословно
// Вход: bgn_addr - начальный адрес данных
//      end_addr - конечный адрес данных
void Lst_OutWords(FILE *of, int nOutLN, bool bOutAddr, bool bOut, int num, int &bgn_addr, int end_addr)
{
	Lst_OutAddress(of, nOutLN, bOutAddr, num, bgn_addr);
	int npc = bgn_addr / 2;
	// самое простое. может быть от 1 до 3 слов данных
	int nlen = (bOut) ? (end_addr - bgn_addr) / 2 : 0;
	int nmax = 3 - nlen;

	for (int i = 0; i < nlen; ++i)
	{
        fprintf(of, "%07o ", g_Memory.w[npc++]);
	}

	for (int i = 0; i < nmax; ++i)
	{
        fprintf(of, "        ");
	}

    fprintf(of, " ");
	bgn_addr = npc * 2;
}
// выводим бинарные данные побайтово
// Вход: bgn_addr - начальный адрес данных
//      end_addr - конечный адрес данных
void Lst_OutBytes(FILE *of, int nOutLN, bool bOutAddr, bool bOut, int num, int &bgn_addr, int end_addr)
{
	Lst_OutAddress(of, nOutLN, bOutAddr, num, bgn_addr);
	int npc = bgn_addr;
	// самое простое. может быть от 1 до 5 байтов данных
	int nlen = (bOut) ? (end_addr - bgn_addr) : 0;
	int nmax = 5 - nlen;

	for (int i = 0; i < nlen; ++i)
	{
        fprintf(of, "%04o ", g_Memory.b[npc++]);
	}

	for (int i = 0; i < nmax; ++i)
	{
        fprintf(of, "     ");
	}

	bgn_addr = npc;
}

void Lst_OutLineString(FILE *of, int nCnt, std::wstring *str = nullptr)
{
	if (nCnt == 0)
	{
		if (str)
		{
//			size_t nLen = str->length();

//			if (nLen)
//			{
//				auto *pBuff = new uint8_t[nLen + 1];

//				if (pBuff)
//				{
//					UNICODEtoBK(*str, pBuff, nLen, false);
//					pBuff[nLen] = 0;
//					fprintf(of, "%s", pBuff);
//					delete[] pBuff;
//					return;
//				}
//			}
            fprintf(of, "%s", wstring_to_utf8(*str).c_str());
		}
	}

    fprintf(of, "\n");
}


void MakeListing(const std::string &strName, const std::string &strExt)
{
	offsetpc = g_GlobalParameters.nStartAddress;
    std::string pPath;
    std::string pName;
    std::string pExt;
    wsplitpath_s(strName, pPath, pName, pExt);
    // сделаем новое имя файла
    std::string outName = std::string(pPath) + std::string(pName) + strExt;
    FILE *of = fopen(outName.c_str(), "w");

	if (of == nullptr)
	{
        printf("Ошибка создания файла %s\n", outName.c_str());
	}

	for (auto &ll : g_Listing)
	{
		int nCount = 0;

		for (auto &lct : ll.vCMD)
		{
			// теперь, в зависимости от типа команды вывести содержимое
			switch (lct.LT)
			{
				case ListType::LT_BLANKLINE:
					Lst_OutLineNum(of, nCount++, ll.nLineNum);
                    fprintf(of, "\n");
					break;

				case ListType::LT_COMMENTARY:
				{
					int adr = 0;
					Lst_OutWords(of, nCount, false, false, ll.nLineNum, adr, 0); // выводим только номер строки
					Lst_OutLineString(of, nCount++, &ll.line);
				}
				break;

				case ListType::LT_ASSIGN:
				{
					Lst_OutAddress(of, nCount, false, ll.nLineNum, 0); // здесь не выводим адрес
                    fprintf(of, "%07o                  ", lct.nEndAdr); // словные данные выводим так
					Lst_OutLineString(of, nCount++, &ll.line);
				}
				break;

				case ListType::LT_LABEL:
				{
					int adr = lct.nPC;
					Lst_OutWords(of, nCount, true, false, ll.nLineNum, adr, lct.nEndAdr); // выводим адрес и данные
					Lst_OutLineString(of, nCount++, &ll.line);
				}
				break;

				case ListType::LT_INSTRUCTION:
				{
					int adr = lct.nPC;
					Lst_OutWords(of, nCount, true, true, ll.nLineNum, adr, lct.nEndAdr); // выводим адрес и данные
					Lst_OutLineString(of, nCount++, &ll.line);
				}
				break;

				case ListType::LT_PSC_ADDR:
				{
					int adr = lct.nPC;
					Lst_OutWords(of, nCount, true, true, ll.nLineNum, adr, lct.nPC + 2);
					Lst_OutLineString(of, nCount++, &ll.line);
					Lst_OutWords(of, nCount, true, true, ll.nLineNum, adr, lct.nEndAdr);
					Lst_OutLineString(of, nCount++, &ll.line);
				}
				break;

				case ListType::LT_PSC_LA:
				{
					offsetpc = GetStartAddress(); // поменяем адрес инструкций в листинге
					int adr = lct.nPC;
					Lst_OutWords(of, nCount, true, false, ll.nLineNum, adr, 0);
					Lst_OutLineString(of, nCount++, &ll.line);
				}
				break;

				case ListType::LT_PSC_BLKW:
				case ListType::LT_PSC_BLKB:
				case ListType::LT_PSC_END:
				{
					int adr = lct.nPC;
					Lst_OutWords(of, nCount, true, false, ll.nLineNum, adr, 0);
					Lst_OutLineString(of, nCount++, &ll.line);
				}
				break;

				case ListType::LT_PSC_PRINT:
				{
					int adr = lct.nPC;
					Lst_OutWords(of, nCount, true, true, ll.nLineNum, adr, lct.nPC + 4);
					Lst_OutLineString(of, nCount++, &ll.line);
					Lst_OutWords(of, nCount, true, true, ll.nLineNum, adr, lct.nPC + 6);
					Lst_OutLineString(of, nCount++, &ll.line);
					Lst_OutWords(of, nCount, true, true, ll.nLineNum, adr, lct.nEndAdr);
					Lst_OutLineString(of, nCount++, &ll.line);
				}
				break;

				case ListType::LT_PSC_FLT2:
				case ListType::LT_PSC_FLT4:
				case ListType::LT_PSC_WORD:
				case ListType::LT_PSC_RAD50:
				{
					// выводить по три слова в строку
					int adr = lct.nPC;
					int len = (lct.nEndAdr - adr) / 2;
					int cnt = len / 3;

					if (len % 3)
					{
						cnt++;
					}

					for (int i = 0; i < cnt; ++i)
					{
						int endadr = (adr + 6 < lct.nEndAdr) ? adr + 6 : lct.nEndAdr;
						Lst_OutWords(of, nCount, true, true, ll.nLineNum, adr, endadr); // выводим адрес и данные
						Lst_OutLineString(of, nCount++, &ll.line);
					}
				}
				break;

				case ListType::LT_PSC_BYTE:
				case ListType::LT_PSC_ASCII:
					// выводить по 5 байтов в строку
				{
					int adr = lct.nPC;
					int len = (lct.nEndAdr - adr);
					int cnt = len / 5;

					if (len % 5)
					{
						cnt++;
					}

					for (int i = 0; i < cnt; ++i)
					{
						int endadr = (adr + 5 < lct.nEndAdr) ? adr + 5 : lct.nEndAdr;
						Lst_OutBytes(of, nCount, true, true, ll.nLineNum, adr, endadr); // выводим адрес и данные
						Lst_OutLineString(of, nCount++, &ll.line);
					}
				}
				break;

				case ListType::LT_PSC_EVEN:
				{
					int adr = lct.nPC;
					bool bOut = ((lct.nEndAdr - lct.nPC) != 0);
					Lst_OutBytes(of, nCount, true, bOut, ll.nLineNum, adr, lct.nEndAdr);
					Lst_OutLineString(of, nCount++, &ll.line);
				}
				break;

				default:
				{
                    fprintf(of, "!!! Unworked case !!!\n");
				}
			}
		}
        // перед строкой выведем ошибки, если есть
                for (auto &err : ll.errors)
                {
                    size_t len = err.length();
                    fprintf(of, "%s", wstring_to_utf8(err).c_str());
        //			auto *pbuf = new uint8_t[len + 1];

        //			if (pbuf)
        //			{
        //				UNICODEtoBK(err, pbuf, len, false);
        //				pbuf[len] = 0;
        //				fprintf(of, "%s", pbuf);
        //				delete[] pbuf;
        //			}
                }
    }

	fclose(of);
}


