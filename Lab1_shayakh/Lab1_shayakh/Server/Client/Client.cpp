// Client.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include "framework.h"
#include "Client.h"
#include "Message.h"
#include <string>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Единственный объект приложения

CWinApp theApp;

using namespace std;

int myId;
mutex hMutex;

void MyChannel(Message& m, MsgHeader& h_msg, CSocket& client, int Type, int ID_ToClient, int ID_FromClient, string textMsg)
{
	client.Create();
	client.Connect(_T("127.0.0.1"), 12345);
	if (textMsg != "")
		Message::SendMessage(client, ID_ToClient, ID_FromClient, Type, textMsg);
	else
		Message::SendMessage(client, ID_ToClient, ID_FromClient, Type);
	h_msg = m.Receive(client);
	client.Close();
}




void GetDataFromServer()
{
	while (true)
	{
		CSocket client;

		Message msg;
		MsgHeader h_msg;

		MyChannel(msg, h_msg, client, M_GETDATA,0, myId, "");

		if (h_msg.m_Type == M_TEXT)
		{
			hMutex.lock();
			cout <<"Сообщение от клиента: "<< msg.M_data << endl;
			hMutex.unlock();
		}
		Sleep(2000);
	}


}


void ConnectToServer(Message& m, MsgHeader& h_msg, CSocket& client) {

	AfxSocketInit();
	MyChannel(m, h_msg, client,M_INIT,0,0,"");

	if (h_msg.m_Type == M_CONFIRM) {
		myId = h_msg.m_To;
		hMutex.lock();
		cout << "Ваш ID = " << myId << endl;
		hMutex.unlock();
		thread t(GetDataFromServer);
		t.detach();

	}
	else {
		cout << "Ошибка. Клиент не подключен." << endl;
		return;
	}
}


void start()
{
	MsgHeader h_msg;
	CSocket client;
	Message msg;

	int result;
	cout << "Выберите: 1 - подключиться к серверу. \n0 - выйти." << endl;
	cin >> result;
	if (result == 1)
	{
		ConnectToServer(msg, h_msg, client);
	}
	

	while (true)
	{
		switch (result)
		{
			case 1:
			{
				string textMsg;
				int ID_ToClient;

				int s;
				cout <<  "Отправить сообщение: 0 - всем или 1 - одному клиенту ?\n2 - чтобы выйти из приложения. "<<endl;
				cin >> s;
			
				bool flag = false;
				if (s == 1)
				{
					cout << "Введите ID клиента:" << endl;
					cin >> ID_ToClient;
					flag = true;
				}
				if (s == 0)
				{
					ID_ToClient = M_ALL;
					flag = true;
				}
				if (s == 2)
				{
					MyChannel(msg, h_msg, client, M_EXIT, 0, myId, "");
					return;
				}

				if (flag)
				{
					cout << "Введите сообщение: " << endl;
					cin.ignore();
					getline(cin, textMsg);
					MyChannel(msg, h_msg, client, M_TEXT, ID_ToClient, myId, textMsg);

					hMutex.lock();
					if (h_msg.m_Type == M_CONFIRM) cout << "Сообщение отправлено!" << endl;
					else cout << "Сообщение не было отправлено!" << endl;
					hMutex.unlock();
				}
				else
				{
					cout << "Не верно введены данные!\nОтправить сообщение: 0 - всем или 1 - одному клиенту ?\n 2 - чтобы выйти из приложения." << endl;
					cin >> s;
				}
				break;
			}
			case 0:
			{
				MyChannel(msg, h_msg, client, M_EXIT, 0, myId, "");

				hMutex.lock();
				if (h_msg.m_Type == M_CONFIRM) cout << "Вы вышли!" << endl;
				else cout << "Ошибка!" << endl;
				hMutex.unlock();

				return;
			}
			default:
			{
				cout << "Не верно введены данные! \nВыберите: 1 - подключиться к серверу. \n0 - выйти." << endl;
				cin >> result;
			}
		}
	}

}


int main()
{
    int nRetCode = 0;

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
			setlocale(LC_ALL, "Russian");
			start();
            // TODO: вставьте сюда код для приложения.
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
