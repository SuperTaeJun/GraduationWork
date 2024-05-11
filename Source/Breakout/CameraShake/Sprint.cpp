// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraShake/Sprint.h"

USprint::USprint()
{
	OscillationDuration = 0.25;

	RotOscillation.Pitch.Amplitude = 0.8;
	RotOscillation.Pitch.Frequency = 18.f;
	RotOscillation.Pitch.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	RotOscillation.Yaw.Amplitude = 0.6;
	RotOscillation.Yaw.Frequency = 9.f;
	RotOscillation.Yaw.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	RotOscillation.Roll.Amplitude = 0.4;
	RotOscillation.Roll.Frequency = 7.f;

	bSingleInstance = true;

}
