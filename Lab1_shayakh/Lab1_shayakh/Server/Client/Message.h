#pragma once


enum Messages
{
	M_INIT,
	M_EXIT,
	M_GETDATA,
	M_NODATA,
	M_TEXT,
	M_CONFIRM
};

enum Members
{
	M_BROKER = 0,
	M_ALL = 10,
	M_USER = 100
};

struct MsgHeader
{
	int m_To;
	int m_From;
	int m_Type;
	int m_Size;
};

class Message {
public:
	MsgHeader M_header;
	string M_data;
public:

	Message() {
		M_header = { 0 };
	}
	Message(int to, int from, int type = M_TEXT, const string& data = "") 
	{
		M_header.m_To = to;
		M_header.m_From = from;
		M_header.m_Type = type;
		M_header.m_Size = data.length();
		M_data = data;
	}

	void Send(CSocket& s) {
		s.Send(&M_header, sizeof(MsgHeader));
		if (M_header.m_Size) {
			s.Send(M_data.c_str(), M_header.m_Size + 1);
		}
	}

	MsgHeader Receive(CSocket& s) {
		s.Receive(&M_header, sizeof(MsgHeader));
		if (M_header.m_Size)
		{
			char* pBuff = new char[M_header.m_Size + 1];
			s.Receive(pBuff, M_header.m_Size + 1);
			M_data = pBuff;
			delete[] pBuff;
		}
		return M_header;
	}

	static void SendMessage(CSocket& s, int To, int From, int Type = M_TEXT, const string& Data = "")
	{
		Message msg(To, From, Type, Data);
		msg.Send(s);
	}
};
