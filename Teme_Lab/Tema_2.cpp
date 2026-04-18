#include <fcntl.h>
#include <io.h>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <TlHelp32.h>
	
	//Tema 2: Sa se proiecteze o aplicatie (PowerShell, cmd, MSVC/C++ preferabil) care sa identifice toate serviciile care ruleaza la modul curent pe masina
	//Folosesc EnumServices pentru a obtine lista tuturor serviciilor, apoi EnumDependentServices pentru a verifica daca sunt in starea de Running
	
	//Optional: Sa se identifice dll-urile multiple


//FUnctie ca nu imi permite sa vad DLL-uri serviciilor System
void EnableDebugPrivilege() {
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		std::wcerr << L"Failed to open process token. Error: " << GetLastError() << std::endl;
		return;
	}
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
		std::wcerr << L"Failed to lookup privilege value. Error: " << GetLastError() << std::endl;
		CloseHandle(hToken);
		return;
	}
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
		std::wcerr << L"Failed to adjust token privileges. Error: " << GetLastError() << std::endl;
	}
	CloseHandle(hToken);
}


void extractDLLs(DWORD processID) {
	if (processID == 0) {
		std::wcerr << L"Invalid process ID." << std::endl;
		return;
	}

	HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID);
	if (hProcess == INVALID_HANDLE_VALUE) {
		std::wcerr << L"Failed to create snapshot for process ID " << processID << L". Error: " << GetLastError() << std::endl;
		return;
	}

	MODULEENTRY32W me;
	me.dwSize = sizeof(MODULEENTRY32W);


	if (Module32FirstW(hProcess, &me)) { //Obtinem primul modul (DLL) al procesului
		std::wcout << L"  DLL: " << me.szModule << L" - " << me.szExePath << std::endl;
		while (Module32NextW(hProcess, &me)) { //iteram prin toate modulele (DLL-urile) ale procesului
			std::wcout << L"  DLL: " << me.szModule << L" - " << me.szExePath << std::endl;
		}
	}
	else {
		std::wcerr << L"Failed to enumerate modules for process ID " << processID << L". Error: " << GetLastError() << std::endl;
	}
	CloseHandle(hProcess);

}
	
int main() {
	
	//FIX: pentru caracterele Unicode, sa se afiseze corect 
	_setmode(_fileno(stdout), _O_U16TEXT);

	EnableDebugPrivilege(); //Activam privilegiul de debug pentru a putea accesa informatii despre procesele serviciilor, inclusiv cele de sistem

	//Deschidem Service Control Manager pentru a putea enumera serviciile
	SC_HANDLE hSCManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE); //NULL pentru serverul local, NULL pentru grupul de servicii (toate), iar ultimul argument specifica permisiunea necesara (doar sa citim lista)
	if (!hSCManager) {
		std::wcerr << L"Failed to open Service Control Manager. Error: " << GetLastError() << std::endl;
		return 1;
	}


	DWORD dwBytesNeeded = 0;
	DWORD dwServiceCount = 0;
	DWORD dwResumeHandle = 0;

	//Prima apelare pentru a afla dimensiunea necesara a bufferului
	EnumServicesStatusExW(
		hSCManager,
		SC_ENUM_PROCESS_INFO, //Obtin informatii despre procesele asociate serviciilor
		SERVICE_WIN32, //Enumerez doar serviciile de tip Win32
		SERVICE_STATE_ALL, //Enumerez toate serviciile indiferent de starea lor
		NULL, 0, //Bufferul pentru rezultate va fi alocat dupa ce stim dimensiunea necesara
		&dwBytesNeeded,
		&dwServiceCount,
		&dwResumeHandle,
		NULL
	);

	//alocam bufferul necesar pentru a stoca informatiile despre servicii pe baza output-ului din prima apelare
	std::vector<BYTE> buffer(dwBytesNeeded); //Alocam bufferul necesar
	LPENUM_SERVICE_STATUS_PROCESSW services = reinterpret_cast<LPENUM_SERVICE_STATUS_PROCESSW>(buffer.data());

	if (!EnumServicesStatusExW(
		hSCManager,
		SC_ENUM_PROCESS_INFO,
		SERVICE_WIN32,
		SERVICE_STATE_ALL,
		buffer.data(),
		dwBytesNeeded,
		&dwBytesNeeded,
		&dwServiceCount,
		&dwResumeHandle,
		NULL
	)) {
		std::wcerr << L"Failed to enumerate services. Error: " << GetLastError() << std::endl;
		CloseServiceHandle(hSCManager);
		return 1;
	}
	else {
		std::wcout << L"Servicii care ruleaza in prezent:\n";
		std::wcout << L"-----------------------------------\n";	

		int runningServicesCount = 0;

		for (DWORD i = 0; i < dwServiceCount; i++) {
			//Verifica daca serviciul curent este in starea de Running
			if (services[i].ServiceStatusProcess.dwCurrentState == SERVICE_RUNNING) {
				DWORD processID = services[i].ServiceStatusProcess.dwProcessId;

				std::wcout << L"[RUNNING] " << services[i].lpServiceName << L" - " << services[i].lpDisplayName << L"(PID: "<< processID << std::endl;

				extractDLLs(processID); //Extragem si afisam DLL-urile asociate procesului serviciului
				std::wcout << L"-----------------------------------\n";
				runningServicesCount++;
			}
		}

	}

	//Cleaning
	CloseServiceHandle(hSCManager);

	return 0;
}