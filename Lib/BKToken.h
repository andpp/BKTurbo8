#pragma once

class CBKToken
{
		std::wstring name;
		size_t hash;
		size_t calcHash();

	public:
		CBKToken(): name(L""), hash(0) {}

		CBKToken(const std::wstring &strName) : name(strName)
		{
			hash = calcHash();
		}

		virtual ~CBKToken() = default;

		void clear()
		{
			name.clear();
			hash = 0;
		}

		void setName(const std::wstring &strName)
		{
			name = strName;
			hash = calcHash();
		}

		const std::wstring &getName()
		{
			return name;
		}

		const size_t &getHash()
		{
			return hash;
		}

		size_t calcHash(std::wstring &str);

		CBKToken &operator=(const CBKToken &t)
		    = default;

		CBKToken &operator=(const CBKToken *t)
		{
			this->name = t->name;
			this->hash = t->hash;
			return *this;
		}
};

