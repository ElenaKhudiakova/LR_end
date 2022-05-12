//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "Unit1.h"
#include "Unit2.h"
#include "Unit3.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall ReadThread::ReadThread(bool CreateSuspended)
	: TThread(CreateSuspended)
{
	FreeOnTerminate = true;
	myEvent = new TEvent(NULL, true, false, "", false);
	ProcessThreadPtr = new ProcessThread(myEvent, true);
	ProcessThreadPtr->dataBuffer = dataBuffer;
}
//---------------------------------------------------------------------------
void __fastcall ReadThread::Execute()
{
	wchar_t MediaType[64];
	Form2->Label1->Visible = False;

	UnicodeString deviceName = Form2->Edit1->Text;
	wchar_t MediaType1 = deviceName.w_str()[0];
	swprintf(MediaType,L"\\\\.\\%c:",MediaType1);

	HANDLE fileHandle = CreateFile(
		MediaType,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	if(fileHandle == INVALID_HANDLE_VALUE)
	{
	   Form2->Label1->Visible = True;
	   Form2->Label1->Caption = "��������� ����� ��������������";
	   CloseHandle(fileHandle);
	   return;
	}
	LARGE_INTEGER sectorOffset;
	sectorOffset.QuadPart = 0;

	unsigned long currentPosition = SetFilePointer(
		  fileHandle,
		  sectorOffset.LowPart,
		  &sectorOffset.HighPart,
		  FILE_BEGIN
		);
	if(currentPosition != sectorOffset.LowPart)
	{
		Form2->Label1->Visible = True;
		Form2->Label1->Caption = "������������ ������� �������";
		CloseHandle(fileHandle);
		return;
	}
	ProcessThreadPtr->Start();
	DWORD bytesToRead = 1024*512;
	DWORD bytesRead;

	while(true)
	{
		//������������ �������� ������ ������
		bool readResult = ReadFile(
			  fileHandle,
			  dataBuffer,
			  bytesToRead,
			  &bytesRead,
			  NULL
			);
		if(readResult == false || bytesRead != bytesToRead)
		{
			Form2->Label1->Visible = True;
			Form2->Label1->Caption = "������ ������ ������";
			break;
		}
		Sleep(100);
		myEvent->SetEvent();
		Sleep(20);
		while (myEvent->WaitFor(0) == wrSignaled)
		{
			Sleep(10);
		}
		if(Terminated) break;
	}
	CloseHandle(fileHandle);
	ProcessThreadPtr->Terminate();
	delete ProcessThreadPtr;
}
//---------------------------------------------------------------------------
