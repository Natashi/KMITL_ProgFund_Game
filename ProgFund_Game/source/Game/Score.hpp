#pragma once
#include "../../pch.h"

struct PlayerStats {
	uint64_t score = 0;
	uint32_t totalAbsorb = 0;
	uint32_t maxAbsorb = 0;
};

class PlayerDataUtil {
	static inline const std::string PATH_SAVE = "th_oes_score.dat";
public:
	using ScorePair = std::pair<std::string, PlayerStats>;
	using ScoreMap = std::multimap<uint64_t, ScorePair, std::greater<uint64_t>>;

	static constexpr const size_t MAX_ENTRY = 5;
public:
	static bool LoadPlayerData(ScoreMap* pMap) {
		if (pMap == nullptr) return false;
		pMap->clear();

		std::ifstream file;
		file.open(PATH_SAVE, std::ios::binary);

		if (file.is_open()) {
			file.seekg(0, std::ios::end);
			size_t fileSize = file.tellg();
			file.seekg(0, std::ios::beg);

			if (fileSize > 0) {
				bool bRead = true;
				auto _WrpRead = [&](void* data, size_t size) -> bool {
					file.read((char*)data, size);
					bRead = file.gcount() == size;
					return bRead;
				};

				size_t i = 0;
				while (bRead && file.good()) {
					std::pair<uint64_t, ScorePair> data;
					if (!_WrpRead(&data.first, sizeof(uint64_t))) goto lab_loop_e;
					{
						uint32_t strLen;
						if (!_WrpRead(&strLen, sizeof(uint32_t))) goto lab_loop_e;
						data.second.first.resize(strLen);
						if (!_WrpRead(data.second.first.data(), sizeof(char) * strLen)) goto lab_loop_e;
						if (!_WrpRead(&data.second.second, sizeof(PlayerStats))) goto lab_loop_e;
					}
					pMap->insert(data);
lab_loop_e:
					++i;
					if (i >= MAX_ENTRY) break;
				}
			}

			file.close();
			return true;
		}

		return false;
	}
	static bool SavePlayerData(ScoreMap* pMap) {
		if (pMap == nullptr) return false;

		std::ofstream file;
		file.open(PATH_SAVE, std::ios::binary | std::ios::trunc);

		if (file.is_open()) {
			size_t i = 0;
			for (auto itr = pMap->begin(); itr != pMap->end() && i < MAX_ENTRY; ++i, ++itr) {
				file.write((char*)&itr->first, sizeof(uint64_t));
				{
					uint32_t strLen = itr->second.first.size();
					file.write((char*)&strLen, sizeof(uint32_t));
					file.write(itr->second.first.data(), sizeof(char) * strLen);
				}
				file.write((char*)&itr->second.second, sizeof(PlayerStats));
			}

			file.close();
			return true;
		}

		return false;
	}
};