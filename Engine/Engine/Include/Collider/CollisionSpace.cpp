#include "CollisionSpace.h"
#include "Collider.h"
#include "../Core/Camera.h"
#include "../Scene/SceneManager.h"
#include "../Application/Window.h"
#include "../Resources/ResourceManager.h"
#include "../Core/Camera.h"
#include "../Object/Object.h"

unique_ptr<CollisionSpace> CollisionSpace::m_CurSpace = nullptr;

CollisionSpace::CollisionSpace()
{
}

void CollisionSpace::SetQuadTree()
{
	m_CurSpace = make_unique<CollisionSpace>();
	m_CurSpace->Init();
}

void CollisionSpace::Init()
{
	// ���� ��ǥ�� ����
	m_tWorldSpace = { 0.f, 0.f, 4096.f , 4096.f };

	// ��� �ʱ�ȭ
	m_QuadHead = QuadSpace::MakeQuadPtr(0, 0, m_tWorldSpace);
	m_mapSpace.insert(make_pair(0, m_QuadHead.get()));

	// �ݶ��̴��� �Ҵ��� Id ť �ʱ�ȭ
	for (int id = 0; id < m_iExpectedCollNum; ++id)
	{
		m_IdQueue.push(id);
	}

	// �浹 üũ ��Ʈ���� �ʱ�ȭ
	m_CheckMat.resize(m_iExpectedCollNum);
	fill(m_CheckMat.begin(), m_CheckMat.end(), vector<bool>(m_iExpectedCollNum, false));

	// �浹ü ��� �����̳�
	m_ColliderContainer.resize(m_iExpectedCollNum);
	fill(m_ColliderContainer.begin(), m_ColliderContainer.end(), nullptr);
}

CollisionSpace::~CollisionSpace()
{
	for (Collider*& pColl : m_ColliderContainer)
	{
		if (pColl)
		{
			pColl->SetId(-1);
			pColl->SetSpaceId(-1);
		}
	}
	m_ColliderContainer.clear();
}

void CollisionSpace::QuadSpace::GetChildCollidersNum(size_t* const sz)
{
	for (int i = 0; i < 4; ++i)
	{
		if (m_QuadPartitions[i])
		{
			*sz += m_QuadPartitions[i]->GetColliderNum();
			m_QuadPartitions[i]->GetChildCollidersNum(sz);
		}
	}
}

void CollisionSpace::Ready()
{
	// üũ ��� �ʱ�ȭ
	auto curSize = m_CheckMat.size();
	fill(m_CheckMat.begin(), m_CheckMat.end(), vector<bool>(curSize, false));
}

void CollisionSpace::Update()
{
	m_tCameraSpace = CAMERA->GetWorldRect();
}

void CollisionSpace::Observe(Collider* pColl)
{
	// ���� Ʈ���� �ִ� �ݶ��̴� �� ���
	if (pColl->GetId() != -1)
	{
		if (!pColl->IsMoved())
			return;

		ErasePreviousCollider(pColl);
	}
	// ID ť���� ����
	int nxt_id = m_IdQueue.top();
	m_IdQueue.pop();

	// ��ü �ݶ��̴� �����̳ʿ� �Ҵ�
	++m_CurSize;
	pColl->SetId(nxt_id);
	m_ColliderContainer[nxt_id] = pColl;
	m_QuadHead->Insert(pColl);

	if (m_IdQueue.size() == 0)
	{
		ExpandId();
	}
}

void CollisionSpace::Mark(Collider* pSrc, Collider* pDst)
{
	int srcId = pSrc->GetId(), dstId = pDst->GetId();
	m_CheckMat[srcId][dstId] = m_CheckMat[dstId][srcId] = true;
}

void CollisionSpace::ErasePreviousCollider(Collider* pColl)
{
	int collId = pColl->GetId();
	if (collId == -1)
		return;

	// ���� ���� Ʈ�� �������� ����
	int spaceId = pColl->GetSpaceId();
	const QuadParentPtr& quad = FindSpace(spaceId);
	list<Collider*>& collList = quad->m_CollList;
	list<Collider*>::iterator iterEnd = collList.end();
	for (auto iter = collList.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter) == pColl)
		{
			// ��ü �ݶ��̴� �����̳ʿ��� ����
			--m_CurSize;
			m_ColliderContainer[collId] = nullptr;
			pColl->SetId(-1);

			// Id ť�� �߰�
			m_IdQueue.push(collId);

			pColl->SetSpaceId(-1);
			collList.erase(iter);
			break;
		}
	}

	// �ʿ�� ���� ������
	if (spaceId != 0)
	{
		int parentId = (spaceId - 1) / 4;
		QuadParentPtr parentQuad = FindSpace(parentId);
		
		// �ݵ�� �־����
		assert(parentQuad);

		// �θ� ������ �ִ� �ݶ��̴�
		auto& parentColliders = parentQuad->m_CollList;
		size_t totalSpaceCollNum = parentColliders.size();

		// �θ� �ִ� �ݶ��̴��� �ڽĿ� �ִ� �ݶ��̴� ���� ���Ѵ� -> O(V), ��� ����� �ƴ�
		parentQuad->GetChildCollidersNum(&totalSpaceCollNum);

		// �ٽ� ��ģ��.
		if (totalSpaceCollNum <= m_iMaxObjectNum && parentQuad->m_QuadPartitions[0])
		{
			parentQuad->Merge(parentId, parentQuad->m_CollList);
		}
	}
}

void CollisionSpace::QuadSpace::Merge(int parentId, list<Collider*> &parentColliders)
{
	// �ݶ��̴� ���� id ������ �� �θ� ������ �߰�
	const auto adjust_collider = [&parentColliders, &parentId](Collider* const& pColl)
	{
		pColl->SetSpaceId(parentId);
		parentColliders.push_back(pColl);
	};

	auto& mapSpace = m_CurSpace->m_mapSpace;
	for (int i = 0; i < 4; ++i)
	{
		if (m_QuadPartitions[i])
		{
			const auto& childColliders = m_QuadPartitions[i]->m_CollList;
			for_each(childColliders.begin(), childColliders.end(), adjust_collider);

			// ���� Ʈ�� �պ�
			m_QuadPartitions[i]->Merge(parentId, parentColliders);
			m_QuadPartitions[i] = nullptr;

			// �ؽ��ʿ��� ����
			int childId = m_iIdx * 4 + i + 1;
			mapSpace.erase(childId);
		}
	}
}

void CollisionSpace::QuadSpace::Insert(Collider* const& pColl)
{
	// �̹� ���ҵ� ������ �ִ� ���
	if (m_QuadPartitions[(int)Partition::LEFT_TOP])
	{
		// ������ ���� ��� ���� ������ ����
		const Rect& bounds = pColl->GetBounds();
		bool tooSmall = m_tArea.right - m_tArea.left < m_fMinSize;
		Partition ePart = tooSmall ? Partition::EQUAL_AREA : GetPartition(bounds);

		// ���� �������� ���
		if (ePart != Partition::EQUAL_AREA)
		{
			m_QuadPartitions[(int)ePart]->Insert(pColl);
			return;
		}
	}

	// �浹ü �߰�
	AddCollider(pColl);

	// �ʹ� ���� �浹ü�� ���� �ִٸ� ���� ����
	if (IsOverLoaded())
	{
		SplitArea();

		// �浹ü�� ���� �������� ����
		const auto& iterEnd = m_CollList.end();
		for (auto iter = m_CollList.begin(); iter != iterEnd;)
		{
			const Rect& bounds = (*iter)->GetBounds();
			Partition ePart = GetPartition(bounds);

			if (ePart != Partition::EQUAL_AREA)
			{
				m_QuadPartitions[(int)ePart]->Insert((*iter));
				iter = m_CollList.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}
}

void CollisionSpace::GetEqualSpaceColliders(Collider* const &pSrc, vector<Collider*>& dstColliders)
{
	m_QuadHead->Search(pSrc, dstColliders);
}

void CollisionSpace::QuadSpace::Draw(HDC& hdc, const float& dt)
{
	Rect screenRect = m_tArea.SubtractOffset(CAMERA->GetTopLeft());
	DrawVoidRectWithBorderColor(hdc, screenRect, util::Green);

	for (int i = 0; i < 4; ++i)
	{
		if (m_QuadPartitions[i])
		{
			m_QuadPartitions[i]->Draw(hdc, dt);
		}
	}
}

void CollisionSpace::Draw(HDC hdc, float dt)
{
	m_QuadHead->Draw(hdc, dt);
}

void CollisionSpace::ExpandId()
{
	auto curSize = m_CheckMat.size();

	// üũ ��Ʈ���� Ȯ��
	m_CheckMat.resize(2 * curSize);
	fill(m_CheckMat.begin(), m_CheckMat.end(), vector<bool>(2 * curSize, false));

	// �浹ü �����̳� Ȯ��
	for (int i = 0; i < (int)curSize; ++i)
		m_ColliderContainer.push_back(nullptr);

	// �浹ü ID ť Ȯ��
	for (int i = (int)curSize; i <(int) 2 * curSize; ++i)
		m_IdQueue.push(i);
}

Rect CollisionSpace::QuadSpace::MakeArea(Partition ePart) const
{
	float CenterX = (m_tArea.left + m_tArea.right) / 2.f;
	float CenterY = (m_tArea.top + m_tArea.bottom) / 2.f;

	Rect area = {};
	switch (ePart)
	{
	case CollisionSpace::QuadSpace::Partition::LEFT_TOP:
		area.left = m_tArea.left;
		area.top = m_tArea.top;
		area.right = CenterX;
		area.bottom = CenterY;
		break;
	case CollisionSpace::QuadSpace::Partition::LEFT_BOTTOM:
		area.left = m_tArea.left;
		area.top = CenterY;
		area.right = CenterX;
		area.bottom = m_tArea.bottom;
		break;
	case CollisionSpace::QuadSpace::Partition::RIGHT_TOP:
		area.left = CenterX;
		area.top = m_tArea.top;
		area.right = m_tArea.right;
		area.bottom = CenterY;
		break;
	case CollisionSpace::QuadSpace::Partition::RIGHT_BOTTOM:
		area.left = CenterX;
		area.top = CenterY;
		area.right = m_tArea.right;
		area.bottom = m_tArea.bottom;
		break;
	}
	return area;
}

void CollisionSpace::QuadSpace::SplitArea()
{
	for (int i = 0; i < 4; ++i)
	{
		int idx = 4 * m_iIdx + i + 1;
		m_QuadPartitions[i] = MakeQuadPtr(m_iLevel + 1, idx, MakeArea(static_cast<Partition>(i)));
		m_CurSpace->m_mapSpace.insert(make_pair(idx, m_QuadPartitions[i].get()));
	}
}

void CollisionSpace::QuadSpace::Search(Collider* const& pSrc, vector<Collider*>& dstColliders)
{
	const int& srcId = pSrc->GetId();
	const auto& checkMat = m_CurSpace->m_CheckMat;

	// ���� ������Ʈ���� �ݶ��̴��� �ƴϸ鼭 �ݶ��̴� üũ�� ���� ���� �ֵ� 
	const auto InsertByNotEqObjAndNotChecked = [&](Collider* const& pDst)
	{
		const int& dstId = pDst->GetId();
		bool checked = checkMat[srcId][dstId] || checkMat[dstId][srcId];
		if (pSrc->GetObj() != pDst->GetObj() && !checked)
		{
			dstColliders.push_back(pDst);
		}
	};

	for_each(m_CollList.begin(), m_CollList.end(), InsertByNotEqObjAndNotChecked);

	// ��Ƽ�� �и��� �Ǿ� ���� ��
	if (m_QuadPartitions[0])
	{
		const Rect& bounds = pSrc->GetBounds();
		for (int i = 0; i < 4; ++i)
		{
			if (m_QuadPartitions[i]->m_tArea.IsCollideRect(bounds))
			{
				m_QuadPartitions[i]->Search(pSrc, dstColliders);
			}
		}
	}
}


CollisionSpace::QuadParentPtr CollisionSpace::FindSpace(int id)
{
	if (id == -1)
		return nullptr;

	auto found = m_mapSpace.find(id);
	if (found == m_mapSpace.end())
		return nullptr;
	return found->second;
}

bool CollisionSpace::QuadSpace::OutSideOfScreen(Collider* pColl)
{
	if (pColl->IsUICollider())
	{
		// UI�� ��� Screen ���� �󿡼� ���
		const Rect& rect = pColl->GetBounds();
		if (rect.right < 0) return true;
		if (rect.left >= GETRESOLUTION.x) return true;
		if (rect.bottom < 0) return true;
		if (rect.top >= GETRESOLUTION.y) return true;
		return false;
	}

	// �Ϲ� ������Ʈ�� World ���� �󿡼� ���
	const Rect& rect = pColl->GetBounds();
	const Rect& camSpace = m_CurSpace->m_tCameraSpace;
	if (rect.right < camSpace.left) return true;
	if (rect.left >= camSpace.right) return true;
	if (rect.bottom < camSpace.top) return true;
	if (rect.top >= camSpace.bottom) return true;
	return false;
}

CollisionSpace::QuadPtr CollisionSpace::QuadSpace::MakeQuadPtr(unsigned int level, int idx, const Rect& rect)
{
	return make_unique<QuadSpace>(level, idx, rect);
}

CollisionSpace::QuadSpace::QuadSpace(unsigned int level, int idx, const Rect& rect)
	:
	m_iLevel(level),
	m_iIdx(idx),
	m_tArea(rect)
{
}
bool CollisionSpace::QuadSpace::IsOverLoaded() const
{
	return m_CollList.size() > m_iMaxObjectNum && m_QuadPartitions[0] == nullptr
		&& (m_tArea.right - m_tArea.left) / 2.f >= m_fMinSize;
}

void CollisionSpace::QuadSpace::AddCollider(Collider* const& pColl)
{
	pColl->SetSpaceId(m_iIdx);
	m_CollList.push_back(pColl);
}

CollisionSpace::QuadSpace::Partition CollisionSpace::QuadSpace
::GetPartition(const Rect& bounds)
{
	float CenterX = (m_tArea.left + m_tArea.right) / 2.f;
	float CenterY = (m_tArea.top + m_tArea.bottom) / 2.f;

	// Left
	if (bounds.right < CenterX)
	{
		// Top
		if (bounds.bottom < CenterY)
		{
			return Partition::LEFT_TOP;
		}
		// Bottom
		else if(bounds.top > CenterY)
		{
			return Partition::LEFT_BOTTOM;
		}
	}
	// Right
	else if(bounds.left > CenterX)
	{
		// Top
		if (bounds.bottom < CenterY)
		{
			return Partition::RIGHT_TOP;
		}
		// Bottom
		else if(bounds.top > CenterY)
		{
			return Partition::RIGHT_BOTTOM;
		}
	}

	return Partition::EQUAL_AREA;
}

Collider* CollisionSpace::FindCollider(const string& strTag)
{
	const auto& iterEnd = m_ColliderContainer.end();
	for (auto iter = m_ColliderContainer.begin(); iter != iterEnd; ++ iter)
	{
		if ((*iter) == nullptr) continue;
		if ((*iter)->GetTag() == strTag)
		{
			return (*iter);
		}
	}
	return nullptr;
}

