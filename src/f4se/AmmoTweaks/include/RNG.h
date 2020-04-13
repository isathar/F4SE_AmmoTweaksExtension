#pragma once
#include <chrono>

/* random number generator:
- implementation of xoroshiro128+ (http://xoshiro.di.unimi.it/xoroshiro128plus.c)
*/
class ATxoroshiro128p
{
	const float fMaxFloatVal = (float)0xFFFFFFFF;
	const int a = 24, b = 16, c = 37, d = 64;		// xoroshiro parameters
	
	uint64_t state[2];								// current state


	// rotating left shift
	inline uint64_t rotl(const uint64_t x, int k)
	{
		return (x << k) | (x >> (d - k));
	}

	// get the next random number, advance the state
	UInt32 Advance()
	{
		const uint64_t s0 = state[0];
		uint64_t s1 = state[1];
		const uint64_t result = s0 + s1;
		s1 ^= s0;
		state[0] = rotl(s0, a) ^ s1 ^ (s1 << b);
		state[1] = rotl(s1, c);
		return result;
	}


public:
	// rng state seed
	void Seed()
	{
		// nanoseconds since 01 - 01 - 1970 is probably overkill, but whatever....
		state[0] = std::chrono::high_resolution_clock::now().time_since_epoch().count();;
		// reverse bit order
		state[1] = ((state[0] * 0x0202020202UL) & 0x010884422010UL) % 1023;
	}

	// returns a random int between minVal and maxVal (inclusive)
	int RandomInt(int minVal = 0, int maxVal = 100)
	{
		return (int)(((float)Advance() / fMaxFloatVal) * (float)(maxVal + 1 - minVal)) + minVal;
	}

	// returns a random float between minVal and maxVal (inclusive, 2 decimal places)
	float RandomFloat(float minVal = 0.0, float maxVal = 1.0)
	{
		return (float)(RandomInt((int)(minVal * 100.0), (int)(maxVal * 100.0))) * 0.01;
	}
};
