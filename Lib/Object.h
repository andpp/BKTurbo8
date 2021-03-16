#pragma once
#include "LabelManager.h"

enum class OBJTags : uint32_t
{
	OBJ_HDRTAG = 0x5501,
	OBJ_ID,             // тег блока идентификации
	OBJ_DATA,           // массив с данными
	OBJ_GLB,            // тег блока глобальных меток
	OBJ_LOC,            // тег блока локальных меток
	OBJ_REF,            // тег блока ссылок на метки
	OBJ_Label
};

constexpr auto OBJVERSION = 0x1001;

struct OBJBlockHeader
{
	OBJTags  nHdrTag;
	OBJTags  nBlockTag;
	uint32_t nBlkLen;
	uint32_t nEntryCount;
	uint32_t nCheckSum;
};

struct OBJIDBlock
{
	uint32_t nVersion;
	int16_t nStartAddress;
	int16_t nMode;
};

bool MakeObj(const std::string &strName, const std::string &strExt, bool bTbl = false);
bool LoadObj(const std::string &strName);
bool findBlock(OBJBlockHeader &hdr, OBJTags tag);
bool OBJ_SaveIDBlk();
bool OBJ_ReadIDBlk(OBJIDBlock &data);
bool OBJ_SaveLabelTable(CLabelTable *lbltbl, OBJTags nTag);
bool OBJ_LoadLabelTable(CLabelTable *lbltbl, OBJBlockHeader &hdr);
bool OBJ_SaveData();
bool OBJ_ReadData(uint32_t nLen);

/*
Объектный файл.
имеет теговую структуру.
состоит из блоков:
1. Блок глобальных меток
2. Блок локальных меток (если есть)
3. Блок ссылок на метки.
4. Блок идентификатор
*/


