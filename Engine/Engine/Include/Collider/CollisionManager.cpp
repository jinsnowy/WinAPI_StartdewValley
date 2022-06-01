#include "CollisionManager.h"
#include "../Scene/Scene.h"
#include "Collider.h"
#include "../Object/Object.h"
#include "../Object/PointObject.h"
#include "../Collider/ColliderPoint.h"
#include "../Collider/ColliderRect.h"
#include "../Resources/ResourceManager.h"
#include "../Scene/SceneManager.h"
#include "../Core/Input.h"
#include "../Core/Camera.h"
#include "../Scene/GameScene.h"
#include "CollisionSpace.h"
#include "../Application/Window.h"
#include "../utils.h"
#include "../Object/MoveObj/Player.h"
#include "../Object/StaticObj/GameManager.h"
#include "../Profiler.h"
#include "../Option.h"
#include <mutex>

int checkNum = 0;
float duration = 0.f;
mutex g_mtx;
DEFINITION_SINGLE(CollisionManager)

CollisionManager::CollisionManager()
    :
    m_pCollTex(Texture::MakeTexture())
{
    Texture* pTex = Texture::CreateEmptyTexture(WINDOW->GetWndDC(), GETRESOLUTION.x, GETRESOLUTION.y);
    m_pCollTex.reset(pTex);
    m_pCollTex->SetColorKey(util::Black);
}

CollisionManager::~CollisionManager()
{
}

bool CollisionManager::Init()
{
    return true;
}

void CollisionManager::ClickPoint()
{
    AddCollidePoint(MOUSEWORLDPOS, "Click");
}

void CollisionManager::AddCollideObject(Object* pObj)
{
    pObj->LateUpdate(0.f);
    m_tempCollisionObjList.push_back(pObj);
#ifdef QUAD_TREE
    const auto& collList = pObj->GetColliderList();
    for (auto iter = collList->begin(); iter != collList->end(); ++iter)
    {
        const auto& pColl = (*iter);
        if (pColl->GetEnable())
        {
            pColl->SetMoved(true);
            m_CollisionSpace->Observe(pColl);
        }
    }
#else
    m_CollisionObjList.push_back(pObj);
#endif
}

void CollisionManager::AddCollideRect(const Pos& pos, const Rect& rect, const string& strTag)
{
    Object* pPoint = Object::CreateObject<PointObject>(strTag);
    ColliderRect* pColl = pPoint->AddCollider<ColliderRect>(strTag);
    pColl->SetRect(rect.left, rect.top, rect.right, rect.bottom);
    pColl->SetMoved(true);
    pPoint->SetPos(pos.x, pos.y);
    pPoint->LateUpdate(0.f);
    m_tempCollisionObjList.push_back(pPoint);
#ifdef QUAD_TREE
    m_CollisionSpace->Observe(pColl);
#else
    m_CollisionObjList.push_back(pPoint);
#endif
    SAFE_RELEASE(pColl);
}

void CollisionManager::AddCollidePoint(const Pos& pos, const string& strTag)
{
    Object* pPoint = Object::CreateObject<PointObject>(strTag);
    ColliderPoint* pColl = pPoint->AddCollider<ColliderPoint>(strTag);
    pColl->SetMoved(true);
    pPoint->SetPos(pos.x, pos.y);
    pPoint->LateUpdate(0.f);
    m_tempCollisionObjList.push_back(pPoint);
#ifdef QUAD_TREE
    m_CollisionSpace->Observe(pColl);
#else
    m_CollisionObjList.push_back(pPoint);
#endif
    SAFE_RELEASE(pColl);
}

void CollisionManager::Clear()
{
    m_CollisionObjList.clear();

    // �� �����ӿ� �߰��� ��� �浹ü
    const auto iterEnd = m_tempCollisionObjList.end();
    for (auto iter = m_tempCollisionObjList.begin(); 
        iter != iterEnd; ++iter)
    {
        SAFE_RELEASE((*iter));
    }
    m_tempCollisionObjList.clear();
}

void CollisionManager::NaiveAdd(Object* const& pObj)
{
    m_CollisionObjList.push_back(pObj);
}

void CollisionManager::AddObject(Object* pObj)
{
    if (pObj->CheckCollider())
    {
#ifdef QUAD_TREE
        for (Collider* coll : *(pObj->GetColliderList()))
        {
            if (coll->GetEnable())
            {
                m_CollisionSpace->Observe(coll);
            }
        }
#else
        m_CollisionObjList.push_back(pObj);
#endif
    }
}

void CollisionManager::EraseCollider(Collider* pColl)
{
#ifdef QUAD_TREE
    m_CollisionSpace->ErasePreviousCollider(pColl);
#endif
}

void CollisionManager::QuadTreeUpdate()
{
    m_CollisionSpace->Update();
}

void CollisionManager::SetUpCollisionSpace()
{
    CollisionSpace::SetQuadTree();
    m_CollisionSpace = CollisionSpace::GetCurQuadTree();
}

void CollisionManager::CollisionListVersion(float dt)
{
    if (m_CollisionObjList.size() < 2)
    {
        Clear();
        return;
    }
    // �浹 üũ ���� üũ
    PROBE_PERFORMANCE("Collision/CollisionCheck");

    checkNum = 0;
    // ������Ʈ �� �浹 ó���� �Ѵ�.
    list<Object*>::iterator iter;
    list<Object*>::iterator iterEnd = m_CollisionObjList.end();
    --iterEnd;
   
    // ������Ʈ �������� �� �� ��
    for (iter = m_CollisionObjList.begin(); iter != iterEnd; ++iter)
    {
        list<Object*>::iterator jter = iter;
        ++jter;
        list<Object*>::iterator jterEnd = m_CollisionObjList.end();
        for (; jter != jterEnd; ++jter)
        {
            // �� ������Ʈ�� �ݶ��̾�鳢�� ��
            CheckCollision(*iter, *jter, dt);
        }
    }
    
    Clear();

    duration += dt;
    if (duration >= 1.0f)
    {
        duration -= 1.0f;
#ifdef _DEBUG
        _cprintf("collision check num : %d\n", checkNum);
#endif
        wchar_t message[256] = {};
        swprintf_s(message, L"collision check num : %d\n", checkNum);
        OutputDebugString(message);
    }

}

void CollisionManager::CollisionQuadTreeVersion(float dt)
{
    m_pCollTex->ClearBuffer(0, 0, GETRESOLUTION.x, GETRESOLUTION.y);

    int srcCollNum = m_CollisionSpace->GetCurSize();
    if (srcCollNum < 2)
    {
        Clear();
        return;
    }

    m_CollisionSpace->Ready();

    // �ݶ��̴� �����ֱ� On ������ ���
    if (SHOWCHECK(SHOW_COLL))
    {
        m_CollisionSpace->Draw(m_pCollTex->GetDC(), dt);
        const auto& pPlayerColl = m_CollisionSpace->FindCollider("PlayerBody");
        if (pPlayerColl)
        {
            vector<Collider*> pPlayerList;
            m_CollisionSpace->GetEqualSpaceColliders(pPlayerColl, pPlayerList);
            DrawColliders(pPlayerList, nullptr);
        }
    }

    // �浹 üũ ���� üũ
    checkNum = 0;
    PROBE_PERFORMANCE("Collision/CollisionCheck");

    const auto& pSrcList = *(m_CollisionSpace->GetColliderList());
    // �浹ü�� �浹ü �˻� ���� �κ�
    for (Collider* const& pSrc : pSrcList)
    {
        // �ش� ���� ��� �ִ� ���
        if (pSrc == nullptr)
            continue;

        // ��ü�� �������� �ʾҴٸ� üũ���� �ʴ´�.
        if (!pSrc->IsMoved())
            continue;

        // �浹ü �˻�
        vector<Collider*> pDstList;
        m_CollisionSpace->GetEqualSpaceColliders(pSrc, pDstList);

        // �浹������ ��� ���� �浹ü 
        for (Collider* const& pDst : pDstList)
        {

            // ���� �浹���� src�� �����Ǿ��� ��
            if (pSrc == nullptr)
                break;

            ++checkNum;

            // ���ߴٴ� ǥ�ø� ����
            m_CollisionSpace->Mark(pSrc, pDst);

            // �浹ü�� ���� �浹�ߴ°�
            if (pSrc->CheckCollision(pDst))
            {
                // ��Ʈ ����Ʈ ����
                pDst->SetHitPoint(pSrc->GetHitPoint());

                // ������ �浹���� �ʾ���
                if (!pSrc->CheckCollisionList(pDst))
                {
                    // ���� ������ �浹 ������� �߰��Ѵ�.
                    pSrc->AddCollider(pDst);
                    pDst->AddCollider(pSrc);

                    pSrc->CallFunction(CS_ENTER, pDst, dt);
                    pDst->CallFunction(CS_ENTER, pSrc, dt);
                }
                // �ѹ� �浹�� ���� ����
                else
                {
                    pSrc->CallFunction(CS_STAY, pDst, dt);
                    pDst->CallFunction(CS_STAY, pSrc, dt);
                }
            }
            // �浹���� �ʾ����� ���� �̷��� �ִ� ���
            else if (pSrc->CheckCollisionList(pDst))
            {
                // ���� �浹�� �ȵǹǷ� �浹��Ͽ��� �����ش�.
                pSrc->EraseCollisionList(pDst);
                pDst->EraseCollisionList(pSrc);
                // �浹���� ������ ����
                pSrc->CallFunction(CS_LEAVE, pDst, dt);
                pDst->CallFunction(CS_LEAVE, pSrc, dt);
            }
        }
    }

    Clear();

    duration += dt;
    if (duration >= 1.0f)
    {
        duration -= 1.0f;
#ifdef _DEBUG
        _cprintf("collision check num : %d\n", checkNum);
#endif
        wchar_t message[256] = {};
        swprintf_s(message, L"collision check num : %d\n", checkNum);
        OutputDebugString(message);
    }

}

void CollisionManager::Draw(HDC hdc, float dt)
{
    m_pCollTex->DrawImageAt(hdc, Pos(0.f, 0.f));
}

bool CollisionManager::CheckCollision(Object* pSrc, Object* pDst, float dt)
{

    const list<Collider*>* pSrcList = pSrc->GetColliderList();
    const list<Collider*>* pDstList = pDst->GetColliderList();
    const auto& iterSrcEnd = pSrcList->cend();
    const auto& iterDstEnd = pDstList->cend();

    bool bCollision = false;
    for (auto iterSrc = pSrcList->begin(); iterSrc != iterSrcEnd; ++iterSrc)
    {
        if (!(*iterSrc)->GetEnable())
        {
            continue;
        }
        for (auto iterDst = pDstList->begin(); iterDst != iterDstEnd; ++iterDst)
        {
            if (!(*iterDst)->GetEnable())
            {
                continue;
            }

            ++checkNum;

            // �浹ü�� ���� �浹�ߴ°�
            if ((*iterSrc)->CheckCollision(*iterDst))
            {
                bCollision = true;
                // ��Ʈ ����Ʈ ����
                ((*iterDst))->SetHitPoint((*iterSrc)->GetHitPoint());

                // �浹��Ͽ��� ������ �浹�� ���� ���ٸ�
                // ó�� �� �浹�Ǿ��ٴ� �ǹ�
                if (!(*iterSrc)->CheckCollisionList(*iterDst))
                {
                    // ���� ������ �浹 ������� �߰��Ѵ�.
                    (*iterSrc)->AddCollider(*iterDst);
                    (*iterDst)->AddCollider(*iterSrc);

                    (*iterSrc)->CallFunction(CS_ENTER, *iterDst, dt);
                    (*iterDst)->CallFunction(CS_ENTER, *iterSrc, dt);
                }
                // ���� �浹�� ���� �ִٸ� ��� �浹 ������ ���̴�.
                else
                {
                    (*iterSrc)->CallFunction(CS_STAY, *iterDst, dt);
                    (*iterDst)->CallFunction(CS_STAY, *iterSrc, dt);
                }
            }
            // ���� �浹���� �ʾҴٸ�,
            // ���� �浹�� �ȵ� ���¿��� ������ �浹�� �ǰ� �־�
            // �ٸ� ���� �� �浹���¿��� �������ٴ� �ǹ��̴�.
            else if ((*iterSrc)->CheckCollisionList(*iterDst))
            {
                // ���� �浹�� �ȵǹǷ� �浹��Ͽ��� �����ش�.
                (*iterSrc)->EraseCollisionList(*iterDst);
                (*iterDst)->EraseCollisionList(*iterSrc);

                (*iterSrc)->CallFunction(CS_LEAVE, *iterDst, dt);
                (*iterDst)->CallFunction(CS_LEAVE, *iterSrc, dt);
            }
        }
    }
    return bCollision;
}

void CollisionManager::DrawColliders(const vector<Collider*> &colls, Collider* pColl)
{
    if(SHOWCHECK(SHOW_COLL))
    {
        for (Collider* coll : colls)
        {
            Rect screenRect = coll->GetBounds().SubtractOffset(CAMERA->GetTopLeft());
            DrawColorRectWithOutBorder(m_pCollTex->GetDC(), screenRect, util::Magenta);
        }
        if (pColl)
        {
            Rect screenRect = pColl->GetBounds().SubtractOffset(CAMERA->GetTopLeft());
            DrawColorRectWithOutBorder(m_pCollTex->GetDC(), screenRect, util::Magenta);
        }
    }
}
