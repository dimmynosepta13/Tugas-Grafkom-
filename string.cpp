#include "string.h"

// ----------------------------------------------------------------------------------------------------------------------------

CString::CString()
{
	String = NULL;
	Empty();
}

CString::CString(const char *DefaultString)
{
	String = NULL;
	Empty();
	Set(DefaultString);
}

CString::CString(const CString &DefaultString)
{
	String = NULL;
	Empty();
	Set(DefaultString.String);
}

CString::~CString()
{
	delete [] String;
}

CString::operator char* ()
{
	return String;
}

CString& CString::operator = (const char *NewString)
{
	if(String != NewString) Set(NewString);
	return *this;
}

CString& CString::operator = (const CString &NewString)
{
	if(this != &NewString) Set(NewString.String);
	return *this;
}

CString& CString::operator += (const char *NewString)
{
	Append(NewString);
	return *this;
}

CString& CString::operator += (const CString &NewString)
{
	Append(NewString.String);
	return *this;
}

CString operator + (const CString &String1, const char *String2)
{
	CString String = String1;
	String += String2;
	return String;
}

CString operator + (const char *String1, const CString &String2)
{
	CString String = String1;
	String += String2;
	return String;
}

CString operator + (const CString &String1, const CString &String2)
{
	CString String = String1;
	String += String2;
	return String;
}

void CString::Append(const char *Format, ...)
{
	va_list ArgList;

	va_start(ArgList, Format);

	int AppendixLength = vsnprintf(NULL, 0, Format, ArgList);
	char *Appendix = new char[AppendixLength + 1];
	vsprintf(Appendix, Format, ArgList);

	char *OldString = String;
	int OldStringLength = (int)strlen(String);

	int StringLength = OldStringLength + AppendixLength;
	String = new char[StringLength + 1];
	
	strcpy(String, OldString);
	strcat(String, Appendix);

	delete [] OldString;
	delete [] Appendix;
}

void CString::Set(const char *Format, ...)
{
	va_list ArgList;

	va_start(ArgList, Format);

	delete [] String;

	int StringLength = vsnprintf(NULL, 0, Format, ArgList);
	String = new char[StringLength + 1];
	vsprintf(String, Format, ArgList);
}

void CString::Empty()
{
	delete [] String;
	String = new char[1];
	String[0] = 0;
}
