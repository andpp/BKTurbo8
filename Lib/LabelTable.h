#pragma once
#include "BKToken.h"

// определения меток.
#define LABEL_DEFINITE_MASK     0x0000000F  // Маска для выделения типа метки
#define UNKNOWN_LABEL           0x00000000  // значение для ошибочных ситуаций
#define DEFINITE_LABEL          0x00000001  // описание локальной или глобальной метки
#define CONSTANT_LABEL          0x00000002  // описание константы (присваивания)
// определения ссылок на метки
#define LABEL_REFERENCE_MASK    0x000000F0  // Маска для выделения типа метки
#define BRANCH_LABEL            0x00000010  // метка в ветвлении
#define RELATIVE_LABEL          0x00000020  // метка в операнде
#define OFFSET_LABEL            0x00000030  // смещение до адреса
#define CURRENTPC_LABEL         0x00000040  // спец метка, для обработки '.' как текущего PC
// флаги операций
#define HALFLABEL_FLAG          0x00000100  // флаг деления пополам
#define NEGATIVE_FLAG           0x00000200  // флаг минуса
#define INVERSE_FLAG            0x00000400  // флаг инвертирования

typedef struct LBL
{
	uint32_t    label_type;
	int         label_value;
	CBKToken    label;

	LBL() = default;
	LBL(uint32_t lt, int lv, CBKToken *pt)
		: label_type(lt)
		, label_value(lv)
		, label(*pt) {}
} Label;

// Класс для управления таблицей меток.
// необходимые функции - добавление, удаление, очистка таблицы, поиск и выдача результата.
class CLabelTable
{
	protected:
		std::vector <Label> LabelTable;

	public:
		CLabelTable();
		virtual ~CLabelTable();

		void               Clear();    // очистка массива меток
		inline size_t      getSize()
		{
			return LabelTable.size();
		}

		virtual bool        AddLabel(CBKToken *token, int value, uint32_t lt);
		virtual CBKToken   *GetLabel(size_t n);
		virtual void        DeleteLabel(CBKToken *token);
		virtual void        DeleteLabel(size_t n);
		virtual int         SearchLabel(CBKToken *token);
		virtual int         GetValue(CBKToken *token);
		virtual int         GetValue(size_t n);
		virtual uint32_t    GetType(CBKToken *token);
		virtual uint32_t    GetType(size_t n);
};

// класс для таблицы ссылок. главное отличие - в таблице может быть неограниченное количество
// ссылок на одну и ту же метку.
class CLabelTableRefs : public CLabelTable
{
	public:
		CLabelTableRefs();
		virtual ~CLabelTableRefs() override;
		virtual bool        AddLabel(CBKToken *token, int value, uint32_t lt) override;
};

