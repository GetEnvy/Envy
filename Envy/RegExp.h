//
// RegExp.h
//
// Abstracted std::tr1:regex Regular Expressions

#pragma once


namespace RegExp
{

BOOL	Match(LPCTSTR szRegExp, LPCTSTR szContent);
size_t	Split(LPCTSTR szRegExp, LPCTSTR szContent, LPTSTR* pszResult);

};
