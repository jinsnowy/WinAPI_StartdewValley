#include "GameManager.h"
#include "UIGameTimer.h"
#include "UISeedStore.h"
#include "UIFastItemList.h"
#include "UIPlayerInfo.h"
#include "UIPanel.h"
#include "../../Application/Window.h"
#include "../../Core/Input.h"
#include "../../Resources/Texture.h"
#include "../../Collider/ColliderRect.h"
#include "../Object.h"
#include "../MoveObj/Player.h"
#include "../Item/Item.h"
#include "../StaticObj/Plant.h"
#include "../../Sound/SoundManager.h"

DEFINITION_SINGLE(GameManager);

UIGameTimer* GameManager::GetTimer() const
{
	return static_cast<UIGameTimer*>(m_uiPanels[(int)PANEL_TYPE::GAME_TIMER].get());
}

UISeedStore* GameManager::GetStore() const
{
	return static_cast<UISeedStore*>(m_uiPanels[(int)PANEL_TYPE::SEED_STORE].get());
}

bool GameManager::Init()
{
	INPUT->AddKey("PlayerUI", 'E');
	SOUND_MANAGER->LoadSound("OpenUI", false, SD_EFFECT, "OpenUI.mp3");
	SOUND_MANAGER->LoadSound("CloseUI", false, SD_EFFECT, "CloseUI.mp3");
	SOUND_MANAGER->LoadSound("InteractUI", false, SD_EFFECT, "InteractUI.mp3");
	SOUND_MANAGER->LoadSound("ScrollSound", false, SD_EFFECT, "ScrollSound.mp3");

	m_uiPanels.resize((int)PANEL_TYPE::PANEL_END);
	m_uiPanels[(int)PANEL_TYPE::PLAYER_INFO] = make_shared<UIPlayerInfo>();
	m_uiPanels[(int)PANEL_TYPE::GAME_TIMER] = make_shared<UIGameTimer>();
	m_uiPanels[(int)PANEL_TYPE::FAST_ITEMLIST] = make_shared<UIFastItemList>();
	m_uiPanels[(int)PANEL_TYPE::SEED_STORE] = make_shared<UISeedStore>();
	m_uiStates.reset();
	return true;
}

void GameManager::Start()
{
	m_uiStates[(int)PANEL_TYPE::GAME_TIMER] = true;
	m_uiStates[(int)PANEL_TYPE::FAST_ITEMLIST] = true;
}

void GameManager::Acknowleged(UI* changedUI)
{
	const auto& pTimer = GetTimer();
	const auto& pStore = GetStore();
	if (changedUI == pTimer)
	{
		m_iWorldTime = pTimer->GetWorldTime();
		UpdateFarm();
	}
	else if (changedUI == pStore)
	{
		bool show = pStore->IsOn();
		m_uiStates[(int)PANEL_TYPE::SEED_STORE] = show;
		if (show)
		{
			Pos tPos = pStore->GetPos();
			tPos.y -= pTimer->GetSize().y - 70.f;
			tPos.y += pStore->GetSize().y;
			pTimer->SetPos(tPos);
			pStore->SetClickDelay();
			m_pPlayer->DisableMovement();
			SOUND_MANAGER->PlayMusic("OpenUI");
		}
		else 
		{
			pTimer->SetNormalPos();
			m_pPlayer->EnableMovement();
			SOUND_MANAGER->PlayMusic("CloseUI");
		}
	}
}

void GameManager::Input(float dt)
{
	if (KEYDOWN("PlayerUI") && !Activated((int)PANEL_TYPE::SEED_STORE))
	{
		m_uiStates.flip((int)PANEL_TYPE::PLAYER_INFO);
		m_uiStates.flip((int)PANEL_TYPE::FAST_ITEMLIST);
		Activated((int)PANEL_TYPE::PLAYER_INFO) ? 
			SOUND_MANAGER->PlayMusic("OpenUI") : SOUND_MANAGER->PlayMusic("CloseUI");
	}
	for (int i = 0; i < (int)PANEL_TYPE::PANEL_END; ++i)
	{
		if (Activated(i))
		{
			m_uiPanels[i]->Input(dt);
		}
	}
}

int GameManager::Update(float dt)
{
	for (int i = 0; i < (int)PANEL_TYPE::PANEL_END; ++i)
	{
		if (Activated(i))
		{
			m_uiPanels[i]->Update(dt);
		}
	}
	return 0;
}

int GameManager::LateUpdate(float dt)
{
	for (int i = 0; i < (int)PANEL_TYPE::PANEL_END; ++i)
	{
		if (Activated(i))
		{
			m_uiPanels[i]->LateUpdate(dt);
		}
	}
    return 0;
}

void GameManager::Collision(float dt)
{
	for (int i = 0; i < (int)PANEL_TYPE::PANEL_END; ++i)
	{
		if (Activated(i))
		{
			m_uiPanels[i]->Collision(dt);
		}
	}
}

void GameManager::Draw(HDC hdc, float dt)
{
	for (int i = 0; i < (int)PANEL_TYPE::PANEL_END; ++i)
	{
		if (Activated(i))
		{
			m_uiPanels[i]->Draw(hdc, dt);
		}
	}
}

void GameManager::AddWateredTile(int index)
{
	if(IsWateredTile(index))
	{
		const auto find_by_index = [index](const WateredTile &pTile) { return pTile.GetIndex() == index; };
		auto iter = find_if(m_WateredTiles.begin(), m_WateredTiles.end(), find_by_index);
		m_WateredTiles.erase(iter);
	}

	m_WateredTiles.emplace_back(index);
	m_setWateredTileIndices.insert(index);
}

vector<int> GameManager::GetDroughtTiles()
{
	vector<int> vecDroughtTiles;
	while (!m_WateredTiles.empty())
	{
		const auto & tile = m_WateredTiles.front();
		if (tile.IsDrought())
		{
			int index = tile.GetIndex();
			vecDroughtTiles.push_back(index);
			m_setWateredTileIndices.erase(index);
			m_WateredTiles.pop_front();
		}
		else
		{
			break;
		}
	}
	return vecDroughtTiles;
}

bool GameManager::IsWateredTile(int index) const
{
	return m_setWateredTileIndices.find(index) != m_setWateredTileIndices.end();
}

void GameManager::UpdateFarm()
{
	for (auto iter = m_WateredTiles.begin(); iter != m_WateredTiles.end(); ++iter)
	{
		iter->Update();
	}

	for (auto iter = m_plantList.begin(); iter != m_plantList.end();)
	{
		int index = (*iter)->GetTileIndex();
		if (IsWateredTile(index))
		{
			(*iter)->Grow();
		}
		else
		{
			(*iter)->Peek();
		}

		if (!(*iter)->GetLife())
		{
			SAFE_RELEASE((*iter));
			iter = m_plantList.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void GameManager::AddPlantList(Plant* pPlant)
{
	pPlant->AddRef();
	m_plantList.push_back(pPlant);
}

void GameManager::GameTimerFastForward()
{
	GetTimer()->GameTimerFastForward();
}

void GameManager::SetPlayer(Player* pPlayer)
{
	m_pPlayer = pPlayer;
	if (m_pPlayer)
		m_pPlayer->AddRef();
}

unsigned long long GameManager::GetWorldTime() const
{
	return m_iWorldTime;
}

float GameManager::GetDayDarkNess() const
{
	if (!m_uiPanels[(int)PANEL_TYPE::GAME_TIMER]) return 0.0f;

	return GetTimer()->GetDayDarkNess();
}

void GameManager::SleepUntilMorning()
{
	m_pPlayer->Sleep();
	UIGameTimer* pTimer = GetTimer();
	while (!pTimer->IsMorning())
	{
		GameTimerFastForward();
	}
	GameTimerFastForward();
}

GameManager::GameManager()
{

}

GameManager::~GameManager()
{
	SAFE_RELEASE(m_pPlayer);
	Safe_Release_VecList(m_plantList);

	Item::ClearItemMap();
	Plant::ClearPlantMap();
}

void GameManager::WateredTile::Reset()
{
	m_iWaterTime = GAMEWORLDTIME;
}

GameManager::WateredTile::WateredTile(int index)
	: m_iIndex(index), m_iWaterTime(GAMEWORLDTIME)
{
}

GameManager::WateredTile::~WateredTile()
{
}

void GameManager::WateredTile::Update()
{
	if (GAMEWORLDTIME - m_iWaterTime >= m_iDroughtDuration)
		m_bDrought = true;
}