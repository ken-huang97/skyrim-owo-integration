#ifdef _WIN32
#pragma once
#include "../Controller/Network.h"
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

namespace OWOGame
{
	class UDPNetwork : public Network
	{
	private:
		SOCKET mySocket;
		ConnectionState state;
		owoVector<owoString> connectedAddressees;

	public:

		virtual ConnectionState GetState() override;
		virtual void SetState(ConnectionState state) override;
		virtual owoVector<owoString> GetConnectedAddressee() override;

		virtual owoString Listen(owoString& senderIp) override;
		virtual void Connect(owoString serverIp) override;
		virtual void Send(owoString message) override;
		virtual void SendTo(owoString message, owoString ip) override;
		virtual void Disconnect() override;
		virtual void Initialize() override;
	};
}

#endif