#pragma once
#include "CorePch.h"

class EscapeObject
{
public:
	float x, y, z;
	int ob_id;

public:
	EscapeObject() {}
	~EscapeObject() {}
		
    void setPosition(float x_val, float y_val, float z_val) {
        x = x_val;
        y = y_val;
        z = z_val;
    }

    void setRandomPosition(std::mt19937& gen, std::uniform_real_distribution<float>& dis) {
        x = dis(gen);
        y = dis(gen);
        z = dis(gen);
    }

    void removeOBJ() {
        //삭제되었을 때 배열을 밀지말고 돌려야함 삭제된 아이템 정보를 상대들에게 동기화
    }
};

