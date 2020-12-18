#include "pch.h"

#include "Menu.hpp"
#include "Pause.hpp"
#include "StageMain.hpp"
#include "Player.hpp"

Pause_CallParams g_menuParams[] = {
	//Pause
	{ "Temporary Respite", D3DCOLOR_XRGB(192, 192, 192), 0xffffffff,
	{ "Press Z to unpause.", "Press X to return to main menu." }, 0 },

	//Game over
	{ "Severely Wounded", D3DCOLOR_XRGB(255, 16, 16), D3DCOLOR_XRGB(128, 32, 32),
	{ "Game Over", "", "Press any key to return to main menu." }, 1 },

	//Game over
	{ "Mission Successful", D3DCOLOR_XRGB(255, 205, 0), 0xffffffff,
	{ "You have impressed the demon god.", "", "Press any key to return to main menu." }, 2 },
};

static void TransitionToMainMenu(Scene* parent) {
	SceneManager* manager = parent->GetParentManager();

	auto rearScene = manager->GetRearScene().get();
	rearScene->AddTask(new UtilTask_ColorFade(rearScene, 30, 30, 70, D3DCOLOR_XRGB(0, 0, 0)));

	class ToMenu : public TaskBase {
	public:
		ToMenu(Scene* parent) : TaskBase(parent) {
			frameEnd_ = 55;
		}
		~ToMenu() {
			SceneManager* manager = parent_->GetParentManager();
			{
				shared_ptr<Menu_MainScene> menuScene(new Menu_MainScene(manager));
				menuScene->SetType(Scene::Type::Menu);
				manager->AddScene(menuScene, Scene::Type::Menu, true);
			}
			manager->RemoveScene(Scene::Type::Pause);
			manager->RemoveScene(Scene::Type::Background);
			manager->RemoveScene(Scene::Type::StageUI);
			manager->RemoveScene(Scene::Type::Stage);
		}
		virtual void Update() {
			++frame_;
		}
	};
	rearScene->AddTask(new ToMenu(rearScene));
};

//*******************************************************************
//Pause_MainScene
//*******************************************************************
Pause_MainScene::Pause_MainScene(SceneManager* manager) : Scene(manager) {
	bPause_ = false;

	GET_INSTANCE(ResourceManager, resourceManager);
	pRenderTarget_ = resourceManager->GetResourceAs<TextureResource>("__RTARGET_0__");

	objBack_.SetTexture(pRenderTarget_);
	objBack_.SetSourceRect(DxRectangle(0, 0, 1024, 512));
	objBack_.SetDestRect(DxRectangle(0, 0, 640, 480));
	objBack_.UpdateVertexBuffer();

	objBlack_.SetDestRect(DxRectangle(0, 0, 640, 480));
	objBlack_.SetColor(0, 0, 0);
	objBlack_.UpdateVertexBuffer();

	tAlpha_ = 0;

	bAutoDelete_ = false;
}
Pause_MainScene::~Pause_MainScene() {

}

void Pause_MainScene::Render() {
	if (tAlpha_ > 0.001)
		objBack_.Render();
	objBlack_.Render();

	Scene::Render();
}
void Pause_MainScene::Update() {
	if (bPause_) {
		IncUntil(tAlpha_, 1 / 30.0, 1.0);
	}
	else {
		DecUntil(tAlpha_, 1 / 16.0, 0.0);
	}
	objBlack_.SetAlpha(tAlpha_ * 128);

	Scene::Update();
}

void Pause_MainScene::EnterPause(int type) {
	if (type == 0) {
		GET_INSTANCE(ScriptSoundLibrary, soundLib);
		soundLib->PlaySE("Pause");
	}

	bPause_ = true;

	{
		GET_INSTANCE(WindowMain, window);
		IDirect3DDevice9* device = window->GetDevice();

		device->SetRenderTarget(0, pRenderTarget_->GetSurface());
		{
			window->BeginScene();
			manager_->GetScene(Scene::Background)->Render();
			manager_->GetScene(Scene::Stage)->Render();
			manager_->GetScene(Scene::StageUI)->Render();
			window->EndScene(false);
		}
		device->SetRenderTarget(0, window->GetBackBuffer());
	}

	{
		auto stage = manager_->GetScene(Scene::Stage);
		stage->SetEnableUpdate(false);
		stage->SetEnableRender(false);

		auto stageUI = manager_->GetScene(Scene::StageUI);
		stageUI->SetEnableUpdate(false);
		stageUI->SetEnableRender(false);

		auto stageBG = manager_->GetScene(Scene::Background);
		stageBG->SetEnableUpdate(false);
		stageBG->SetEnableRender(false);
	}

	{
		Pause_CallParams params;
		switch (type) {
		case 1:		//Game Over
			params = g_menuParams[1];
			break;
		case 2:		//Game end
			params = g_menuParams[2];
			break;
		case 0:		//Pause
		default:
			params = g_menuParams[0];
			break;
		}
		this->AddTask(new Pause_PauseTask(this, params));
	}
}
void Pause_MainScene::LeavePause() {
	bPause_ = false;

	class Delay : public TaskBase {
	public:
		Delay(Scene* parent) : TaskBase(parent) {
			frameEnd_ = 16;
		}
		~Delay() {
			SceneManager* manager = parent_->GetParentManager();

			auto stage = manager->GetScene(Scene::Stage);
			stage->SetEnableUpdate(true);
			stage->SetEnableRender(true);

			auto stageUI = manager->GetScene(Scene::StageUI);
			stageUI->SetEnableUpdate(true);
			stageUI->SetEnableRender(true);

			auto stageBG = manager->GetScene(Scene::Background);
			stageBG->SetEnableUpdate(true);
			stageBG->SetEnableRender(true);
		}
		virtual void Update() { ++frame_; }
	};
	this->AddTask(new Delay(this));
}

//*******************************************************************
//Pause_PauseTask
//*******************************************************************
Pause_PauseTask::Pause_PauseTask(Scene* parent, const Pause_CallParams& params) : TaskBase(parent)
{
	GET_INSTANCE(ResourceManager, resourceManager);

	callParams_ = params;

	auto textureAscii = resourceManager->GetResourceAs<TextureResource>("img/system/ascii_960.png");

	{
		VertexTLX verts[4];
		std::vector<VertexTLX> vertex;
		std::vector<uint16_t> index;

		{
			objHeader_.SetTexture(textureAscii);

			DxRectangle<float> rcBaseDst(-14, -14, 14, 14);
			for (size_t iChar = 0; iChar < params.head.size(); ++iChar) {
				{
					uint16_t bii = iChar * 4;
					index.push_back(bii + 0);
					index.push_back(bii + 1);
					index.push_back(bii + 2);
					index.push_back(bii + 2);
					index.push_back(bii + 1);
					index.push_back(bii + 3);
				}

				{
					SystemUtility::SetVertexAsciiSingle(verts, params.head[iChar],
						rcBaseDst + DxRectangle<float>::SetFromSize(18 * iChar, 0),
						D3DXVECTOR2(21, 21), D3DXVECTOR2(512, 512));
					verts[0].diffuse = params.headColorTop;
					verts[1].diffuse = params.headColorTop;
					verts[2].diffuse = params.headColorBot;
					verts[3].diffuse = params.headColorBot;

					{
						vertex.push_back(verts[0]);
						vertex.push_back(verts[1]);
						vertex.push_back(verts[2]);
						vertex.push_back(verts[3]);
					}
				}
			}

			objHeader_.SetArrayVertex(vertex);
			objHeader_.SetArrayIndex(index);

			objHeader_.SetPosition(320 - (18 * (params.head.size() - 1)) / 2.0f, 192, 1);
		}
		{
			int y = 0;
			for (auto& str : params.desc) {
				vertex.clear();
				index.clear();

				if (str.size() > 0) {
					shared_ptr<StaticRenderObject2D> text(new StaticRenderObject2D());
					text->SetTexture(textureAscii);

					DxRectangle<float> rcBaseDst(-8, -8, 8, 8);
					DxRectangle<float> dstOff = DxRectangle<float>::SetFromSize(0, y);
					for (size_t iChar = 0; iChar < str.size(); ++iChar) {
						{
							uint16_t bii = iChar * 4;
							index.push_back(bii + 0);
							index.push_back(bii + 1);
							index.push_back(bii + 2);
							index.push_back(bii + 2);
							index.push_back(bii + 1);
							index.push_back(bii + 3);
						}

						{
							SystemUtility::SetVertexAsciiSingle(verts, str[iChar],
								rcBaseDst + dstOff, D3DXVECTOR2(21, 21), D3DXVECTOR2(512, 512));
							{
								vertex.push_back(verts[0]);
								vertex.push_back(verts[1]);
								vertex.push_back(verts[2]);
								vertex.push_back(verts[3]);
							}
						}

						dstOff += DxRectangle<float>::SetFromSize(11, 0);
					}

					text->SetArrayVertex(vertex);
					text->SetArrayIndex(index);

					text->SetPosition(320 - (11 * (str.size() - 1)) / 2.0f, 192 + 64, 1);

					objDescription_.push_back(text);
				}

				y += 17;
			}
		}
	}

	bEnd_ = false;
	typePause_ = params.type;
	flgGetInput_ = false;
}

void Pause_PauseTask::Render(byte layer) {
	if (layer != 3) return;
	objHeader_.Render();
	for (auto& obj : objDescription_)
		obj->Render();
}
void Pause_PauseTask::Update() {
	if (!bEnd_) {
		if (frame_ < 24) {
			double tmp = frame_ / 23.0;
			int tmp_a = Math::Lerp::Linear(0, 255, tmp);
			objHeader_.SetAlpha(tmp_a);
			for (auto& obj : objDescription_)
				obj->SetAlpha(tmp_a);
		}
	}
	else {
		if (frameEnd_ == UINT_MAX) {
			frameEnd_ = 14;
			frame_ = 0;
		}

		double tmp = frame_ / 14.0;
		double tmp_s = Math::Lerp::Smooth<double>(1, 0, tmp);
		objHeader_.SetScaleY(tmp_s);
		for (auto& obj : objDescription_)
			obj->SetScaleY(tmp_s);
	}

	if (!bEnd_) {
		GET_INSTANCE(ScriptSoundLibrary, soundLib);
		GET_INSTANCE(InputManager, inputManager);

		if (flgGetInput_ && frame_ > 20) {
			bool statePause = inputManager->GetKeyState(VirtualKey::Pause) == KeyState::Push;
			bool stateZ = inputManager->GetKeyState(VirtualKey::Ok) == KeyState::Push;
			bool stateX = inputManager->GetKeyState(VirtualKey::Cancel) == KeyState::Push;

			switch (typePause_) {
			case 0:		//Pause
			{
				if (statePause || stateZ) {
					//Unpause
					bEnd_ = true;

					soundLib->PlaySE("Unpause");

					((Pause_MainScene*)parent_)->LeavePause();
				}
				else if (stateX) {
					//To highscore
					//_ToMenu();
					parent_->AddTask(new Pause_NameRegistTask(parent_, g_menuParams[typePause_]));
					soundLib->PlaySE("Ok");
					bEnd_ = true;
				}
				break;
			}
			case 1:		//Game Over
			case 2:		//Game end
			{
				if (stateZ || stateX) {
					//To highscore (coming soon?)
					parent_->AddTask(new Pause_NameRegistTask(parent_, g_menuParams[typePause_]));
					soundLib->PlaySE("Ok");
					bEnd_ = true;
				}
				break;
			}
			}
		}
		else {		//Wait until the user releases the pause key
			flgGetInput_ = inputManager->GetKeyState(VirtualKey::Pause) == KeyState::Free &&
				inputManager->GetKeyState(VirtualKey::Cancel) == KeyState::Free;
		}
	}

	++frame_;
}

//*******************************************************************
//Pause_NameRegistTask
//*******************************************************************
Pause_NameRegistTask::Pause_NameRegistTask(Scene* parent, const Pause_CallParams& prevMenuParams) : TaskBase(parent) {
	GET_INSTANCE(ResourceManager, resourceManager);

	{
		auto stage = parent_->GetParentManager()->GetSceneAs<Stage_MainScene>(Scene::Stage);
		auto objPlayer = stage->GetPlayer();

		statPlayer_ = objPlayer->GetPlayerData()->stat;
	}

	if (statPlayer_.score == 0) {
		bEnd_ = true;
		TransitionToMainMenu(parent_);
		return;
	}

	prevMenuData_ = prevMenuParams;

	auto textureAscii = resourceManager->GetResourceAs<TextureResource>("img/system/ascii.png");

	{
		objPlayerStats_.resize(2);

		VertexTLX verts[4];
		std::vector<VertexTLX> vertex;
		std::vector<uint16_t> index;

		{
			std::string displayText =
				"Score              Total Absorbs      Highest Chain";
			{
				DxRectangle<float> rcBaseDst(-8, -8, 8, 8);
				DxRectangle<float> dstOff;
				for (size_t iChar = 0; iChar < displayText.size(); ++iChar) {
					{
						uint16_t bii = iChar * 4;
						index.push_back(bii + 0);
						index.push_back(bii + 1);
						index.push_back(bii + 2);
						index.push_back(bii + 2);
						index.push_back(bii + 1);
						index.push_back(bii + 3);
					}

					{
						SystemUtility::SetVertexAsciiSingle(verts, displayText[iChar],
							rcBaseDst + dstOff, D3DXVECTOR2(14, 14), D3DXVECTOR2(256, 256));
						for (int i = 0; i < 4; ++i)
							verts[i].diffuse = D3DCOLOR_XRGB(199, 255, 232);
						{
							vertex.push_back(verts[0]);
							vertex.push_back(verts[1]);
							vertex.push_back(verts[2]);
							vertex.push_back(verts[3]);
						}
					}

					dstOff += DxRectangle<float>::SetFromSize(9, 0);
				}

				objPlayerStats_[0].SetTexture(textureAscii);

				objPlayerStats_[0].SetArrayVertex(vertex);
				objPlayerStats_[0].SetArrayIndex(index);

				objPlayerStats_[0].SetPosition(92, 108, 1);
			}
		}

		vertex.clear();
		index.clear();
		{
			std::string displayText;
			displayText += StringUtility::Format("%-9s          ", 
				std::to_string(statPlayer_.score).c_str());
			displayText += StringUtility::Format("%-5s              ",
				std::to_string(std::min<uint32_t>(statPlayer_.totalAbsorb, 99999)).c_str());
			displayText += StringUtility::Format("%-5s",
				std::to_string(std::min<uint32_t>(statPlayer_.maxAbsorb, 99999)).c_str());
			{
				DxRectangle<float> rcBaseDst(-8, -8, 8, 8);
				DxRectangle<float> dstOff;
				for (size_t iChar = 0; iChar < displayText.size(); ++iChar) {
					{
						uint16_t bii = iChar * 4;
						index.push_back(bii + 0);
						index.push_back(bii + 1);
						index.push_back(bii + 2);
						index.push_back(bii + 2);
						index.push_back(bii + 1);
						index.push_back(bii + 3);
					}

					{
						SystemUtility::SetVertexAsciiSingle(verts, displayText[iChar],
							rcBaseDst + dstOff, D3DXVECTOR2(14, 14), D3DXVECTOR2(256, 256));
						for (int i = 0; i < 4; ++i)
							verts[i].diffuse = D3DCOLOR_XRGB(224, 224, 224);
						{
							vertex.push_back(verts[0]);
							vertex.push_back(verts[1]);
							vertex.push_back(verts[2]);
							vertex.push_back(verts[3]);
						}
					}

					dstOff += DxRectangle<float>::SetFromSize(9, 0);
				}

				objPlayerStats_[1].SetTexture(textureAscii);

				objPlayerStats_[1].SetArrayVertex(vertex);
				objPlayerStats_[1].SetArrayIndex(index);

				objPlayerStats_[1].SetPosition(92, 108 + 32, 1);
			}
		}
	}

	{
		objNameDisplay_.resize(MAX_NAME_SIZE);

		size_t _i = 0;
		for (auto& iChar : objNameDisplay_) {
			iChar.SetTexture(textureAscii);

			DxRectangle<float> rcChar = SystemUtility::GetAsciiRect('_', 14, 14);
			iChar.SetSourceRect(rcChar);
			iChar.SetDestRect(DxRectangle<int>(-8, -8, 8, 8));
			iChar.UpdateVertexBuffer();

			iChar.SetPosition(212 + 13 * _i, 216, 1);

			++_i;
		}

		nameLength_ = 0;
		name_.resize(MAX_NAME_SIZE);
	}

	{
		std::vector<char> listCharRender = {
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
			'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
			'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '-', '=',
			'.', ',', '!', '?', '@', ':', ';', '[', ']', '(', ')', '_', '/',
			'{', '}', '|', '~', '^', '#', '$', '%', '&', '*', '\xf2', '\xf0', '\xf1'
		};
		
		listAllChars_.resize(listCharRender.size());
		size_t _i = 0;
		for (auto& iChar : listAllChars_) {
			int tx = _i % 13;
			int ty = _i / 13;

			{
				char ch = listCharRender[_i];

				iChar.second = ch;
				iChar.first.SetTexture(textureAscii);

				DxRectangle<float> rcChar = SystemUtility::GetAsciiRect(ch, 14, 14);
				iChar.first.SetSourceRect(rcChar);
				iChar.first.SetDestRect(DxRectangle<int>(-8, -8, 8, 8));
				iChar.first.UpdateVertexBuffer();

				iChar.first.SetPosition(186 + 22 * tx, 276 + 20 * ty, 1);
			}

			++_i;
		}
	}

	frameHoldDelay_ = 0;
	bEnd_ = false;
	selectIndex_ = 0;
}

void Pause_NameRegistTask::Render(byte layer) {
	if (layer != 2) return;
	for (auto& iChar : objPlayerStats_)
		iChar.Render();
	for (size_t i = 0; i < objNameDisplay_.size() && i < nameLength_; ++i)
		objNameDisplay_[i].Render();
	for (auto& iChar : listAllChars_)
		iChar.first.Render();
}
void Pause_NameRegistTask::Update() {
	GET_INSTANCE(ScriptSoundLibrary, soundLib);
	GET_INSTANCE(InputManager, inputManager);

	if (!bEnd_) {
		if (frame_ < 12) {
			double tmp = frame_ / 11.0;
			int tmp_a = Math::Lerp::Linear(0, 255, tmp);

			for (auto& iChar : objPlayerStats_)
				iChar.SetAlpha(tmp_a);
			for (auto& iChar : objNameDisplay_)
				iChar.SetAlpha(tmp_a);
			for (auto& iChar : listAllChars_)
				iChar.first.SetAlpha(tmp_a);
		}
	}
	else {
		if (frameEnd_ == UINT_MAX) {
			frameEnd_ = 16;
			frame_ = 0;
		}

		double tmp = frame_ / 16.0;
		int tmp_a = Math::Lerp::Linear(255, 0, tmp);

		for (auto& iChar : objPlayerStats_)
			iChar.SetAlpha(tmp_a);
		for (auto& iChar : objNameDisplay_)
			iChar.SetAlpha(tmp_a);
		for (auto& iChar : listAllChars_)
			iChar.first.SetAlpha(tmp_a);
	}

	if (!bEnd_) {
		{
			int i = 0;
			for (auto& iChar : listAllChars_) {
				if (i == selectIndex_) {
					float sc = 1 + sinf(frame_ * GM_DTORA(6)) * 0.08;
					iChar.first.SetScale(sc, sc, 1);
					iChar.first.SetColor(D3DCOLOR_XRGB(87, 122, 241));
				}
				else {
					iChar.first.SetScale(1, 1, 1);
					iChar.first.SetColor(0xffffffff);
				}
				++i;
			}
		}

		if (frame_ > 15) {
			constexpr int COUNT_CHARS = 7 * 13;

			auto _ChkPush = [&](VirtualKey key) {
				auto state = inputManager->GetKeyState(key);
				if (state == KeyState::Push) {
					frameHoldDelay_ = 0;
					return true;
				}
				else if (state == KeyState::Hold) {
					++frameHoldDelay_;
					return (frameHoldDelay_ > 15) && (frameHoldDelay_ % 8 == 0);
				}
				return false;
			};

			if (_ChkPush(VirtualKey::Up)) {
				selectIndex_ -= 13;
				while (selectIndex_ < 0) selectIndex_ += COUNT_CHARS;
				soundLib->PlaySE("Select");
			}
			else if (_ChkPush(VirtualKey::Down)) {
				selectIndex_ += 13;
				selectIndex_ %= COUNT_CHARS;
				soundLib->PlaySE("Select");
			}
			else if (_ChkPush(VirtualKey::Left)) {
				int row = selectIndex_ / 13 * 13;
				int col = selectIndex_ % 13 - 1;
				while (col < 0) col += 13;
				selectIndex_ = row + col;
				soundLib->PlaySE("Select");
			}
			else if (_ChkPush(VirtualKey::Right)) {
				int row = selectIndex_ / 13 * 13;
				int col = selectIndex_ % 13 + 1;
				col %= 13;
				selectIndex_ = row + col;
				soundLib->PlaySE("Select");
			}
			else if (_ChkPush(VirtualKey::Ok)) {
				char ch = listAllChars_[selectIndex_].second;

				switch (ch) {
				case '\xf0':	//"Backspace"
				{
					name_[nameLength_] = '\0';
					if (nameLength_ > 0)
						--nameLength_;

					soundLib->PlaySE("Cancel");
					break;
				}
				case '\xf1':	//Confirm name
				{
					bEnd_ = true;

					if (nameLength_ > 0) {
						std::string registName = name_;
						registName.resize(nameLength_);

						PlayerDataUtil::ScoreMap mapScores;
						PlayerDataUtil::LoadPlayerData(&mapScores);			//Loads saved score

						//Also checks if the current score is WORTHY
						if ((mapScores.size() < PlayerDataUtil::MAX_ENTRY)
							|| (std::prev(mapScores.end())->first >= statPlayer_.score)) {
							mapScores.insert(std::make_pair(statPlayer_.score,
								std::make_pair(registName, statPlayer_)));	//Adds the current score
							PlayerDataUtil::SavePlayerData(&mapScores);		//Save the score

							soundLib->PlaySE("extend");
						}
						else {
							soundLib->PlaySE("Invalid");
						}

						TransitionToMainMenu(parent_);
					}
					else {
						soundLib->PlaySE("Cancel");
						TransitionToMainMenu(parent_);
					}
					break;
				}
				default:
				{
					Sprite2D& objChar = objNameDisplay_[std::min(nameLength_, MAX_NAME_SIZE - 1)];

					DxRectangle<float> rcChar = SystemUtility::GetAsciiRect(ch, 14, 14);
					objChar.SetSourceRect(rcChar);
					objChar.UpdateVertexBuffer();

					name_[nameLength_] = ch;
					if (nameLength_ < MAX_NAME_SIZE)
						++nameLength_;
					if (nameLength_ == MAX_NAME_SIZE)
						selectIndex_ = COUNT_CHARS - 1;

					soundLib->PlaySE("Ok");
					break;
				}
				}
			}
			else if (_ChkPush(VirtualKey::Cancel)) {
				name_[nameLength_] = '\0';
				if (nameLength_ > 0)
					--nameLength_;
				else {
					bEnd_ = true;
					parent_->AddTask(new Pause_PauseTask(parent_, prevMenuData_));
				}

				soundLib->PlaySE("Cancel");
			}
		}
	}

	++frame_;
	//if (frame_ == frameEnd_) __debugbreak();
}