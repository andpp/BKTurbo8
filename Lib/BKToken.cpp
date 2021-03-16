#include "pch.h"
#include "BKToken.h"

size_t CBKToken::calcHash()
{
	return calcHash(name);
}

size_t CBKToken::calcHash(std::wstring &str)
{
	std::hash<std::wstring> hash_fn;
	size_t hash = hash_fn(str);
	return hash;
}
