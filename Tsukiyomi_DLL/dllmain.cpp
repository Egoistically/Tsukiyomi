#include "Windows.h"

#include <print>
#include <functional>

#include "MinHook.h"

#ifndef NDEBUG
#define PRINTD( x, ... ) std::println( x, __VA_ARGS__ )
#else
#define PRINTD( x )
#endif

// TESTED: Only ChangeDisplaySettingsExW is ever used (W10 22H2)
using ChangeDisplaySettingsExW_t = LONG ( * )( LPCWSTR, DEVMODEW *, HWND, DWORD, LPVOID );
ChangeDisplaySettingsExW_t og_ChangeDisplaySettingsExW = nullptr;
LONG _ChangeDisplaySettingsExW( LPCWSTR lpszDeviceName, DEVMODEW *dev, HWND hwnd, DWORD dwflags, LPVOID lParam )
{
	static LONG result = DISP_CHANGE_SUCCESSFUL;
	static std::once_flag flag;

	std::call_once( flag, [ & ]() {
		result = og_ChangeDisplaySettingsExW( lpszDeviceName, dev, hwnd, dwflags, lParam );
	} );

	if ( dev != nullptr )
	{
		PRINTD( "ChangeDisplaySettingsExW | dmFields:{:x} dmBitsPerPel:{} dmPelsWidth:{} dmPelsHeight:{} dmDisplayFlags:{}",
			dev->dmFields,
			dev->dmBitsPerPel,
			dev->dmPelsWidth,
			dev->dmPelsHeight,
			dev->dmDisplayFlags );
	}

	return result;
}

bool entry( void )
{
#ifndef NDEBUG
	if ( !AllocConsole() )
		return false;
	static_cast< void >( std::freopen( "CONOUT$", "w", stdout ) );
#endif

	if ( MH_Initialize() != MH_OK )
	{
		PRINTD( "Failed to init MH" );
		return false;
	}

	if ( MH_CreateHookApi(
			 L"user32.dll",
			 "ChangeDisplaySettingsExW",
			 _ChangeDisplaySettingsExW,
			 reinterpret_cast< void ** >( &og_ChangeDisplaySettingsExW ) )
		 != MH_OK )
	{
		PRINTD( "Failed to create hook" );
		return false;
	}

	if ( MH_EnableHook( MH_ALL_HOOKS ) != MH_OK )
	{
		PRINTD( "Failed to enable hook" );
		return false;
	}

	PRINTD( "Hooked successfully" );
	return true;
}

extern "C" __declspec( dllexport ) bool WINAPI DllMain(
	const HMODULE hModule,
	const DWORD fdwReason,
	const LPVOID lpReserved [[maybe_unused]] )
{
	if ( fdwReason != DLL_PROCESS_ATTACH )
		return false;

	return entry();
}
