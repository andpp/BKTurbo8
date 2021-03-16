// BKbin2obj.cpp: определяет точку входа для консольного приложения.
//

#include "pch.h"
#include "getopt.h"
#include "BKbin2obj.h"
#include "Globals.h"
#include "LabelManager.h"
#include "Object.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <clocale>
//#pragma warning(disable:4996)

using namespace std;

std::string strType, strLabel, strFinalLabel, strInFileName, strOutFileName;
/*
опции:
bColor - чб / цветная картинка
bTransparency - прозрачность
*/
bool bColor = false, bTransparency = false, bEven = false;
int nEvenBound = 0;

int main(int argc, char *argv[])
{
//	_wsetlocale(LC_ALL, L"Russian");
    std::setlocale(LC_ALL, "");
    InitGlobalParameters(&g_GlobalParameters);
	static struct option long_options[] =
	{
        { "help",         ARG_NONE, nullptr, L'?' },
        { "source",       ARG_REQ,  nullptr, L's' },
        { "even",         ARG_OPT,  nullptr, L'e' },
        { "label",        ARG_REQ,  nullptr, L'l' },
        { "final",        ARG_REQ,  nullptr, L'f' },
        { "color",        ARG_NONE, nullptr, L'c' },
        { "transparency", ARG_NONE, nullptr, L't' },
        { nullptr,        ARG_NULL, nullptr, ARG_NULL }
	};
    static char optstring[] = "?s:e::l:f:ct";
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

			case L's':
				if (optarg)
				{
                    strType = std::string(optarg);
					strType = strType.substr(0, 3);
					strType[0] = toupper(strType[0]);
					strType[1] = toupper(strType[1]);
					strType[2] = toupper(strType[2]);
				}

				break;

			case L'e':
				bEven = true;

				if (optarg)
				{
                    int nTmp = std::stol(optarg, nullptr, 8) & 0xffff;

					if (0 <= nTmp && nTmp <= 0100000)
					{
						nEvenBound = nTmp;
					}
				}

				break;

			case L'l':
				bEven = true;

				if (optarg)
				{
                    strLabel = strToUpper(std::string(optarg));

//					if (!strLabel.empty())
//					{
//						for (wchar_t &n : strLabel)
//						{
//							n = toupper(n);
//						}
//					}
				}

				break;

			case L'f':
				bEven = true;

				if (optarg)
				{
                    strFinalLabel = strToUpper(std::string(optarg));

//					if (!strFinalLabel.empty())
//					{
//						for (wchar_t &n : strFinalLabel)
//						{
//							n = toupper(n);
//						}
//					}
				}

				break;

			case L'c':
				bColor = true;
				break;

			case L't':
				bTransparency = true;
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
        printf("Ошибка в командной строке: не задано имя входного файла.\n\n");
		Usage();
	}
	else
	{
        strInFileName = std::string(*argv);
		argc--;
		argv++;

		if (argc >= 1)
		{
            strOutFileName = std::string(*argv);
		}
		else
		{
			strOutFileName = strInFileName;
		}

		if (strLabel.empty())
		{
			// если имя метки не было задано
            std::string pPath;
            std::string pName;
            std::string pExt;
            wsplitpath_s(strInFileName, pPath, pName, pExt);
            // сделаем имя метки по умолчанию
            strLabel = std::string(pName);
		}

		g_GlobalParameters.nModeLinkage = -1;

        if (strType == "BIN")
		{
			ConvertBin();
		}
        else if (strType == "IMG")
		{
			ConvertImage();
		}
	}

	return 0;
}

void ConvertImage()
{
    CImage img;

    if (SUCCEEDED(img.Load(strInFileName.c_str())))
	{
        int w = img.GetWidth();
        int h = img.GetHeight();
		// посчитаем размер БКшного ресурса при таких размерах картинки.
		int size = 0;

		if (bColor)
		{
			// для цветной картинки.
			// 1 пиксел - 2 бита
			size = ((w + 3) / 4) * h * (bTransparency ? 2 : 1);
		}
		else
		{
			// для чёрно-белой картинки.
			// 1 пиксел - 1 бит
			size = ((w + 7) / 8) * h * (bTransparency ? 2 : 1);
		}

		if (size < 65536)
		{
			if (size >= 32768)
			{
                printf("Предупреждение! Размер ресурса больше половины доступной памяти БК.\n");
			}

			// конвертируем
			if (bColor)
			{
                convertBitmap2(&g_Memory.b[BASE_ADDRESS], &img, bTransparency);
			}
			else
			{
                convertBitmap1(&g_Memory.b[BASE_ADDRESS], &img, bTransparency);
			}

            CBKToken token(utf8_to_wstring(std::string(strLabel).c_str())); // добавляем метку
			g_labelGlobalDefs.AddLabel(&token, BASE_ADDRESS, DEFINITE_LABEL);

			if (bEven)
			{
				if (nEvenBound)
				{
					// выравнивание по произвольной границе степени 2
					size = size ? ((size - 1) | (nEvenBound - 1)) + 1 : 0;
				}
				else
				{
					// выравниваем по чётному адресу
					size++;
					size &= ~1;
				}
			}

			g_GlobalParameters.nProgramLength = size;
            MakeObj(strOutFileName, std::string(".obj"));
		}
		else
		{
            printf("Размер ресурса слишком велик.\n");
		}

        img.Destroy();
	}
	else
	{
        printf("Ошибка загрузки файла изображения %s\n", strInFileName.c_str());
	}
}


// преобразовываем обычный массив бинарных данных
void ConvertBin()
{
    FILE *inf = fopen(strInFileName.c_str(), "rb");

	if (inf)
	{
		// узнаем размер файла
		fseek(inf, 0, SEEK_END);
		long size = ftell(inf);
		fseek(inf, 0, SEEK_SET);

		if (size < 65536)
		{
			if (size >= 32768)
			{
                printf("Предупреждение! Размер ресурса больше половины доступной памяти БК.\n");
			}

			fread(&g_Memory.b[BASE_ADDRESS], 1, size, inf); // читаем файл
            CBKToken token(utf8_to_wstring(strLabel)); // добавляем метку
			g_labelGlobalDefs.AddLabel(&token, BASE_ADDRESS, DEFINITE_LABEL);

			if (bEven)
			{
				if (nEvenBound)
				{
					// выравнивание по произвольной границе степени 2
					size = size ? ((size - 1) | (nEvenBound - 1)) + 1 : 0;
				}
				else
				{
					// выравниваем по чётному адресу
					size++;
					size &= ~1;
				}
			}

			if (!strFinalLabel.empty())
			{
                CBKToken finalToken(utf8_to_wstring(strFinalLabel).c_str()); // добавляем метку
				g_labelGlobalDefs.AddLabel(&finalToken, BASE_ADDRESS + size, DEFINITE_LABEL);
			}

			g_GlobalParameters.nProgramLength = size;
            MakeObj(strOutFileName, std::string(".obj"));
		}
		else
		{
            printf("Размер ресурса слишком велик.\n");
		}

		fclose(inf);
	}
	else
	{
        printf("Ошибка открытия файла %s\n", strInFileName.c_str());
	}
}

// функции из pdp11asm vinxru
// это цветная картинка
void convertBitmap2(uint8_t *destp, CImage *img, bool t)
{
	int w = img->GetWidth();
	int h = img->GetHeight();
	int destbpl = (w + 3) / 4 * (t ? 2 : 1);

	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
            COLORREF color = img->GetPixel(x, y);
			// ваще какая-то непонятная шняга.
            bool tr = (color == 0xFF00FF);

//          if (сolor == 0)
//          {
//              сolor = 0;
//          }
//          else if (сolor == 0x0000FF)
//          {
//              сolor = 1;
//          }
//          else if (сolor == 0x00FF00)
//          {
//              сolor = 2;
//          }
//          else
//          {
//              сolor = 3;
//          }

			// теперь определим, какой цвет у пикселя
            if ((color & 0xff0000) && !(color & 0x00f8f8)) // если в синем канале что-то есть, а в остальных почти нету
			{
                color = 1; // то это синий цвет
			}
            else if ((color & 0x00ff00) && !(color & 0xf800f8)) // если в зелёном канале что-то есть, а в остальных почти нету
			{
                color = 2; // то это зелёный цвет
			}
            else if ((color & 0x0000ff) && !(color & 0xf8f800)) // если в красном канале что-то есть, а в остальных почти нету
			{
                color = 3; // то это красный цвет
			}
			else
			{
				// все остальные комбинации - чёрный цвет. нефиг тут.
                color = 0; // чёрный
			}

			if (t)
			{
				if (!tr)
				{
					if (w >= 16)
					{
                        destp[x / 4 + x / 4 / 2 * 2 + 2 + y * destbpl] |= color << ((x & 3) * 2);
						destp[x / 4 + x / 4 / 2 * 2 + y * destbpl] |= 3 << ((x & 3) * 2);
					}
					else
					{
                        destp[x / 4 * 2 + 1 + y * destbpl] |= color << ((x & 3) * 2);
						destp[x / 4 * 2 + y * destbpl] |= 3 << ((x & 3) * 2);
					}
				}
			}
			else
			{
                destp[x / 4 + y * destbpl] |= (color << ((x & 3) * 2));
			}
		}
	}
}

// -----------------------------------------------------------------------------
// это чёрно белая картинка
void convertBitmap1(uint8_t *destp, CImage *img, bool t)
{
	int w = img->GetWidth();
	int h = img->GetHeight();
	size_t destbpl = (w + 7) / 8 * (t ? 2 : 1);

	for (int y = 0; y < h; ++y)
	{
		for (int x = 0; x < w; ++x)
		{
			COLORREF c = img->GetPixel(x, y);
			bool tr = (c == 0xFF00FF);

			if (c == 0)
			{
				c = 0;
			}
			else
			{
				c = 1;
			}

			if (t)
			{
				if (!tr)
				{
					if (w >= 16)
					{
						if (c)
						{
							destp[x / 8 + x / 8 / 2 * 2 + 2 + y * destbpl] |= 1 << (x & 7);
						}

						destp[x / 8 + x / 8 / 2 * 2 + y * destbpl] |= 1 << (x & 7);
					}
					else
					{
						if (c)
						{
							destp[x / 8 * 2 + 1 + y * destbpl] |= 1 << (x & 7);
						}

						destp[x / 8 * 2 + y * destbpl] |= 1 << (x & 7);
					}
				}
			}
			else
			{
				if (c)
				{
					destp[x / 8 + y * destbpl] |= c << (x & 7);
				}
			}
		}
	}
}

void Usage()
{
    printf("Конвертер бинарных объектов в объектные модули кросс ассемблера Turbo8.\n" \
            "(с) 2014-2020 gid\n\n" \
            "Использование:\n" \
            "BKbin2obj -? (--help)\n" \
            "  Вывод этой справки.\n\n" \
            "BKbin2obj [-s<type>][-c][-t][-e[0bound]][-l<label_name>][-f<label_name>] <input_file_name> [output_file_name]\n" \
            "  -s<source> (--source <source>) - тип входного объекта.\n" \
            "    Возможные типы:\n" \
            "    bin - просто бинарный массив;\n" \
            "    img - картинка в формате BMP, GIF, JPEG, PNG и TIFF.\n\n" \
            "  -e[0bound] (--even [0bound]) - выравнивание массива данных по границе блока.\n" \
            "    Если параметр не задан - делается выравнивание по границе слова.\n" \
            "    Параметр - число в восьмеричном виде. Предполагается, что число - степень\n" \
            "    двойки. Если задать произвольное число, результат будет совсем не тем, что\n" \
            "    ожидался.\n\n" \
            "  -l<label_name> (--label <label_name>) - задать имя метки. Если имя метки не\n" \
            "    задано, оно формируется из имени входного файла.\n\n" \
            "  -f<label_name> (--final <label_name>) - задать имя финальной метки в конце.\n" \
            "    Если задан ключ -e, то метка ставится в конце выравнивания.\n\n" \
            "  Ключи, действующие только при выборе типа img:\n" \
            "  -c (--color) - обрабатывать картинку как цветное изображение, иначе - чёрно-\n" \
            "    белое.\n\n" \
            "  -t (--transparency) - использование прозрачности.\n" \
            "    Алгоритмы преобразования взяты из проекта pdp11asm vinxru, один к одному,\n" \
            "    я даже не разбирался как они работают.\n\n" \
            "  input_file_name - входной файл.\n" \
            "  output_file_name - необязательное имя выходного файла, если нужно задать\n" \
            "  объектному файлу имя, отличное от входного.\n");
}

/*
конвертор всяких бинарных объектов в объектные файлы, чтобы их прилинковывать к
ассемблерным объектникам.
*/
