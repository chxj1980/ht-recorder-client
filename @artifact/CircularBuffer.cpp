/**
 * @file CircularBuffer.cpp
 * @desc
 * implementation of the CCircularBuffer class.
 **/
#include "stdafx.h"
#include <windows.h>

#include "FrameBuffer2.h"
#include "CircularBuffer.h"

#define DEFAULT_CBUFFER_SIZE  10240

CCircularBuffer::CCircularBuffer()
{
	_Size    = 0;
	_In      = 0;
	_Out     = 0;
	_pBuffer = NULL;
	_bReadHeader = FALSE;

	//_HanNomManCBuffer = ::CreateMutex(NULL, FALSE, NULL);
	::InitializeCriticalSection( &_lock );
}

CCircularBuffer::CCircularBuffer(int length)
{
	if(length <= 0) length = DEFAULT_CBUFFER_SIZE;

	_Size    = length;
	_In      = 0;
	_Out     = 0;
	_pBuffer = NULL;

	_bReadHeader = FALSE;

	//_HanNomManCBuffer = ::CreateMutex(NULL, FALSE, NULL);
	::InitializeCriticalSection( &_lock );

	SetBufferSize(_Size);
}

CCircularBuffer::~CCircularBuffer()
{
	if(_pBuffer)
	{
		delete _pBuffer;
		_pBuffer = NULL;
	}

	//CloseHandle(_HanNomManCBuffer);
	::DeleteCriticalSection( &_lock );
}

// ���� ����Ʈ ���� �����Ѵ�
int CCircularBuffer::Read(BYTE *dest, int length)
{
	if(_Size <= 0 || length <= 0) return 0;

	int left, right, written;
	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	// �о� �� �� �ִ� �ִ� ũ�� ����
	written = (_In-_Out+_Size) % _Size; // UsedData()�� ����
	length  = (length <= written ? length : written);

	// �� �� �� �ι� ���� ũ�� ����
	right = (length <= _Size-_Out ? length : _Size-_Out);
	left  = length - right;

	// �б�
	if(right)
	{
		memcpy(dest, _pBuffer+_Out, right);
		_Out = (_Out + right) % _Size;
	}
	if(left)
	{
		memcpy(dest+right, _pBuffer+_Out, left);
		_Out += left;
	}

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();
	return length;
}


// �� ����Ʈ ���� �����Ѵ�
int CCircularBuffer::Write(BYTE *src, int length)
{
	if(_Size <= 0 || length <= 0) return 0;

	int left, right, free;
	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	// �� �� �ִ� �ִ� ũ�� ����
	free   = ((_Out - _In + _Size - 1) % _Size); // FreeSpace()�� ����
	length = (length <= free ? length : free);

	// �� �� �� �� �� �� ũ�� ����
	right = (length <= _Size-_In ? length : _Size-_In);
	left  = length - right;

	// ����
	if(right)
	{
		memcpy(_pBuffer+_In, src, right);
		_In = (_In + right) % _Size;
	}

	if(left)
	{
		memcpy(_pBuffer+_In, src+right, left);
		_In += left;
	}

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();
	return length;
}

// �б� ������ �����⸸ �ϰ� �״�� �д�
int CCircularBuffer::Peek(BYTE *dest, int length)
{
	if(_Size <= 0 || length <= 0) return 0;

	int left, right, written, tempout;
	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	// �о� �� �� �ִ� �ִ� ũ�� ����
	written = (_In-_Out+_Size) % _Size; // UsedData()�� ����
	length  = (length <= written ? length : written);

	// �� �� �� �ι� ���� ũ�� ����
	right = (length <= _Size-_Out ? length : _Size-_Out);
	left  = length - right;

	// �б�
	tempout = _Out;
	if(right)
	{
		CopyMemory(dest, _pBuffer+tempout, right);
		tempout = (tempout + right) % _Size;
	}
	if(left)
	{
		CopyMemory(dest+right, _pBuffer+tempout, left);
		tempout += left;
	}

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();

	return length;
}

// CircularBuffer --> CircularBuffer
int CCircularBuffer::MoveData(CCircularBuffer *src, int length)
{
	if(_Size <= 0 || length <= 0) return 0;

	int left, right, used, free;
	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	// �� �� �ִ� �ִ� ũ�� ����
	used   = src->UsedSpace();
	length = (length <= used ? length : used);
	free   = ((_Out - _In + _Size - 1) % _Size); // FreeSpace()�� ����
	length = (length <= free ? length : free);

	// �� �� �� �� �� �� ũ�� ����
	right = (length <= _Size-_In ? length : _Size-_In);
	left  = length - right;

	// ����
	if(right)
	{
		src->Read(_pBuffer+_In, right);
		_In = (_In + right) % _Size;
	}
	if(left)
	{
		src->Read(_pBuffer+_In, left);
		_In += left;
	}

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();

	return length;
}

// ����ִ� �������� ũ��
int CCircularBuffer::UsedSpace()
{
	if(_Size <= 0) return 0;
	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	int usedsize = (_In - _Out + _Size) % _Size;

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();
	return usedsize;
}

// pBuffer�� �� ������ �ִ� ũ�� = (������ ũ�� - 1)
// �� ��� ���� �� �� (_In == _Out) �� �� ������ ũ�⸸ŭ ä���
// �ٽ� (_In==_Out)�� �Ǿ� �� ���� �� ���� ������ �� ���� ����.
int CCircularBuffer::FreeSpace()
{
	if(_Size <= 0) return 0;
	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	int freesize = (_Out - _In + _Size - 1) % _Size;

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();

	return freesize;
}

// �� ������ ��ü ũ�⿡�� ���� ����
float CCircularBuffer::FreeSpaceRatio()
{
	if(_Size <= 0) return 0;
	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();
	float freeratio = (FreeSpace() / ((float)_Size));
	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();

	return freeratio;
}

// ũ�� ����
void CCircularBuffer::SetBufferSize(int buffersize)
{
	if(buffersize <= 0) buffersize = DEFAULT_CBUFFER_SIZE;

	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	_Size = buffersize;

	if(_pBuffer)
	{
		delete _pBuffer;
		_pBuffer = NULL;
	}

	_pBuffer = (BYTE *) malloc(sizeof(BYTE)*_Size);

	// reset
	_In  = 0;
	_Out = 0;
	_bReadHeader = FALSE;

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();
}

// data reset
void CCircularBuffer::Reset()
{
	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	_In  = 0;
	_Out = 0;

	_bReadHeader = FALSE;

	//ReleaseMutex(_HanNomManCBuffer);]
	Unlock();
}

// move _Out postion (����ִ� ������ �� �Ϻθ� ������)
int CCircularBuffer::Dump(int length)
{
	int used, dumped;

	if(_Size <= 0 || length <= 0) return 0;

	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	used   = UsedSpace();
	dumped = length <= used ? length : used;

	_Out = (_Out + dumped) % _Size;

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();

	return dumped;
}

// ���Ͽ� �� ������
int CCircularBuffer::Write2File(FILE *file)
{
	int left, right, written;

	if(_Size <= 0) return 0;

	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	// �о� �� �� �ִ� �ִ� ũ�� ����
	written = (_In-_Out+_Size) % _Size; // UsedData()�� ����

	// �� �� �� �ι� ���� ũ�� ����
	right = (written <= _Size-_Out ? written : _Size-_Out);
	left  = written - right;

	// �о ���Ͽ� ����
	if(right)
	{
		fwrite(_pBuffer+_Out, sizeof(BYTE), right, file);
		_Out = (_Out + right) % _Size;
	}
	if(left)
	{
		fwrite(_pBuffer+_Out, sizeof(BYTE), left, file);
		_Out += left;
	}

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();

	return written;
}

// ���۰� ������� �� ���Ͽ��� ���� �д´�
// [���� -> �Ϲݹ��� -> cbuffer] ---> [���� -> cbuffer]
int CCircularBuffer::FillFromFile(FILE *file, int length)
{
	if(_Size <= 0 || length <= 0) return 0;

	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	int read = fread(_pBuffer, sizeof(BYTE), length, file);
	_In  = read;
	_Out = 0;

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();

	return read;
}

int CCircularBuffer::MovePos(int add)
{
	if(add<0)
		return 0;

	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	if((_In - _Out + _Size) % _Size >= add)
	{
		_Out += add;
		_Out = _Out % _Size;
		//ReleaseMutex(_HanNomManCBuffer);
		Unlock();
		return 1;
	}
	else
	{
		//ReleaseMutex(_HanNomManCBuffer);
		Unlock();
		return 0;
	}
}

int CCircularBuffer::Search(const char *str)
{
	int i;

	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	for(i=0; (i+3) < (_In - _Out + _Size) % _Size; i++)
	{
		if(*(char*)(_pBuffer+(_Out+i)%_Size)==str[0])
		{
			if(*(char*)(_pBuffer+(_Out+i+1)%_Size)==str[1])
			{
				if(*(char*)(_pBuffer+(_Out+i+2)%_Size)==str[2])
				{
					if(*(char*)(_pBuffer+(_Out+i+3)%_Size)==str[3])
					{
						//ReleaseMutex(_HanNomManCBuffer);
						Unlock();
						return i;
					}
				}
			}
		}
	}

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();
	return i;
}

int CCircularBuffer::Search2(int *type)
{
	int i, val;

	//WaitForSingleObject(_HanNomManCBuffer, INFINITE);
	Lock();

	for(i=0; (i+3) < (_In - _Out + _Size) % _Size; i++)
	{
		val = *(unsigned int*)(_pBuffer+(_Out+i)%_Size);
		if(val==0x63643030)
		{
			//ReleaseMutex(_HanNomManCBuffer);
			Unlock();
			*type = 1; // VIDEO
			return i;
		}
		else if(val==0x62773130)
		{
			//ReleaseMutex(_HanNomManCBuffer);
			Unlock();
			*type = 0; // AUDIO
			return i;
		}
	}

	//ReleaseMutex(_HanNomManCBuffer);
	Unlock();
	return -1;
}