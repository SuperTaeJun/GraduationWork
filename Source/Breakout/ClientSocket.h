// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/PreWindowsApi.h"
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <fstream>
#include <map>
#include <mutex>
#include <vector>
#include <iostream>
#include "../../Server/Server/ServerCore/protocol.h"
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"
#include "CoreMinimal.h"
#include <concurrent_queue.h>

class UBOGameInstance;
class ACharacterController;
using namespace std;

// 플레이어 클래스 
class CPlayer
{
public:
    CPlayer() { };
    ~CPlayer() {};

    // 세션 아이디
    int Id = -1;
    // 아이디 비번
    char    userId[20] = {};
    char    userPw[20] = {};
    // 위치
    float X;
    float Y = 0;
    float Z = 0;
    // 회전값
    float Yaw = 0;
    float Pitch = 0;
    float Roll = 0;
    // 속도
    float VeloX = 0;
    float VeloY = 0;
    float VeloZ = 0;
    float Max_Speed = 400;
    bool  IsAlive = true;
    FVector FMyLocation;
    FVector FMyDirection;
    PlayerType p_type;
    WeaponType w_type;
    friend ostream& operator<<(ostream& stream, CPlayer& info)
    {
        stream << info.Id << endl;
        stream << info.X << endl;
        stream << info.Y << endl;
        stream << info.Z << endl;
        stream << info.VeloX << endl;
        stream << info.VeloY << endl;
        stream << info.VeloZ << endl;
        stream << info.Yaw << endl;
        stream << info.Pitch << endl;
        stream << info.Roll << endl;
        return stream;
    }

    friend istream& operator>>(istream& stream, CPlayer& info)
    {
        stream >> info.Id;
        stream >> info.X;
        stream >> info.Y;
        stream >> info.Z;
        stream >> info.VeloX;
        stream >> info.VeloY;
        stream >> info.VeloZ;
        stream >> info.Yaw;
        stream >> info.Pitch;
        stream >> info.Roll;
        return stream;
    }
};

const int buffsize = 1000;

enum IO_type
{
    IO_RECV,
    IO_SEND,
    IO_ACCEPT,
};

class Overlap {
public:
    WSAOVERLAPPED   _wsa_over;
    IO_type         _op;
    WSABUF         _wsa_buf;
     char   _net_buf[buffsize];
    int            _target;
public:
    Overlap(IO_type _op, char num_bytes, void* mess) : _op(_op)
    {
        ZeroMemory(&_wsa_over, sizeof(_wsa_over));
        _wsa_buf.buf = reinterpret_cast<char*>(_net_buf);
        _wsa_buf.len = num_bytes;
        memcpy(_net_buf, mess, num_bytes);
    }

    Overlap(IO_type _op) : _op(_op) {}

    Overlap()
    {
        _op = IO_RECV;
    }

    ~Overlap()
    {
    }
};

class CPlayerInfo
{
public:
    CPlayerInfo() {};
    ~CPlayerInfo() {};

    map<int, CPlayer> players;

    friend ostream& operator<<(ostream& stream, CPlayerInfo& info)
    {
        stream << info.players.size() << endl;
        for (auto& kvp : info.players)
        {
            stream << kvp.first << endl;
            stream << kvp.second << endl;
        }

        return stream;
    }

    friend istream& operator>>(istream& stream, CPlayerInfo& info)
    {
        int nPlayers = 0;
        int SessionId = 0;
        CPlayer Player;
        info.players.clear();

        stream >> nPlayers;
        for (int i = 0; i < nPlayers; i++)
        {
            stream >> SessionId;
            stream >> Player;
            info.players[SessionId] = Player;
        }

        return stream;
    }
};

class BREAKOUT_API ClientSocket : public FRunnable
{
public:
    ClientSocket();
    virtual ~ClientSocket();
    bool InitSocket();
    bool Connect();
    void CloseSocket();
    bool PacketProcess(char* ptr);
    void Send_Login_Info(char* id, char* pw);
    void Send_Move_Packet(int sessionID, FVector Location, FRotator Rotation, FVector Velocity, float Max_speed);
    void Send_Character_Type(PlayerType type);
    void Send_Weapon_Type(WeaponType type, int id);
    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;
    virtual void Exit() override;
    char    _id[MAX_NAME_SIZE];
    char    _pw[MAX_NAME_SIZE];
    void RecvPacket();
    void SendPacket(void* packet);
    bool StartListen();
    //void StopListen();
    bool Send(const int SendSize, void* SendData);

    static ClientSocket* GetSingleton() {
        static ClientSocket ins;
        return &ins;
    }
    void SetGameInstance(UBOGameInstance* inst) { gameinst = inst; }
    void SetPlayerController(ACharacterController* CharacterController);
    HANDLE Iocp;
    Overlap _recv_over;

    SOCKET ServerSocket;
    unsigned char recvBuffer[1000];
    FRunnableThread* Thread;
    FThreadSafeCounter StopTaskCounter;
    int _prev_size = 0;
    int local_id = -1;
    bool login_cond = false;
    Concurrency::concurrent_queue<char> buffer;
private:
    ACharacterController* MyCharacterController;
    CPlayerInfo PlayerInfo;
    UBOGameInstance* gameinst;
   
};

