// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraShake/Jog.h"

UJog::UJog()
{
	OscillationDuration = 0.25;

	RotOscillation.Pitch.Amplitude = 0.6;
	RotOscillation.Pitch.Frequency = 7.5f;
	RotOscillation.Pitch.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	RotOscillation.Yaw.Amplitude = 0.4;
	RotOscillation.Yaw.Frequency = 6.f;
	RotOscillation.Yaw.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	RotOscillation.Roll.Amplitude = 0.4;
	RotOscillation.Roll.Frequency = 3.f;

	bSingleInstance = true;
}
