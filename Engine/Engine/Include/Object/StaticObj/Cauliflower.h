#pragma once
#include "Plant.h"
class Cauliflower :
	public Plant
{
	friend class Object;
	static constexpr char m_plantTag[] = "Cauliflower";
protected:
	Cauliflower();
	virtual ~Cauliflower();
public:
	virtual bool Init();
	virtual void Input(float dt);
	virtual int Update(float dt);
	virtual int LateUpdate(float dt);
	virtual void Collision(float dt);
	virtual void Draw(HDC hDC, float dt);
	virtual void LateInit();
public:
	virtual void Save(FILE* pFile);
	virtual void Load(FILE* pFile);
};
