#pragma once
#include "../../pch.h"

//Xoroshiro256**
class RandProvider {
	static RandProvider* base_;
private:
	uint64_t states_[4];
	uint32_t seed_;

	uint64_t _GenrandInt64();
	inline uint64_t rotl(uint64_t u, size_t x);
	inline uint64_t rotr(uint64_t u, size_t x);
public:
	RandProvider();

	static RandProvider* const GetBase() { return base_; }

	void Initialize(uint32_t s);
	void Release();

	uint32_t GetSeed() { return seed_; }

	int64_t GetInt();
	int64_t GetInt(int64_t min, int64_t max);
	double GetReal();
	double GetReal(double min, double max);
};