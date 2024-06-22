#include "Windows.h"
#include <tlhelp32.h>

#include <print>
#include <thread>
#include <filesystem>
#include <functional>

#undef NDEBUG
#include <cassert>
#define assertm( expression, msg ) ( void )( ( !!( expression ) ) || ( _wassert( _CRT_WIDE( msg " [" #expression "]" ), _CRT_WIDE( __FILE__ ), ( unsigned )( __LINE__ ) ), 0 ) )

int main( void )
{
	wchar_t buffer[ MAX_PATH ]{};
	static_cast< void >( GetModuleFileName( nullptr, buffer, MAX_PATH ) );

	const auto path = ( std::filesystem::path{ buffer }.parent_path() / "Tsukiyomi_DLL.dll" ).wstring();
	assertm( path.size() > 17, "Unable to get DLL path" );

	DWORD pid = 0;

	std::this_thread::sleep_for( std::chrono::seconds( 5 ) );
	{
		PROCESSENTRY32 pe32{ .dwSize = sizeof( PROCESSENTRY32 ) };

		std::unique_ptr< std::remove_pointer_t< HANDLE >, decltype( &CloseHandle ) > snap{
			CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 ),
			&CloseHandle
		};

		do
		{
			if ( wcscmp( pe32.szExeFile, L"ffxiv_dx11.exe" ) == 0 )
			{
				pid = pe32.th32ProcessID;
				break;
			}
		} while ( Process32Next( snap.get(), &pe32 ) );
	}
	assertm( pid, "Unable to find PID" );

	std::unique_ptr< std::remove_pointer_t< HANDLE >, decltype( &CloseHandle ) > proc{
		OpenProcess( PROCESS_ALL_ACCESS, 0, pid ),
		&CloseHandle
	};
	assertm( proc.get() != INVALID_HANDLE_VALUE, "Failed to OpenProcess" );

	auto virtual_free = std::bind( VirtualFree, std::placeholders::_1, 0, MEM_RELEASE );
	std::unique_ptr< std::remove_pointer_t< LPVOID >, decltype( virtual_free ) > alloc{
		VirtualAllocEx( proc.get(), 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE ),
		virtual_free
	};
	assertm( alloc.get(), "Failed to VirtualAllocEx" );

	assertm( WriteProcessMemory( proc.get(), alloc.get(), path.c_str(), sizeof( wchar_t ) * path.size() + 1, 0 ), "Failed to WriteProcessMemory" );

	std::unique_ptr< std::remove_pointer_t< HANDLE >, decltype( &CloseHandle ) > thread{
		CreateRemoteThread( proc.get(), 0, 0, reinterpret_cast< LPTHREAD_START_ROUTINE >( LoadLibraryW ), alloc.get(), 0, 0 ),
		&CloseHandle
	};
	assertm( thread.get(), "Failed to LoadLibrary" );

	std::println( "Successfully injected!" );

	return 0;
}