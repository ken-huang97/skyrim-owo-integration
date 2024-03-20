#pragma once
#include "../Domain/OWOTypes.h"
#include <string>

namespace OWOGame
{
    enum class ConnectionState { Disconnected, Connecting, Connected };

    class Network
    {
    public:
        virtual ConnectionState GetState() = 0;
        virtual void SetState(ConnectionState state) = 0;
        virtual owoVector<owoString> GetConnectedAddressee() = 0;

        virtual owoString Listen(owoString& senderIp) = 0;
        virtual void Initialize() = 0;
        virtual void Connect(owoString serverIp) = 0;
        virtual void Send(owoString message) = 0;
        virtual void SendTo(owoString message, owoString ip) = 0;
        virtual void Disconnect() = 0;

        virtual ~Network() = default;
    };
}