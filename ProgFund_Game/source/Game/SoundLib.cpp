#include "pch.h"
#include "SoundLib.hpp"

ScriptSoundLibrary* ScriptSoundLibrary::base_ = nullptr;
ScriptSoundLibrary::ScriptSoundLibrary() {
	if (base_) return;
	base_ = this;

	GET_INSTANCE(ResourceManager, resourceManager);

	{
		std::string nameBase = "sound/se/";
		std::string pathBase = "resource/" + nameBase;

		auto _Load = [&](const std::string& name) -> shared_ptr<SoundResource> {
			return resourceManager->LoadResource<SoundResource>(pathBase + name, nameBase + name);
		};

		mapSound_["Select"] = _Load("se_select00.wav");
		mapSound_["Cancel"] = _Load("se_cancel.wav");
		mapSound_["Invalid"] = _Load("se_invalid.wav");
		mapSound_["Ok"] = _Load("se_ok00.wav");
		mapSound_["Pause"] = _Load("se_pause.wav");
		mapSound_["Unpause"] = _Load("se_unpause.wav");

		mapSound_["bossdown"] = _Load("bossdown.wav");
		mapSound_["enemydown"] = _Load("enemydown.wav");
		mapSound_["playerdown"] = _Load("se_pldead.wav");

		mapSound_["damage0"] = _Load("damage0.wav");
		mapSound_["damage1"] = _Load("damage1.wav");
		mapSound_["boon0"] = _Load("se_boon00.wav");
		mapSound_["boon1"] = _Load("se_boon01.wav");
		mapSound_["spellcard"] = _Load("se_cat00.wav");
		mapSound_["charge"] = _Load("se_ch02.wav");
		mapSound_["flare"] = _Load("se_enep02.wav");
		mapSound_["slash"] = _Load("se_slash.wav");

		mapSound_["bonus"] = _Load("se_bonus2.wav");
		mapSound_["capture"] = _Load("se_cardget.wav");
		mapSound_["switch"] = _Load("se_change.wav");
		mapSound_["extend"] = _Load("se_extend.wav");

		mapSound_["shot1"] = _Load("shot1.wav");
		mapSound_["shot2"] = _Load("shot2.wav");
		mapSound_["shot3"] = _Load("shot3.wav");
		mapSound_["plshot"] = _Load("se_plshot.wav");
	}
}

void ScriptSoundLibrary::PlaySE(const std::string& id, double volume) {
	auto itr = mapSound_.find(id);
	if (itr != mapSound_.end()) {
		itr->second->GetData()->SetVolumeRate(volume);
		itr->second->Play();
	}
}