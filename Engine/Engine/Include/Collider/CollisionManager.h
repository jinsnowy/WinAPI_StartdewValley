#pragma once
#include "../framework.h"
#include "../Resources/Texture.h"
class CollisionManager
{
DECLARE_SINGLE(CollisionManager)
private:
	unique_ptr<Texture, Texture::Deleter> m_pCollTex;
	list <class Object*> m_tempCollisionObjList;
	list <class Object*> m_CollisionObjList;
	class CollisionSpace* m_CollisionSpace = nullptr;
	void NaiveAdd(class Object* const& pObj);
public:
	bool Init();
	void ClickPoint();
	void AddCollideObject(class Object* pObj);
	void AddCollideRect(const Pos& pos, const Rect& rect, const string& strTag);
	void AddCollidePoint(const Pos& pos, const string& strTag);
	void AddObject(class Object* pObj);

	void EraseCollider(class Collider* pColl);
	void QuadTreeUpdate();
	void SetUpCollisionSpace();
	void CollisionListVersion(float dt);
	void CollisionQuadTreeVersion(float dt);
	void Draw(HDC hdc, float dt);
	bool CheckCollision(class Object* pSrc, class Object* pDst, float dt);
	void DrawColliders(const vector<class Collider*> &colls, class Collider* pColl);
	void Clear();
};