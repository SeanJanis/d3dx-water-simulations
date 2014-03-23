#pragma once

#include <string>
using namespace std;

#define MAX_NUM_GERSTNER_WAVES         10

struct GerstnerWave
{
   float vecWaveDirection[3];
	float fAmplitude;
	float fAngularFrequency;
	float fWaveLength;
	float fPhaseShift;
};