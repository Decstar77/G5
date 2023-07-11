#include <enet.h>

#include "atto_client.h"
#include "atto_containers_thread_safe.h"


namespace atto {
    static std::atomic_bool     isRunning = false;
    static std::atomic_bool     isConnected = false;
    static std::atomic_bool     shouldConnect = false;
    static std::atomic_bool     shouldDisconnect = false;

    static std::atomic<SmallString> statusText = {};

    static ENetAddress          address = {};
    static ENetPeer*            peer = nullptr;
    static ENetHost*            client = nullptr;
    
    static FixedQueueThreadSafe<NetworkMessage, 1024> * incommingMessages;
    static FixedQueueThreadSafe<NetworkMessage, 1024> * outgoingMessages;

    static void SetStatusStringInfo(const char* text) {
        //ATTOINFO(text);
        statusText = SmallString::FromLiteral(text);
    }

    static void SetStatusStringErr(const char* text) {
        //ATTOERROR(text);
        statusText = SmallString::FromLiteral(text);
    }
    
    static void NetworkLoop() {
        //sATTOINFO("Network thread starting");

        if (enet_initialize() != 0) {
            //ATTOFATAL("An error occurred while initializing ENet.");
            return;
        }

        isRunning = true;

        while (isRunning) {
            if (!isConnected && shouldConnect) {
                SetStatusStringInfo("Attempting to connect.");
                client = enet_host_create(NULL, 1, 2, 0, 0);
                if (client != nullptr) {
                    enet_address_set_host(&address, "127.0.0.1");
                    address.port = 27164;

                    peer = enet_host_connect(client, &address, 2, 0);
                    if (peer != nullptr) {

                        ENetEvent event = {};
                        if (enet_host_service(client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
                            SetStatusStringInfo("Connection succeeded.");
                            isConnected = true;
                        }
                        else {
                            SetStatusStringInfo("Not connection to the server could be made");
                            enet_peer_reset(peer);
                            enet_host_destroy(client);
                        }
                    }
                    else {
                        SetStatusStringErr("Failed to create enet peer!");
                        enet_host_destroy(client);
                    }
                }
                else {
                    SetStatusStringErr("Failed to create enet host!");
                }

                shouldConnect = false;
            }

            while (isConnected) {
                if (shouldDisconnect) {
                    enet_peer_disconnect(peer, 0);
                    bool disconnected = false;

                    ENetEvent event = {};
                    while (enet_host_service(client, &event, 3000) > 0) {
                        switch (event.type) {
                        case ENET_EVENT_TYPE_RECEIVE:
                            enet_packet_destroy(event.packet);
                            break;
                        case ENET_EVENT_TYPE_DISCONNECT:
                            SetStatusStringInfo("Disconnection succeeded.");
                            disconnected = true;
                            break;
                        }
                    }

                    // Drop connection, since disconnection didn't succeed
                    if (!disconnected) {
                        enet_peer_reset(peer);
                    }

                    enet_host_destroy(client);

                    isConnected = false;
                    shouldDisconnect = false;

                    break;
                }

                ENetEvent event = {};
                i32 res = enet_host_service(client, &event, 30);

                if (res > 0) {
                    if (event.type == ENET_EVENT_TYPE_NONE) {

                    }
                    else if (event.type == ENET_EVENT_TYPE_CONNECT) {
                    }
                    else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                        SetStatusStringInfo("Connection to server has been lost");
                        enet_peer_reset(peer);
                        enet_host_destroy(client);
                        isConnected = false;
                        break;
                    }
                    else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                        //SetStatusStringInfo("Received packet");

                        NetworkMessage msg = {};
                        //NetworkMessageBuffer msgBuf = {};
                        //msgBuf.CreateFromByte((u8*)event.packet->data, (u32)event.packet->dataLength);
                        //DecompileNetworkMessage(msg, msgBuf);
                        incommingMessages->Enqueue(msg);

                        enet_packet_destroy(event.packet);
                    }
                    else {

                    }
                }
                else if (res < 0) {
                    SetStatusStringErr("Encountered error while polling the network");
                    //enet_peer_reset(peer);
                    //enet_host_destroy(client);
                }

                while (outgoingMessages->IsEmpty() == false) {
#if 0 
                    NetworkMessage msg = outgoingMessages->Dequeue();
                    NetworkMessageBuffer msgBuf = {};
                    CompileNetworkMessage(msg, msgBuf);
                    ATTOINFO("Sending %d bytes", msgBuf.GetHeadLocation());
                    ENetPacket* packet = enet_packet_create(msgBuf.GetData(), msgBuf.GetHeadLocation(), msg.isUDP ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(peer, 0, packet);
#else
                    NetworkMessage msg = outgoingMessages->Dequeue();
                    ENetPacket* packet = enet_packet_create(&msg, sizeof(msg), msg.isUDP ? ENET_PACKET_FLAG_UNSEQUENCED : ENET_PACKET_FLAG_RELIABLE);
                    enet_peer_send(peer, 0, packet);
#endif
      
                }

                //ATTOINFO("Enet ping %d", peer->roundTripTime);
            }

            Sleep(200);
        }

        enet_deinitialize();

        //ATTOINFO("Network thread finished");
    }

    void CreateNetworkState() {
        incommingMessages = new FixedQueueThreadSafe<NetworkMessage, 1024>();
        outgoingMessages = new FixedQueueThreadSafe<NetworkMessage, 1024>();
        std::thread(NetworkLoop).detach();
    }

    void DestroyNetworkState() {
        NetworkDisconnect();
        isRunning = false;
    }

    void NetworkConnect() {
        if (!isConnected) {
            shouldConnect = true;
        }
    }

    bool NetworkIsConnected() {
        return isConnected.load();
    }

    void NetworkDisconnect() {
        if (isConnected) {
            shouldDisconnect = true;
        }
    }

    SmallString NetworkStatusText() {
        SmallString copy = statusText.load();
        return copy;
    }

    void NetworkSend(const NetworkMessage &msg) {
        if (isConnected) {
            outgoingMessages->Enqueue(msg);
        }
    }

    bool NetworkRecieve(NetworkMessage& msg) {
        if (incommingMessages->IsEmpty()) {
            return false;
        }

        msg = incommingMessages->Dequeue();

        return true;
    }
}

