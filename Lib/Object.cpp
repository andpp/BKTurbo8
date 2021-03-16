#include "pch.h"
#include "Object.h"
#include "Globals.h"

#pragma warning(disable:4996)

FILE *objFile = nullptr;

bool MakeObj(const std::string &strName, const std::string &strExt, bool bTbl)
{
    std::string pPath;
    std::string pName;
    std::string pExt;
    wsplitpath_s(strName, pPath, pName, pExt);
    // сделаем новое имя файла
    std::string outName = std::string(pPath) + std::string(pName) + strExt;
    objFile = fopen(outName.c_str(), "wb");

	if (objFile == nullptr)
	{
        printf("Ошибка создания файла %s\n", outName.c_str());
		return false;
	}

	OBJ_SaveIDBlk();

	if (!bTbl) // когда создаём таблицу меток, сохраняем только глобальные метки
	{
		OBJ_SaveData();
	}

	OBJ_SaveLabelTable(&g_labelGlobalDefs, OBJTags::OBJ_GLB);

	if (!bTbl)
	{
		OBJ_SaveLabelTable(&g_labelLocalDefs, OBJTags::OBJ_LOC);
		OBJ_SaveLabelTable(&g_labelRefs, OBJTags::OBJ_REF);
	}

	return true;
}

OBJIDBlock g_loadID;
bool LoadObj(const std::string &strName)
{
    objFile = fopen(strName.c_str(), "rb");

	if (objFile)
	{
		if (OBJ_ReadIDBlk(g_loadID))
		{
			if (g_loadID.nVersion == OBJVERSION)
			{
				if (g_GlobalParameters.bLinkerFirst)
				{
					g_GlobalParameters.nStartAddress = int(g_loadID.nStartAddress) & 0xffff; // стартовый адрес берём только у первого модуля.
					g_GlobalParameters.bLinkerFirst = false;
				}

				bool bRes = true;
				OBJBlockHeader hdr;

				if (findBlock(hdr, OBJTags::OBJ_GLB))
				{
					bRes = bRes && OBJ_LoadLabelTable(&g_labelGlobalDefs, hdr);
				}

				if (findBlock(hdr, OBJTags::OBJ_LOC))
				{
					bRes = bRes && OBJ_LoadLabelTable(&g_labelLocalDefs, hdr);
				}

				if (findBlock(hdr, OBJTags::OBJ_REF))
				{
					bRes = bRes && OBJ_LoadLabelTable(&g_labelRefs, hdr);
				}

				// данные подчитывать после меток, т.к. g_GlobalParameters.nProgramLength тут корректируется
				if (findBlock(hdr, OBJTags::OBJ_DATA))
				{
					bRes = bRes && OBJ_ReadData(hdr.nBlkLen);
				}

				return bRes;
			}
		}

		fclose(objFile);
	}

	return false;
}



bool OBJ_SaveIDBlk()
{
	OBJBlockHeader hdr;
//	long fp = ftell(objFile);
	hdr.nHdrTag = OBJTags::OBJ_HDRTAG;
	hdr.nBlockTag = OBJTags::OBJ_ID;
	hdr.nBlkLen = sizeof(OBJIDBlock);
	hdr.nEntryCount = 1;
	hdr.nCheckSum = 0;
	OBJIDBlock data;
	data.nVersion = OBJVERSION;
	data.nStartAddress = static_cast<int16_t>(g_GlobalParameters.nStartAddress); // адрес запуска
	data.nMode = static_cast<int16_t>(g_GlobalParameters.nModeLinkage); // режим компоновки
	fwrite(&hdr, 1, sizeof(hdr), objFile);
	fwrite(&data, 1, sizeof(data), objFile);
	return true;
}

bool findBlock(OBJBlockHeader &hdr, OBJTags tag)
{
	fseek(objFile, 0, SEEK_SET);

	while (true)
	{
		fread(&hdr, 1, sizeof(hdr), objFile);

		if (feof(objFile))
		{
			break;
		}

		if (hdr.nHdrTag == OBJTags::OBJ_HDRTAG)
		{
			if (hdr.nBlockTag == tag)
			{
				return true;
			}

			if (fseek(objFile, hdr.nBlkLen, SEEK_CUR))
			{
				break;
			}
		}
	}

	return false;
}

bool OBJ_ReadIDBlk(OBJIDBlock &data)
{
	OBJBlockHeader hdr;

	if (findBlock(hdr, OBJTags::OBJ_ID))
	{
		if (sizeof(data) == fread(&data, 1, sizeof(data), objFile))
		{
			return true;
		}
	}

	return false;
}


bool OBJ_SaveData()
{
	int dataLen = g_GlobalParameters.nProgramLength;
	OBJBlockHeader hdr;
	hdr.nHdrTag = OBJTags::OBJ_HDRTAG;
	hdr.nBlockTag = OBJTags::OBJ_DATA;
	hdr.nBlkLen = dataLen;
	hdr.nEntryCount = 1;
	hdr.nCheckSum = 0;
	fwrite(&hdr, 1, sizeof(hdr), objFile);

	if (dataLen)
	{
		fwrite(&g_Memory.b[BASE_ADDRESS], 1, dataLen, objFile);
	}

	return true;
}

bool OBJ_ReadData(uint32_t nLen)
{
	if (nLen)
	{
		if (BASE_ADDRESS + g_GlobalParameters.nProgramLength + nLen < HIGH_BOUND)
		{
			fread(&g_Memory.b[BASE_ADDRESS + g_GlobalParameters.nProgramLength], 1, nLen, objFile);
			g_GlobalParameters.nProgramLength += nLen;
			return true;
		}
		else
		{
            printf("Критическая ошибка:: Достигнут предел свободной памяти\n");
			return false;
		}
	}

	return true;
}

bool OBJ_SaveLabelTable(CLabelTable *lbltbl, OBJTags nTag)
{
	size_t sz = lbltbl->getSize();

	if (sz > 0)
	{
		OBJBlockHeader hdr;
		long fp = ftell(objFile);
		hdr.nHdrTag = OBJTags::OBJ_HDRTAG;
		hdr.nBlockTag = nTag;
		hdr.nBlkLen = 0;
		hdr.nEntryCount = static_cast<uint32_t>(sz);
		hdr.nCheckSum = 0;
		fwrite(&hdr, 1, sizeof(hdr), objFile);

		for (size_t i = 0; i < sz; ++i)
		{
            std::string name = wstring_to_utf8(lbltbl->GetLabel(i)->getName());
			auto len = static_cast<uint32_t>(name.length());
			uint32_t value = lbltbl->GetValue(i);
			uint32_t type = lbltbl->GetType(i);
			OBJTags tag = OBJTags::OBJ_Label;
			fwrite(&tag, 1, sizeof(uint32_t), objFile);
			fwrite(&len, 1, sizeof(uint32_t), objFile);
//			fwrite(name.c_str(), 1, len * sizeof(wchar_t), objFile);
            fwrite(name.c_str(), 1, len, objFile);
            fwrite(&value, 1, sizeof(uint32_t), objFile);
			fwrite(&type, 1, sizeof(uint32_t), objFile);
		}

		// как посчитать контрольную сумму пока неясно.
		long efp = ftell(objFile);
		hdr.nBlkLen = efp - fp - sizeof(hdr);
		fseek(objFile, fp, SEEK_SET);
		fwrite(&hdr, 1, sizeof(hdr), objFile); // скорректируем размер блока
		fseek(objFile, efp, SEEK_SET);
	}

	return true;
}

bool OBJ_LoadLabelTable(CLabelTable *lbltbl, OBJBlockHeader &hdr)
{
	// предполагается, что мы прочитали заголовок блока, и знаем нужную таблицу
	// и указатель стоит на начале массива
	uint32_t num = 0; // буфер, куда читаем данные
	uint32_t nLen = hdr.nEntryCount;

	for (uint32_t i = 0; i < nLen; ++i)
	{
		OBJTags tag;
		fread(&tag, 1, sizeof(uint32_t), objFile);

		if (tag != OBJTags::OBJ_Label)
		{
			return false;
		}

		uint32_t len;
		fread(&len, 1, sizeof(uint32_t), objFile);

		if (int(len) <= 0)
		{
			return false;
		}

//		auto name = new wchar_t[len + 1];
        auto name = new char[len + 1];

        if (name)
		{
//			fread(name, 1, len * sizeof(wchar_t), objFile);
            fread(name, 1, len, objFile);
            name[len] = 0;
//			std::wstring labelName = std::wstring(name);
            std::wstring labelName = utf8_to_wstring(std::string(name));
            delete [] name;

            fread(&num, 1, sizeof(uint32_t), objFile);
			int value = num;
			fread(&num, 1, sizeof(uint32_t), objFile);
			auto type = uint32_t(num);
			CBKToken token(labelName);

			// здесь надо корректировать адреса меток

			if ((type & LABEL_REFERENCE_MASK) == CURRENTPC_LABEL)
			{
				// для спец ссылок точка надо скорректировать PC
				// независимо от всего, правда они могут тут появиться только в режиме CL и только в ссылках на метки
				// но всё равно, на всякий случай тут проверку делать будем
				std::wstring strCPC = token.getName(); // имя метки используем как текущий PC.
				strCPC = strCPC.substr(1); // убираем первый символ
				int pc = std::stoi(strCPC, nullptr, 8);
				pc += g_GlobalParameters.nProgramLength - BASE_ADDRESS;
				wchar_t buf[32] = { 0 };
                swprintf(buf, 32, L"!%06o\0", pc);
				token.setName(std::wstring(buf));
			}

			if (hdr.nBlockTag == OBJTags::OBJ_REF) // в ссылках на метки корректируем вообще всё
			{
				// для ссылок на метки надо ко всем прибавить g_GlobalParameters.nProgramLength - BASE_ADDRESS
				value += g_GlobalParameters.nProgramLength - BASE_ADDRESS;
			}
			else
			{
				// в таблицах меток надо делать коррекцию в зависимости от режима компоновки
				if (g_loadID.nMode == -1) // какой режим компоновки?
				{
					// режим CL, тут надо корректировать всё

					// для глобальных меток типа DEFINITE_LABEL надо прибавлять g_GlobalParameters.nProgramLength - BASE_ADDRESS
					if ((type & LABEL_DEFINITE_MASK) == DEFINITE_LABEL)
					{
						value += g_GlobalParameters.nProgramLength - BASE_ADDRESS;
					}

					// константы надо оставлять нетронутыми
				}
				else
				{
					// режим CO, тут надо корректировать только ссылки на метки

					// если таблица меток - то их адреса надо по-особому корректировать
					if ((type & LABEL_DEFINITE_MASK) == DEFINITE_LABEL) // причём только определения
					{
						value += (g_loadID.nStartAddress - g_GlobalParameters.nStartAddress);
						value &= 0xffff; // не давать переполнения 16 разрядов.
					}

					// константы надо оставлять нетронутыми
                }
			}

			if (!lbltbl->AddLabel(&token, value, type))
			{
				// добавим немного интеллекту
				int n = lbltbl->SearchLabel(&token); // найдём уже существующую метку

				if (lbltbl->GetValue(n) != value)  // если значение её не совпадает со значением новой метки
				{
					// выведем предупреждение
                    printf("LINK:: Метка '%s' уже существует. Повторная метка не добавлена.\nИсполняемый модуль может быть с ошибкой.\n", wstring_to_utf8(labelName).c_str());
				}
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}
