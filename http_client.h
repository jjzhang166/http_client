#pragma once

class CHttpClient
{
public:
	class Writeable
	{
	public:
		virtual ~Writeable() {}
		virtual int write(const void* buffer, int length) = 0;
	};

	static bool Get(const wchar_t* url, Writeable* write);
};
