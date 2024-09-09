#pragma once
#include "pch.h"


// Room 클래스 정의
class Room {
public:
    void AddClient(int clientId) {
        players.push_back(clientId);
    }

    void RemoveClient(int clientId) {
        players.erase(std::remove(players.begin(), players.end(), clientId), players.end());
    }

    bool IsFull() const {
        return players.size() >= ENTER_CLIENT;
    }

    void Clear() {
        players.clear();
    }

    int GetPlayerCount() const {
        return players.size();
    }

    const std::vector<int>& GetPlayers() const {
        return players;
    }

private:
    std::vector<int> players;
};