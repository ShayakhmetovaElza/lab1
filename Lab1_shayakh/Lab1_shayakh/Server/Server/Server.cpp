// Server.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "framework.h"
#include "Server.h"
#include "Message.h"
#include "Session.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Единственный объект приложения

CWinApp theApp;
int gMaxID = M_USER;
map<int, shared_ptr<Session>> gSessions;

void ProcessClient(SOCKET hSocket)
{
	CSocket Client;
	Client.Attach(hSocket);
	Message msg;

	switch (msg.Receive(Client))
	{
	case M_INIT:
	{
		auto pSession = make_shared<Session>(++gMaxID, msg.M_data);
		cout << "Клиент " << gMaxID << " подключен." << endl;
		gSessions[pSession->m_ID] = pSession;
		gSessions[pSession->m_ID]->time = clock();
		Message::SendMessage(Client, pSession->m_ID, M_BROKER, M_CONFIRM);
		break;
	}
	case M_EXIT:
	{
		cout << "Клиент " << msg.M_header.m_From << " отключен." << endl;
		gSessions.erase(msg.M_header.m_From);
		Message::SendMessage(Client, msg.M_header.m_From, M_BROKER, M_CONFIRM);
		break;
	}
	case M_GETDATA:
	{

		if (gSessions.find(msg.M_header.m_From) != gSessions.end())
		{
			gSessions[msg.M_header.m_From]->time = clock();
			gSessions[msg.M_header.m_From]->Send(Client);
		}
		break;
	}
	default:
	{
		if (gSessions.find(msg.M_header.m_From) != gSessions.end())
		{
			if (gSessions.find(msg.M_header.m_To) != gSessions.end())
			{
				gSessions[msg.M_header.m_To]->Add(msg);
			}
			else if (msg.M_header.m_To == M_ALL)
			{
				cout << msg.M_data<< endl;
				for (auto& [id, Session] : gSessions)
				{
					if (id != msg.M_header.m_From)
						Session->Add(msg);
				}
			}
			Message::SendMessage(Client, msg.M_header.m_From, M_BROKER, M_CONFIRM);
			gSessions[msg.M_header.m_From]->time = clock();
		}
		break;
	}
	}
}

void TimeOut()
{
	while (true)
	{
		for (int i = M_USER; i <= gMaxID; i++)
		{
			int time = clock();
			if (gSessions.find(i) != gSessions.end())
			{
				if (time - gSessions[i]->time > 150000)
				{
					cout << "Клиент " << i << " был отключен." << endl;
					gSessions.erase(i);
				}
			}
		}
		Sleep(3000);
	}
}

void Start() {
	AfxSocketInit();
	CSocket Server;
	Server.Create(12345);

	thread tt(TimeOut);
	tt.detach();

	while (true)
	{
		Server.Listen();
		CSocket s;
		Server.Accept(s);
		thread t(ProcessClient, s.Detach());
		t.detach();
	}
}




using namespace std;

int main()
{
    int nRetCode = 0;
	setlocale(LC_ALL, "rus");

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // инициализировать MFC, а также печать и сообщения об ошибках про сбое
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: вставьте сюда код для приложения.
            wprintf(L"Критическая ошибка: сбой при инициализации MFC\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: вставьте сюда код для приложения.
			Start();
        }
    }
    else
    {
        // TODO: измените код ошибки в соответствии с потребностями
        wprintf(L"Критическая ошибка: сбой GetModuleHandle\n");
        nRetCode = 1;
    }

    return nRetCode;
}
