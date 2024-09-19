#pragma once
#include "pch.h"


class Room {
public:
    Room() : ready_count(0), ingamecount(0) {}

    // 복사 생성자 구현
    Room(const Room& other)
        : players(other.players),
        ready_count(other.ready_count.load()),  // atomic 변수의 값을 load하여 복사
        ingamecount(other.ingamecount.load())   // atomic 변수의 값을 load하여 복사
    {}

    // 대입 연산자 구현
    Room& operator=(const Room& other) {
        if (this != &other) {
            players = other.players;
            ready_count.store(other.ready_count.load());  // atomic 변수 복사
            ingamecount.store(other.ingamecount.load());  // atomic 변수 복사
        }
        return *this;
    }

    void AddClient(int clientId) {
        players.push_back(clientId);
    }

    void RemoveClient(int clientId) {
        players.erase(std::remove(players.begin(), players.end(), clientId), players.end());
    }

    bool IsFull() const {
        cout << "사이즈 ? " << players.size() << endl;
        return players.size() >= ENTER_CLIENT;
    }

    void Clear() {
        players.clear();
        ready_count = 0;
        ingamecount = 0;
    }

    void IncrementReadyCount() {
        ++ready_count;
    }

    void DecrementReadyCount() {
        --ready_count;
    }

    void IncrementInGameCount() {
        ++ingamecount;
    }

    void DecrementInGameCount() {
        --ingamecount;
    }

    void ResetReadyCount() {
        ready_count = 0;
    }

    std::atomic<int>& GetReadyCount() {
        return ready_count;
    }

    std::atomic<int>& GetInGameCount() {
        return ingamecount;
    }

    int GetPlayerCount() const {
        return players.size();
    }

    const std::vector<int>& GetPlayers() const {
        return players;
    }

private:
    std::vector<int> players;
    std::atomic<int> ready_count;  // 준비 상태인 플레이어 수
    std::atomic<int> ingamecount;  // 게임 중인 플레이어 수
};
