#include "Stage.h"
#include "Tile.h"
#include "Tree.h"
#include "../../Resources/Texture.h"
#include "../../Resources/ResourceManager.h"
#include "../../framework.h"
#include "../../Core/Camera.h"
#include "../../Scene//Layer.h"
#include "../../Scene/Scene.h"
#include "../../Application/Window.h"
#include "../../Core/PathManager.h"
#include "../../Scene/SceneManager.h"

Stage::Stage()
{
}

Stage::Stage(const Stage& stage)
    :
    StaticObject(stage)
{
    ClearTile();

    for (size_t i = 0; i < stage.m_baseTile.size(); ++i)
    {
        m_baseTile.push_back(stage.m_baseTile[i]->Clone());
    }
}

Stage::~Stage()
{
    ClearTile();
}



void Stage::CreateTile(int iNumX, int iNumY)
{
    Safe_Release_VecList(m_baseTile);

    m_iTileNumX = iNumX;
    m_iTileNumY = iNumY;

    Pos offset = Pos();
    for (int i = 0; i < iNumY; ++i)
    {
        for (int j = 0; j < iNumX; ++j)
        {
            Tile* pTile = Object::CreateObject<Tile>("Tile");
            
            offset.x = (float) j * TILESIZE;
            offset.y = (float) i * TILESIZE;

            pTile->SetSize(TILESIZE, TILESIZE);
            pTile->SetPos(offset.x, offset.y);
            pTile->SetPivot(0.f, 0.0f);
            m_baseTile.push_back(pTile);
        }
    }
}

Tile* Stage::GetTile(int index)
{
    m_baseTile[index]->AddRef();
    return m_baseTile[index]; 
}

bool Stage::IsBlockTile(const Pos& worldPos) const
{
    int index = GetTileIndex(worldPos);
    if (index == -1) return false;
    return IsBlockTile(index);
}

bool Stage::IsBlockTile(int index) const
{
    return m_baseTile[index]->m_eOption == TO_NOMOVE;
}

bool Stage::Init()
{
    return true;
}

void Stage::Input(float dt)
{
    StaticObject::Input(dt);
}

int Stage::Update(float dt)
{
    StaticObject::Update(dt);
    return 0;
}

int Stage::LateUpdate(float dt)
{
    StaticObject::LateUpdate(dt);
    return 0;
}

void Stage::Collision(float dt)
{
    StaticObject::Collision(dt);
}

void Stage::Draw(HDC hDC, float dt)
{
    for (size_t i = 0; i < m_baseTile.size(); ++i)
    {
        m_baseTile[i]->Draw(hDC, dt);
    }

#ifdef _DEBUG
    if (SHOWCHECK(SHOW_GRID))
    {
        // Grid?? ??????.
        Pos tCamPos = CAMERA->GetTopLeft();
        for (int i = 1; i <= m_iTileNumY; ++i)
        {
            // ???????? ??????.
            MoveToEx(hDC, 0, i * TILESIZE - tCamPos.y, NULL);
            LineTo(hDC, m_iTileNumX * TILESIZE - tCamPos.x, i * TILESIZE - tCamPos.y);
        }

        // ???????? ??????.
        for (int i = 1; i <= m_iTileNumX; ++i)
        {
            MoveToEx(hDC, i * TILESIZE - tCamPos.x, 0, NULL);
            LineTo(hDC, i * TILESIZE - tCamPos.x, m_iTileNumY * TILESIZE - tCamPos.y);
        }
    }
#endif
}

Stage* Stage::Clone()
{
    return new Stage(*this);
}


void Stage::Save(FILE* pFile)
{
    StaticObject::Save(pFile);

    // ???????? ???? ????
    fwrite(&m_iTileNumX, 4, 1, pFile);
    fwrite(&m_iTileNumY, 4, 1, pFile);

    for (size_t i = 0; i < m_baseTile.size(); ++i)
    {
        m_baseTile[i]->Save(pFile);
    }
}

void Stage::Load(FILE* pFile)
{
    StaticObject::Load(pFile);

    // ???????? ???? ????
    fread(&m_iTileNumX, 4, 1, pFile);
    fread(&m_iTileNumY, 4, 1, pFile);

    ClearTile();

    for (int i = 0; i < m_iTileNumX * m_iTileNumY; ++i)
    {
        Tile* pTile = Object::CreateObject<Tile>("Tile");

        pTile->Load(pFile);

        m_baseTile.push_back(pTile);
    }
}

TILE_OPTION Stage::GetTileOption(const Pos& pos) const
{
    int ind = GetTileIndex(pos);
    if (ind == -1)
        return TO_NONE;
    return m_baseTile[ind]->m_eOption;
}

string Stage::GetTileName(const Pos& pos) const
{
    int ind = GetTileIndex(pos);
    if (ind == -1)
        return "";

    if(m_baseTile[ind]->m_pTexture)
        return m_baseTile[ind]->m_pTexture->GetTag();
    return "";
}

void Stage::SetTileNone(const Pos& tPos)
{
    int ind = GetTileIndex(tPos);

    if (ind == -1)
        return;

    SAFE_RELEASE(m_baseTile[ind]);
    Pos offset = GetTilePos(ind);

    m_baseTile[ind] = Object::CreateObject<Tile>("Tile");
    m_baseTile[ind]->SetSize(TILESIZE, TILESIZE);
    m_baseTile[ind]->SetPos(offset.x, offset.y);
    m_baseTile[ind]->SetPivot(0.f, 0.0f);
}

void Stage::ChangeTileByCloneTile(const Pos& tPos, Tile* pClone)
{
    int ind = GetTileIndex(tPos);

    if (ind == -1)
        return;

    SAFE_RELEASE(m_baseTile[ind]);

    m_baseTile[ind] = pClone;
    m_baseTile[ind]->AddRef();
}


void Stage::ChangeTileOption(const Pos& tPos, TILE_OPTION eOption)
{
    int ind = GetTileIndex(tPos);

    if (ind == -1)
        return;

    m_baseTile[ind]->SetTileOption(eOption);
}


void Stage::ClearTile()
{
    Safe_Release_VecList(m_baseTile);
}