#include "pch.h"
#include "LabelTable.h"


CLabelTable::CLabelTable()
    = default;


CLabelTable::~CLabelTable()
{
	Clear();
}

void CLabelTable::Clear()
{
	LabelTable.clear();
}

// выход: true - метка добавлена.
//      false - метка не добавлена, потому что такая уже есть
bool CLabelTable::AddLabel(CBKToken *token, int value, uint32_t lt)
{
	if (SearchLabel(token) == -1) // если такая метка не найдена
	{
		// добавляем
		Label lbl(lt, value, token);
		LabelTable.push_back(lbl);
		return true;
	}

	return false; // если метка уже есть - сообщим об этом
}

/*
Выдача метки по номеру
*/
CBKToken *CLabelTable::GetLabel(size_t n)
{
	if (n < getSize()) // если номер в допустимых пределах
	{
		return &LabelTable.at(n).label; // выдаём
	}

	return nullptr; // иначе null
}

/*
Удаление метки по номеру
*/
void CLabelTable::DeleteLabel(size_t n)
{
	if (n < getSize()) // если номер в допустимых пределах
	{
		LabelTable.erase(LabelTable.begin() + n); // стираем
	}
}

/*
Удаление метки по имени
*/
void CLabelTable::DeleteLabel(CBKToken *token)
{
	int n = SearchLabel(token); // ищем метку

	if (n >= 0) // если нашли
	{
		DeleteLabel(n); // удаляем по номеру
	}
}

/*
поиск метки в таблице определений, если не найдено, возвращает -1
иначе - номер позиции.
*/
int CLabelTable::SearchLabel(CBKToken *token)
{
	size_t sz = getSize();

	for (size_t i = 0; i < sz; ++i)
	{
		if (LabelTable.at(i).label.getHash() == token->getHash())
		{
#ifdef _DEBUG

			if (LabelTable.at(i).label.getName() != token->getName())
			{
                printf(L"HASH Error: %s:%#zX ~~ %s:%#zX\n", LabelTable.at(i).label.getName().c_str(), LabelTable.at(i).label.getHash(), token->getName().c_str(), token->getHash());
				assert(false);
			}

#endif
			return static_cast<int>(i);
		}
	}

	return -1; // ничего не найдено
}

/*
Выдача значения метки по имени
*/
int CLabelTable::GetValue(CBKToken *token)
{
	int n = SearchLabel(token); // ищем метку

	if (n >= 0) // если нашли
	{
		return LabelTable.at(n).label_value; // возвращаем её значение
	}

	return -1; // если не нашли
}

/*
Выдача значения метки по номеру
*/
int CLabelTable::GetValue(size_t n)
{
	if (n < getSize()) // если номер в допустимых пределах
	{
		return LabelTable.at(n).label_value; // возвращаем её значение
	}

	return -1; // если не нашли
}

/*
Выдача типа метки по имени
*/
uint32_t CLabelTable::GetType(CBKToken *token)
{
	int n = SearchLabel(token);

	if (n >= 0)
	{
		return LabelTable.at(n).label_type;
	}

	return UNKNOWN_LABEL;
}

/*
Выдача типа метки по номеру
*/
uint32_t CLabelTable::GetType(size_t n)
{
	if (n < getSize())
	{
		return LabelTable.at(n).label_type;
	}

	return UNKNOWN_LABEL;
}



CLabelTableRefs::CLabelTableRefs()
{
}


CLabelTableRefs::~CLabelTableRefs()
{
	Clear();
}

// выход: true - метка добавлена.
// может быть много ссылок на одну и ту же метку.
bool CLabelTableRefs::AddLabel(CBKToken *token, int value, uint32_t lt)
{
	// добавляем
	Label lbl(lt, value, token);
	LabelTable.push_back(lbl);
	return true; // метка всегда добавляется
}
