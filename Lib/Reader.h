#pragma once

// таблица соответствия верхней половины аскии кодов с 128 по 255, включая псевдографику
static const wchar_t koi8tbl[128] =   // {200..237} этих символов на бк нету.
{
	L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',
	L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',
	L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',
	L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',   L' ',
	// {240..277}
	0xb6,   0x2534, 0x2665, 0x2510, 0x2561, 0x251c, 0x2514, 0x2550,
	0x2564, 0x2660, 0x250c, 0x252c, 0x2568, 0x2193, 0x253c, 0x2551,
	0x2524, 0x2190, 0x256c, 0x2191, 0x2663, 0x2500, 0x256b, 0x2502,
	0x2666, 0x2518, 0x256a, 0x2565, 0x2567, 0x255e, 0x2192, 0x2593,
	// {300..337}
	L'ю',   L'а',   L'б',   L'ц',   L'д',   L'е',   L'ф',   L'г',
	L'х',   L'и',   L'й',   L'к',   L'л',   L'м',   L'н',   L'о',
	L'п',   L'я',   L'р',   L'с',   L'т',   L'у',   L'ж',   L'в',
	L'ь',   L'ы',   L'з',   L'ш',   L'э',   L'щ',   L'ч',   L'ъ',
	// {340..377}
	L'Ю',   L'А',   L'Б',   L'Ц',   L'Д',   L'Е',   L'Ф',   L'Г',
	L'Х',   L'И',   L'Й',   L'К',   L'Л',   L'М',   L'Н',   L'О',
	L'П',   L'Я',   L'Р',   L'С',   L'Т',   L'У',   L'Ж',   L'В',
	L'Ь',   L'Ы',   L'З',   L'Ш',   L'Э',   L'Щ',   L'Ч',   L'Ъ'
};


class CBKToken;

class CReader
{
	public:
		enum class FILE_CHARSET
		{
			FILEERROR,
			UNDEFINE,
			KOI8,
			CP866,
			CP1251,
			UTF8,
			UTF16LE
		};
		enum class CHAR_TYPE
		{
			CT_EOF,     // конец файла
			LN,         // перевод строки
			SPACES,     // пробельные символы
			LETTERS,    // буквы
			DIGITS,     // цифры
			OTHERS      // прочие символы
		};

	private:
        std::string m_strFileName;         // имя обрабатываемого файла
		wchar_t     m_chPrev;
		wchar_t     m_chCurrent;            // текущий символ
		wchar_t     m_chNext;               // следующий символ
		CHAR_TYPE   m_nCurrCharType;        // тип текущего символа
		FILE       *m_file;
		FILE       *m_dfile;
		FILE_CHARSET m_nCharset;
		int         m_nLineNum;             // номер текущей строки
		std::wstring m_strCur;              // собственно текущая строка. Нужна чтобы выводить её при выводе ошибки.
		size_t      m_nLen;

		/*
		Используется алгоритм автоматического определения кодировки текста (ALT, WIN, KOI)
		Описание алгоритма: http://ivr.webzone.ru/articles/defcod_2/
		(c) Иван Рощин, Москва, 2004.
		*/

		static uint8_t table_2s[128];
		/* =========================================================================
		Вспомогательная функция alt2num.
		Вход: a - код русской буквы в кодировке ALT.
		Выход: порядковый номер этой буквы (0-31).
		========================================================================= */
		int alt2num(int a)
		{
			if (a >= 0xE0)
			{
				a -= 0x30;
			}

			return (a & 31);
		}
		/* =========================================================================
		Вспомогательная функция koi2num.
		Вход: a - код русской буквы в кодировке KOI.
		Выход: порядковый номер этой буквы (0-31).
		========================================================================= */

		int koi2num(int a)
		{
			static uint8_t t[32] =
			{
				30, 0, 1, 22, 4, 5, 20, 3, 21, 8, 9, 10, 11, 12, 13, 14, 15, 31,
				16, 17, 18, 19, 6, 2, 28, 27, 7, 24, 29, 25, 23, 26
			};
			return (t[a & 31]);
		}

		/* =========================================================================
		Вспомогательная функция work_2s - обработка двухбуквенного сочетания.
		Вход:  с1 - порядковый номер первой буквы (0-31),
		c2 - порядковый номер второй буквы (0-31),
		check - надо ли проверять, встречалось ли сочетание раньше
		(1 - да, 0 - нет),
		buf - адрес массива с информацией о встреченных сочетаниях.
		Выход: 0 - указанное сочетание уже встречалось раньше,
		1 - сочетание не встречалось раньше и является допустимым,
		2 - сочетание не встречалось раньше и является недопустимым.
		========================================================================= */
		int work_2s(int c1, int c2, int check, uint8_t buf[128]);

		/* =========================================================================
		Вспомогательная функция def_code - определение кодировки текста.
		Вход:
		n - количество различных сочетаний русских букв (1-255), которого
		достаточно для определения кодировки.
		Выход: определённая кодировка
		========================================================================= */
		FILE_CHARSET    def_code(int n = 128);
		int             get_char();
		// прочитать очередной символ из файла.
		wchar_t         getChar();
        FILE_CHARSET    AnalyseCharset(const std::string &strFileName);

		void            getString();

	public:
        CReader(const std::string &strFileName, FILE_CHARSET nInputCharset = FILE_CHARSET::UNDEFINE);
		virtual ~CReader();

        const std::string &GetFileName()
		{
			return m_strFileName;
		}

		FILE_CHARSET GetFileCharset()
		{
			return m_nCharset;
		}

		wchar_t readChar(bool bSkipWS = false);

		/*
		*Посмотреть текущий символ, без передвижения указателя
		*/
		inline wchar_t getCurrChar()
		{
			return m_chCurrent;
		}

		/*
		* Посмотреть следующий символ, без передвижения указателя
		*/
		inline wchar_t getNextChar()
		{
			return m_chNext;
		}

		/*
		* Посмотреть предыдущий символ (на всякий случай).
		*/
		inline wchar_t getPrevChar()
		{
			return m_chPrev;
		}

		inline CHAR_TYPE getCurrCharType()
		{
			return m_nCurrCharType;
		}

		inline CHAR_TYPE getNextCharType()
		{
			return AnalyseChar(m_chNext);
		}

		bool isEOF();
		CHAR_TYPE AnalyseChar(wchar_t ch);
		bool SkipWhitespaces(wchar_t &ch);
		bool readToken(CBKToken *token, wchar_t &ch);

		inline const std::wstring &getCurString()
		{
			return m_strCur;
		}

		inline int getLineNum()
		{
			return m_nLineNum;
		}
};

void UNICODEtoBK(std::wstring &ustr, uint8_t *pBuff, size_t bufSize, bool bFillBuf);
uint8_t UNICODEtoBK_ch(wchar_t uchr);

/*
При выдаче символов, у нас m_chCurrent - текущий символ,
m_chNext - следующий за ним, а указатель в файле указывает на
следующий за следующим.
 */
