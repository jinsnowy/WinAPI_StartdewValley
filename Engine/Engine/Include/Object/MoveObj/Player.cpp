#include <cassert>
#include "Player.h"
#include "../../Math.h"
#include "../../Scene/Scene.h"
#include "../../Core/Input.h"
#include "../../Resources/ResourceManager.h"
#include "../../Resources/Texture.h"
#include "../../Collider/ColliderRect.h"
#include "../../Collider/ColliderPixel.h"
#include "../../Collider/CollisionManager.h"
#include "../../Core/Camera.h"
#include "../../Animation/Animation.h"
#include "../../Scene/SceneManager.h"
#include "../../Scene/GameScene.h"
#include "../../Sound/SoundManager.h"
#include "../../Collider/ColliderPoint.h"
#include "PlayerTool.h"
#include "../Item/Item.h"
#include "../Item/Seed.h"
#include "../Item/Tool.h"

Player::Player()
	:
	m_tPrev(Pos())
{
	m_vecItem.resize(m_iMaxItemNum, nullptr);
}

Player::~Player()
{
	Safe_Release_VecList(m_vecItem);
}

void Player::InitTexture()
{
	Texture* pTex;
	pTex = RESOURCE_MANAGER->LoadTexture("IdleRight", L"Player/SV/IdleRight.bmp");
	pTex->SetColorKey(255, 255, 255);
	SAFE_RELEASE(pTex);
	pTex = RESOURCE_MANAGER->LoadTexture("IdleLeft", L"Player/SV/IdleLeft.bmp");
	pTex->SetColorKey(255, 255, 255);
	SAFE_RELEASE(pTex);
	pTex = RESOURCE_MANAGER->LoadTexture("IdleDown", L"Player/SV/IdleDown.bmp");
	pTex->SetColorKey(255, 255, 255);
	SAFE_RELEASE(pTex);
	pTex = RESOURCE_MANAGER->LoadTexture("IdleUp", L"Player/SV/IdleUp.bmp");
	pTex->SetColorKey(255, 255, 255);
	SAFE_RELEASE(pTex);
}

void Player::InitAnimation()
{
	Animation* pAnim = CreateAnimation("PlayerAnimation");
	SAFE_RELEASE(pAnim);
	wstring basePath;
	vector<wstring> vecFileName;
	wstringstream ss;

	// Walk �ִϸ��̼�
	{
		basePath = L"Player/SV/WalkRight/";
		vecFileName.clear();
		for (int i = 1; i <= 6; ++i)
		{
			ss << basePath << L"WalkRight" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("WalkRight",
			AT_FRAME, AO_LOOP,
			0.0f, WALKSPEED,
			6, 1,
			0, 0,
			6, 1,
			0.f, "WalkRight_Anim", vecFileName);
		SetClipColorKey("WalkRight", 255, 255, 255);

		basePath = L"Player/SV/WalkLeft/";
		vecFileName.clear();
		for (int i = 1; i <= 6; ++i)
		{
			ss << basePath << L"WalkLeft" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("WalkLeft",
			AT_FRAME, AO_LOOP,
			0.0f, WALKSPEED,
			6, 1,
			0, 0,
			6, 1,
			0.f, "WalkLeft_Anim", vecFileName);
		SetClipColorKey("WalkLeft", 255, 255, 255);

		basePath = L"Player/SV/WalkDown/";
		vecFileName.clear();
		for (int i = 1; i <= 9; ++i)
		{
			ss << basePath << L"WalkDown" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("WalkDown",
			AT_FRAME, AO_LOOP,
			0.0f, WALKSPEED,
			9, 1,
			0, 0,
			9, 1,
			0.f, "WalkDown_Anim", vecFileName);
		SetClipColorKey("WalkDown", 255, 255, 255);

		basePath = L"Player/SV/WalkUp/";
		vecFileName.clear();
		for (int i = 1; i <= 9; ++i)
		{
			ss << basePath << L"WalkUp" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("WalkUp",
			AT_FRAME, AO_LOOP,
			0.0f, WALKSPEED,
			9, 1,
			0, 0,
			9, 1,
			0.f, "WalkUp_Anim", vecFileName);
		SetClipColorKey("WalkUp", 255, 255, 255);
	}

	// Swing �ִϸ��̼�
	{
		basePath = L"Player/SV/SwingRight/";
		vecFileName.clear();
		for (int i = 1; i <= 5; ++i)
		{
			ss << basePath << L"SwingRight" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("SwingRight",
			AT_FRAME, AO_ONCE_RETURN,
			0.0f, SWINGSPEED,
			5, 1,
			0, 0,
			5, 1,
			0.f, "SwingRight_Anim", vecFileName);
		SetClipColorKey("SwingRight", 255, 255, 255);
		SetClipNextState("SwingRight", IDLE_RIGHT);

		basePath = L"Player/SV/SwingLeft/";
		vecFileName.clear();
		for (int i = 1; i <= 5; ++i)
		{
			ss << basePath << L"SwingLeft" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("SwingLeft",
			AT_FRAME, AO_ONCE_RETURN,
			0.0f, SWINGSPEED,
			5, 1,
			0, 0,
			5, 1,
			0.f, "SwingLeft_Anim", vecFileName);
		SetClipColorKey("SwingLeft", 255, 255, 255);
		SetClipNextState("SwingLeft", IDLE_LEFT);

		basePath = L"Player/SV/SwingUp/";
		vecFileName.clear();
		for (int i = 1; i <= 5; ++i)
		{
			ss << basePath << L"SwingUp" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("SwingUp",
			AT_FRAME, AO_ONCE_RETURN,
			0.0f, SWINGSPEED,
			5, 1,
			0, 0,
			5, 1,
			0.f, "SwingUp_Anim", vecFileName);
		SetClipColorKey("SwingUp", 255, 255, 255);
		SetClipNextState("SwingUp", IDLE_UP);

		basePath = L"Player/SV/SwingDown/";
		vecFileName.clear();
		for (int i = 1; i <= 5; ++i)
		{
			ss << basePath << L"SwingDown" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("SwingDown",
			AT_FRAME, AO_ONCE_RETURN,
			0.0f, SWINGSPEED,
			5, 1,
			0, 0,
			5, 1,
			0.f, "SwingDown_Anim", vecFileName);
		SetClipColorKey("SwingDown", 255, 255, 255);
		SetClipNextState("SwingDown", IDLE_DOWN);
	}

	// Tool �ִϸ��̼�
	{
		basePath = L"Player/SV/ToolDown/";
		vecFileName.clear();
		for (int i = 1; i <= 3; ++i)
		{
			ss << basePath << L"ToolDown" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("ToolDown",
			AT_FRAME, AO_ONCE_RETURN,
			0.0f, TOOLSPEED,
			3, 1,
			0, 0,
			3, 1,
			0.f, "ToolDown_Anim", vecFileName);
		SetClipColorKey("ToolDown", 255, 255, 255);
		SetClipNextState("ToolDown", IDLE_DOWN);

		basePath = L"Player/SV/ToolUp/";
		vecFileName.clear();
		for (int i = 1; i <= 3; ++i)
		{
			ss << basePath << L"ToolUp" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("ToolUp",
			AT_FRAME, AO_ONCE_RETURN,
			0.0f, TOOLSPEED,
			3, 1,
			0, 0,
			3, 1,
			0.f, "ToolUp_Anim", vecFileName);
		SetClipColorKey("ToolUp", 255, 255, 255);
		SetClipNextState("ToolUp", IDLE_UP);

		basePath = L"Player/SV/ToolLeft/";
		vecFileName.clear();
		for (int i = 1; i <= 3; ++i)
		{
			ss << basePath << L"ToolLeft" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("ToolLeft",
			AT_FRAME, AO_ONCE_RETURN,
			0.0f, TOOLSPEED,
			3, 1,
			0, 0,
			3, 1,
			0.f, "ToolLeft_Anim", vecFileName);
		SetClipColorKey("ToolLeft", 255, 255, 255);
		SetClipNextState("ToolLeft", IDLE_LEFT);

		basePath = L"Player/SV/ToolRight/";
		vecFileName.clear();
		for (int i = 1; i <= 3; ++i)
		{
			ss << basePath << L"ToolRight" << i << ".bmp";
			vecFileName.push_back(ss.str());
			ss.clear();
			ss.str(L"");
		}
		AddAnimationClip("ToolRight",
			AT_FRAME, AO_ONCE_RETURN,
			0.0f, TOOLSPEED,
			3, 1,
			0, 0,
			3, 1,
			0.f, "ToolRight_Anim", vecFileName);
		SetClipColorKey("ToolRight", 255, 255, 255);
		SetClipNextState("ToolRight", IDLE_RIGHT);
	}
}

float Player::GetToolPower() const
{
	if(IsToolSelected())
		return static_cast<Tool*>(m_vecItem[m_iCurItemSel])->GetPower();
	return 0.f;
}

void Player::SetCurItemSel(int sel)
{
	if (sel < 0 || sel >= m_iMaxItemNum)
		return;

	m_iCurItemSel = sel; 
	if (m_vecItem[m_iCurItemSel] && m_vecItem[m_iCurItemSel]->IsToolItem())
	{
		m_pPlayerTool->SetTool(m_vecItem[m_iCurItemSel]);
	}
}

Item* Player::GetCurItem() const
{
	if (m_vecItem[m_iCurItemSel])
	{
		m_vecItem[m_iCurItemSel]->AddRef();
	}
	return m_vecItem[m_iCurItemSel];
}

bool Player::SellItem(int index)
{
	if (!m_vecItem[index] || m_vecItem[index]->IsToolItem())
		return false;

	m_iMoney += m_vecItem[index]->GetItemSellPrice();
	DecreaseItem(m_vecItem[index]);
	return true;
}

bool Player::SellItem(const string& itemTag)
{
	for (int i = 0; i < m_iMaxItemNum; ++i)
	{
		if (!m_vecItem[i]) continue;
		if (m_vecItem[i]->GetTag() == itemTag)
		{
			if (itemTag == "Ring")
			{
				m_bRing = false;
			}
			return SellItem(i);
		}
	}
	return false;
}

void Player::BuyItem(Item* pItem)
{
	if (!Affordable(pItem->GetPrice()) || IsFull())
	{
		return;
	}

	if (pItem->GetTag() == "Ring")
	{
		m_bRing = true;
	}
	m_iMoney -= pItem->GetPrice();
	AddItem(pItem);

	SOUND_MANAGER->PlayMusic("BuyItem");
}

Rect Player::BuildSwingAttack(int dx, int dy)
{
	Rect rect = {};
	if (dx > 0)
	{
		rect.left = 0;
		rect.top = -32.f - m_fAttackRange;
		rect.right = rect.left + m_fAttackRange;
		rect.bottom = rect.top + m_fAttackRange + 32.f;
	}
	else if (dx < 0)
	{
		rect.left = - m_fAttackRange;
		rect.top = -32.f - m_fAttackRange;
		rect.right = rect.left + m_fAttackRange;
		rect.bottom = rect.top + m_fAttackRange + 32.f;
	}
	else if (dy < 0)
	{
		rect.left = - m_fAttackRange;
		rect.top = -TILESIZE - m_fAttackRange;
		rect.right = rect.left + 2 * m_fAttackRange;
		rect.bottom = rect.top + TILESIZE + m_fAttackRange;
	}
	else if (dy >= 0)
	{
		rect.left = - m_fAttackRange;
		rect.top = -TILESIZE;
		rect.right = rect.left + 2 * m_fAttackRange;
		rect.bottom = rect.top  + TILESIZE + m_fAttackRange;
	}
	return rect;
}

void Player::PlayToolAnimation(const INDEX& index)
{
	bool bSwingTool = HasSwingTool();
	if (index.x > 0)
	{
		bSwingTool ? m_pAnimation->ChangeClip("SwingRight") :
			m_pAnimation->ChangeClip("ToolRight");
		StateTransit(IDLE_RIGHT);
	}
	else if (index.x < 0)
	{
		bSwingTool ? m_pAnimation->ChangeClip("SwingLeft") :
			m_pAnimation->ChangeClip("ToolLeft");
		StateTransit(IDLE_LEFT);
	}
	else if (index.y < 0)
	{
		bSwingTool ? m_pAnimation->ChangeClip("SwingUp") :
			m_pAnimation->ChangeClip("ToolUp");
		StateTransit(IDLE_UP);
	}
	else if (index.y >= 0)
	{
		bSwingTool ? m_pAnimation->ChangeClip("SwingDown") :
			m_pAnimation->ChangeClip("ToolDown");
		StateTransit(IDLE_DOWN);
	}
	m_pPlayerTool->Play();
}

void Player::MovePlayer(float dt)
{
	if (KEYPRESS("MoveUp"))
	{
		MoveYFromSpeed(dt, MD_BACK);
		StateTransit(WALK_UP);
	}
	else if (KEYPRESS("MoveDown"))
	{
		MoveYFromSpeed(dt, MD_FRONT);
		StateTransit(WALK_DOWN);
	}
	else if (KEYPRESS("MoveLeft"))
	{
		MoveXFromSpeed(dt, MD_BACK);
		StateTransit(WALK_LEFT);
	}
	else if (KEYPRESS("MoveRight"))
	{
		MoveXFromSpeed(dt, MD_FRONT);
		StateTransit(WALK_RIGHT);
	}
}

void Player::AfterMove()
{
	// ���� Ÿ���� �� �� ���ٸ�,
	const GameScene* gameScene = static_cast<GameScene*>(m_pScene);
	if (gameScene->IsBlockTile(GetCenterPos()))
	{
		SetPos(m_tPrev);
	}
	if (gameScene->GetSceneType() == SC_INHOUSE && SOUND_MANAGER->IsEnd(SD_EFFECT))
	{
		SOUND_MANAGER->PlayMusic("InHouse_Walking");
	}
}

void Player::AfterStop()
{
	// �������� �ʾҴٸ�,
	switch (m_eState)
	{
	case WALK_RIGHT:
		StateTransit(IDLE_RIGHT);
		break;
	case WALK_LEFT:
		StateTransit(IDLE_LEFT);
		break;
	case WALK_UP:
		StateTransit(IDLE_UP);
		break;
	case WALK_DOWN:
		StateTransit(IDLE_DOWN);
		break;
	}
}

void Player::ClickEventHandling()
{
	if (KEYDOWN("MouseLButton"))
	{
		COLLISION_MANAGER->ClickPoint();
	}
	if (KEYDOWN("MouseLButton") && IsIdleState())
	{
		Pos tMousePos = MOUSEWORLDPOS;
		Pos tCenterPos = GetCenterPos();
		Pos tPos = GetPos();
		const GameScene* gameScene = static_cast<GameScene*>(m_pScene);
		INDEX index = gameScene->GetIndexDiff(tMousePos, tCenterPos);
		if (max(abs(index.x), abs(index.y)) <= 1)
		{
			if (IsToolSelected())
			{
				if (HasEnoughMP())
				{
					UseMP();
					PlayToolAnimation(index);
					StateTransit(TOOL_USE);

					if (HasSwingTool())
					{
						// ��/�� üũ
						Rect attackRange = BuildSwingAttack(index.x, index.y);
						TRIGGER_RECTEVENT(GetCenterPos(), attackRange, "SwingTool");
						SOUND_MANAGER->PlayMusic("SwingTool");
					}
					else {
						// ����/���/ȣ�� üũ
						if (HasTool(PlayerTool::TOOL_AXE))
						{
							TRIGGER_CLICKEVENT(tMousePos, "AxeTool");
						}
						else if (HasTool(PlayerTool::TOOL_PICK))
						{
							TRIGGER_CLICKEVENT(tMousePos, "PickTool");
						}
						else if (HasTool(PlayerTool::TOOL_HOE))
						{
							static_cast<GameScene*>(m_pScene)->DigTile(tMousePos);
						}
						else if (HasTool(PlayerTool::TOOL_WATER))
						{
							static_cast<GameScene*>(m_pScene)->WaterTile(tMousePos);
						}
					}

				}
			}
			else if (IsSeedSelected())
			{
				static_cast<GameScene*>(m_pScene)->SpawnPlant(tMousePos);
			}
			TRIGGER_CLICKEVENT(tMousePos, "Harvest");
		}
	}
}

bool Player::HasEnoughMP() const
{
	return m_iMP >= 50.f;
}

void Player::UseMP()
{
	m_iMP -= m_iUseMPUnit;
}

void Player::StateTransit(int iNext)
{
	m_eState = static_cast<PlayerState>(iNext);
	switch (m_eState)
	{
	case IDLE_LEFT:
	case IDLE_RIGHT:
	case IDLE_UP:
	case IDLE_DOWN:
		m_bEnableAnimation = false;
		break;
	default:
		m_bEnableAnimation = true;
		break;
	}
	switch (m_eState)
	{
	case IDLE_LEFT:
		SetTexture("IdleLeft");
		break;
	case IDLE_RIGHT:
		SetTexture("IdleRight");
		break;
	case IDLE_UP:
		SetTexture("IdleUp");
		break;
	case IDLE_DOWN:
		SetTexture("IdleDown");
		break;
	case WALK_LEFT:
		m_pAnimation->ChangeClip("WalkLeft");
		break;
	case WALK_RIGHT:
		m_pAnimation->ChangeClip("WalkRight");
		break;
	case WALK_DOWN:
		m_pAnimation->ChangeClip("WalkDown");
		break;
	case WALK_UP:
		m_pAnimation->ChangeClip("WalkUp");
		break;
	}
}

bool Player::Init()
{
	SetPos(700.0f, 700.0f);
	SetSize(64.0f, 128.0f);
	SetPivot(0.0f, 1.0f);
	SetSpeed(m_fPlayerSpeed);

	InitTexture();
	InitAnimation();

	StateTransit(IDLE_RIGHT);
	EnableMovement();

	ColliderRect* pRC = AddCollider<ColliderRect>("PlayerBody");
	pRC->SetRect(10.f, -100.f, 50.f, -10.f);
	pRC->AddCollisionFunction(CS_ENTER, this, &Player::Hit);
	pRC->AddCollisionFunction(CS_STAY, this, &Player::Hit);

	SAFE_RELEASE(pRC);

	ColliderRect* pFoot = AddCollider <ColliderRect>("PlayerFoot");
	pFoot->SetRect(5.f, -55.f, 55.f, -5.f);
	pFoot->AddCollisionFunction(CS_ENTER, this, &Player::HitFoot);
	pFoot->AddCollisionFunction(CS_STAY, this, &Player::HitFoot);
	SAFE_RELEASE(pFoot);

	m_pPlayerTool->SetPlayer(this);

	SOUND_MANAGER->LoadSound("InHouse_Walking", false, SD_EFFECT, "InHouse_Walking.mp3");
	SOUND_MANAGER->LoadSound("SwingTool", false, SD_EFFECT, "SwingTool.mp3");
	SOUND_MANAGER->LoadSound("BuyItem", false, SD_EFFECT, "BuyItem.mp3");

	for (int i = 0; i < 10; ++i)
	{
		char name[10] = { 0 };
		sprintf_s(name, "Item%d", i);
		INPUT->AddKey(name, '0' + i);
	}

	INPUT->AddKey("Item-", char(0x2d));
	INPUT->AddKey("Item=", char(0x3d));

	Seed* pSeed = static_cast<Seed*>(Item::FindItem("Parsnip_Seed"));
	for (int i = 0; i < 10; ++i)
	{
		AddItem(pSeed);
	}
	SAFE_RELEASE(pSeed);
	return true;
}

void Player::Input(float dt)
{
	if (IsUsingTool())
	{
		return;
	}
	if (!HasEnoughMP())
	{
		SCENE_MANAGER->PlayerSleep();
	}
	MovableObject::Input(dt);
	ChangePlayerTool(dt);

	m_tPrev = GetPos();
	if (m_bMoveEnabled)
	{
		MovePlayer(dt);
	}
	if (m_bMove)
	{
		AfterMove();
	}
	else
	{
		AfterStop();
		ClickEventHandling();
	}
}

int Player::Update(float dt)
{
	MovableObject::Update(dt);
	m_pPlayerTool->Update(dt);
	return 0;
}

int Player::LateUpdate(float dt)
{
	MovableObject::LateUpdate(dt);
	m_pPlayerTool->LateUpdate(dt);
	return 0;
}

void Player::Collision(float dt)
{
	MovableObject::Collision(dt);
}

void Player::Draw(HDC hDC, float dt)
{
  	MovableObject::Draw(hDC, dt);

	m_pPlayerTool->Draw(hDC, dt);
	if (m_eState != TOOL_USE)
	{
		Item* pItem = GetCurItem();
		if (pItem && !pItem->IsToolItem())
		{
			Pos tPos = GetCenterPos();
			tPos -= CAMERA->GetTopLeft();
			tPos.x -= pItem->GetSize().x / 2;
			tPos.y -= GetSize().y + (pItem->GetSize().y) * 0.8f;
			pItem->DrawImageAt(hDC, tPos, true);
		}
		SAFE_RELEASE(pItem);
	}

#ifdef _DEBUG
	wchar_t playerPos[32] = {};
	swprintf_s(playerPos, L"Pos: %.1f, %.1f", GetPos().x, GetPos().y);
	Pos tPos = m_tPos - m_tSize * m_tPivot;
	tPos -= CAMERA->GetTopLeft();
	TextOut(hDC, int(tPos.x), int(tPos.y) - 10, playerPos, lstrlen(playerPos));

	INDEX rcIndex = static_cast<GameScene*>(m_pScene)->GetTileRowColIndex(MOUSEWORLDPOS);
	Object* pObj = m_pScene->FindObjectByIndex(rcIndex);
	if (pObj)
	{
		string str = "Object: " + pObj->GetTexTag();
		int length = (int) str.size();
		TextOut(hDC, int(MOUSECLIENTPOS.x), int(MOUSECLIENTPOS.y) + 20, GetWChar(str.c_str()), length);
	}
#endif
	if (SHOWCHECK(SHOW_TILEOPTION))
	{
		Pos tilePos = static_cast<GameScene*>(m_pScene)->GetTilePos(GetCenterPos());
		tilePos -= CAMERA->GetTopLeft();
		DrawVoidRectWithBorderColor(hDC, MakeRect(int(tilePos.x), int(tilePos.y), TILESIZE, TILESIZE),
			RGB(0, 200, 0));
	}
}

void Player::HitFoot(Collider* pSrc, Collider* pDst, float dt)
{
	if (pDst->GetTag() == "TileBlock")
	{
		SetPos(m_tPrev);
	}
}

void Player::Hit(Collider* pSrc, Collider* pDst, float dt)
{
	if (pDst->GetTag() == "ItemBody")
	{
		Item* pItem = static_cast<Item*>(pDst->GetObj());
		if (pItem->GetLife())
		{
			AddItem(pItem);
			SOUND_MANAGER->PlayMusic("ItemGet");
			pItem->Die();
		}
	}
}

void Player::Save(FILE* pFile)
{
}

void Player::Load(FILE* pFile)
{
}

void Player::ChangePlayerTool(float dt)
{
	if (KEYDOWN("Item1"))
	{
		m_iCurItemSel = 0;
	}
	else if (KEYDOWN("Item2"))
	{
		m_iCurItemSel = 1;
	}
	else if (KEYDOWN("Item3"))
	{
		m_iCurItemSel = 2;
	}
	else if (KEYDOWN("Item4"))
	{
		m_iCurItemSel = 3;
	}
	else if (KEYDOWN("Item5"))
	{
		m_iCurItemSel = 4;
	}
	else if (KEYDOWN("Item6"))
	{
		m_iCurItemSel = 5;
	}
	else if (KEYDOWN("Item7"))
	{
		m_iCurItemSel = 6;
	}
	else if (KEYDOWN("Item8"))
	{
		m_iCurItemSel = 7;
	}
	else if (KEYDOWN("Item9"))
	{
		m_iCurItemSel = 8;
	}
	else if (KEYDOWN("Item0"))
	{
		m_iCurItemSel = 9;
	}
	else if (KEYDOWN("Item-"))
	{
		m_iCurItemSel = 10;
	}
	else if (KEYDOWN("Item="))
	{
		m_iCurItemSel = 11;
	}

	if (m_vecItem[m_iCurItemSel] && m_vecItem[m_iCurItemSel]->IsToolItem())
	{
		m_pPlayerTool->SetTool(m_vecItem[m_iCurItemSel]);
	}
}

void Player::AddTool(Item* pItem)
{
	int index = FindEmptyIndex();
	if (index == -1) return;
	SAFE_RELEASE(m_vecItem[index]);
	m_vecItem[index] = pItem;
	m_vecItem[index]->AddRef();
}

Item* Player::FindItem(const string& itemTag)
{
	for (Item* pItem : m_vecItem)
	{
		if (pItem && pItem->GetTag() == itemTag)
		{
			pItem->AddRef();
			return pItem;
		}
	}
	return nullptr;
}

void Player::AddItem(Item* pItem)
{
	if (IsFull())
		return;

	Item* exist = FindItem(pItem->GetTag());
	if (!exist)
	{
		exist = pItem->Clone();
		exist->EraseAllColiders();
		m_vecItem[FindEmptyIndex()] = exist;
		return;
	}

	exist->Increase();
	SAFE_RELEASE(exist);
}

void Player::DecreaseItem(Item* pItem)
{
	auto iter = find(m_vecItem.begin(), m_vecItem.end(), pItem);
	if (iter != m_vecItem.end())
	{
		(*iter)->Decrease();
		if (!(*iter)->GetLife())
		{
			SAFE_RELEASE((*iter));
		}
	}
}

int Player::FindEmptyIndex() const
{
	for (int i = 0; i < m_iMaxItemNum; ++i)
	{
		if (m_vecItem[i] == nullptr)
			return i;
	}
	return -1;
}