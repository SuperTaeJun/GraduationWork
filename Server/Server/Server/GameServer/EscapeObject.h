#pragma once
#include "CorePch.h"

class EscapeObject
{
public:
    float x, y, z;
    float pitch, yaw, roll;
    int ob_id;
    int currentGameRoom;
public:
    EscapeObject() {}
    ~EscapeObject() {}

    void setPosition(float x_val, float y_val, float z_val) {
        x = x_val;
        y = y_val;
        z = z_val;
    }

    void removeOBJ() {
        //삭제되었을 때 배열을 밀지말고 돌려야함 삭제된 아이템 정보를 상대들에게 동기화
    }
};

