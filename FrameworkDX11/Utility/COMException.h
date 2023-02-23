#pragma once
#ifndef COMEXCEPTION_H
#define COMEXCEPTION_H

#include <string>
#include <comdef.h>
#include "StringHelper.h"

#define COM_ERROR_IF_FAILED( hr, msg ) if( FAILED( hr ) ) throw COMException( hr, msg, __FILE__, __FUNCTION__, __LINE__ )

/// <summary>
/// Output error information by logging this class as a type through ErrorLogger.h when catching exceptions.
/// Use the macro "COM_ERROR_IF_FAILED( hr, msg )" to check for errors on DirectX components that return an HRESULT.
/// </summary>
class COMException
{
public:
	COMException(
		HRESULT hr,
		const std::string& msg,
		const std::string& file,
		const std::string& function,
		int line )
	{
		_com_error error( hr );
		whatMsg = L"Message: " + StringHelper::StringToWide( msg ) + L"\n";
		whatMsg += error.ErrorMessage();
		whatMsg += L"\nFile: " + StringHelper::StringToWide( file );
		whatMsg += L"\nFunction: " + StringHelper::StringToWide( function );
		whatMsg += L"\nLine: " + StringHelper::StringToWide( std::to_string( line ) );
	}
	inline const wchar_t* what() const
	{
		return whatMsg.c_str();
	}
private:
	std::wstring whatMsg;
};

#endif