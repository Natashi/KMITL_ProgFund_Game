#include "pch.h"
#include "Rand.hpp"
#include "Utility.hpp"

//*******************************************************************
//RandProvider
//*******************************************************************
RandProvider* RandProvider::base_ = nullptr;
RandProvider::RandProvider() {
	ZeroMemory(states_, sizeof(states_));
	seed_ = 0;
}

inline uint64_t RandProvider::rotl(uint64_t u, size_t x) {
	return (u << x) | (u >> (64U - x));
}
inline uint64_t RandProvider::rotr(uint64_t u, size_t x) {
	return (u >> x) | (u << (64U - x));
}

void RandProvider::Initialize(uint32_t s) {
	if (base_) throw EngineError("RandProvider already initialized.");
	base_ = this;

	ZeroMemory(states_, sizeof(states_));

	static const uint64_t JUMP[] = { 
		(uint64_t)0x180ec6d33cfd0aba, (uint64_t)0xd5a61266f0c9392c,
		(uint64_t)0xa9582618e03fc9aa, (uint64_t)0x39abdc4529b1661c
	};

	seed_ = s;

	states_[0] = s ^ JUMP[0];
	for (uint64_t i = 1ui64; i < 4ui64; ++i) {
		uint64_t s = (uint64_t)0x76e15d3efefdcbbf * states_[i - 1] ^ (states_[i - 1] >> 37) + i;
		s ^= JUMP[i];
		states_[i] = s & UINT64_MAX;
	}
}
void RandProvider::Release() {
}

uint64_t RandProvider::_GenrandInt64() {
	uint64_t result = rotl(states_[1] * 5ui64, 7U) * 9ui64;

	uint64_t t = states_[1] << 17U;

	states_[2] ^= states_[0];
	states_[3] ^= states_[1];
	states_[1] ^= states_[2];
	states_[0] ^= states_[3];

	states_[2] ^= t;

	states_[3] = rotl(states_[3], 45U);

	return result;
}

int64_t RandProvider::GetInt() {
	return (int64_t)GetReal();
}
int64_t RandProvider::GetInt(int64_t min, int64_t max) {
	return (int64_t)GetReal(min, max);
}
double RandProvider::GetReal() {
	return (double)(_GenrandInt64() * (1.0 / (double)UINT64_MAX));
}
double RandProvider::GetReal(double a, double b) {
	if (a == b) return a;
	return (a + GetReal() * (b - a));
}