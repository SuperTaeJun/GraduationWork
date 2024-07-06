// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraShake/Shoot.h"

UShoot::UShoot()
{
	OscillationDuration = 0.3f;

	RotOscillation.Pitch.Amplitude = 0.5f;
	RotOscillation.Pitch.Frequency = 10.f;
	RotOscillation.Pitch.InitialOffset = EInitialOscillatorOffset::EOO_OffsetZero;

	RotOscillation.Yaw.Amplitude = 0.5f;
	RotOscillation.Yaw.Frequency = 50.f;
	RotOscillation.Yaw.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

	RotOscillation.Roll.Amplitude = 2.f;
	RotOscillation.Roll.Frequency = 500.f;

	bSingleInstance = true;
}

UShoot::UShoot(int Type)
{
	switch (Type)
	{
		case 1:
			OscillationDuration = 0.3f;

			RotOscillation.Pitch.Amplitude = 0.5f;
			RotOscillation.Pitch.Frequency = 10.f;
			RotOscillation.Pitch.InitialOffset = EInitialOscillatorOffset::EOO_OffsetZero;

			RotOscillation.Yaw.Amplitude = 0.5f;
			RotOscillation.Yaw.Frequency = 50.f;
			RotOscillation.Yaw.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;
		
			RotOscillation.Roll.Amplitude = 2.f;
			RotOscillation.Roll.Frequency = 500.f;

			bSingleInstance = true;
			break;
		case 2:
			OscillationDuration = 0.3f;

			RotOscillation.Pitch.Amplitude = 2.f;
			RotOscillation.Pitch.Frequency = 10.f;
			RotOscillation.Pitch.InitialOffset = EInitialOscillatorOffset::EOO_OffsetZero;

			RotOscillation.Yaw.Amplitude = 2.f;
			RotOscillation.Yaw.Frequency = 50.f;
			RotOscillation.Yaw.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

			RotOscillation.Roll.Amplitude = 2.f;
			RotOscillation.Roll.Frequency = 500.f;

			bSingleInstance = true;
			break;
		case 3:
			OscillationDuration = 0.3f;

			RotOscillation.Pitch.Amplitude = 2.f;
			RotOscillation.Pitch.Frequency = 10.f;
			RotOscillation.Pitch.InitialOffset = EInitialOscillatorOffset::EOO_OffsetZero;

			RotOscillation.Yaw.Amplitude = 2.f;
			RotOscillation.Yaw.Frequency = 50.f;
			RotOscillation.Yaw.InitialOffset = EInitialOscillatorOffset::EOO_OffsetRandom;

			RotOscillation.Roll.Amplitude = 2.f;
			RotOscillation.Roll.Frequency = 500.f;

			bSingleInstance = true;
			break;
	}
}
