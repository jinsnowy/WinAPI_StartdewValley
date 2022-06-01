#include "Seed.h"
#include "../StaticObj/Parsnip.h"
#include "../StaticObj/Garlic.h"
#include "../StaticObj/Cauliflower.h"
#include "../StaticObj/Radish.h"
#include "../StaticObj/BlueBerry.h"
#include "../StaticObj/Tomato.h"
#include "../StaticObj/Pepper.h"
Seed::Seed()
{
	m_eItemType = ITEM_SEED;
}

Seed::Seed(const Seed& item)
	:Item(item)
{
	m_strPlantTag = item.m_strPlantTag;
}

Seed::~Seed()
{
}

bool Seed::Init()
{
	return true;
}

void Seed::Input(float dt)
{
	Item::Input(dt);
}

int Seed::Update(float dt)
{
	Item::Update(dt);
	return 0;
}

int Seed::LateUpdate(float dt)
{
	Item::LateUpdate(dt);
	return 0;
}

void Seed::Collision(float dt)
{
	Item::Collision(dt);
}

void Seed::Draw(HDC hdc, float dt)
{
	Item::Draw(hdc, dt);
}

Seed* Seed::Clone()
{
	return new Seed(*this);
}

Plant* Seed::SpawnPlant() const
{
	if (m_strPlantTag == "Parsnip_Plant")
	{
		return Object::CreateObject<Parsnip>(m_strPlantTag);
	}
	if (m_strPlantTag == "Cauliflower_Plant")
	{
		return Object::CreateObject<Cauliflower>(m_strPlantTag);
	}
	if (m_strPlantTag == "Garlic_Plant")
	{
		return Object::CreateObject<Garlic>(m_strPlantTag);
	}
	if (m_strPlantTag == "Pepper_Plant")
	{
		return Object::CreateObject<Pepper>(m_strPlantTag);
	}
	if (m_strPlantTag == "Radish_Plant")
	{
		return Object::CreateObject<Radish>(m_strPlantTag);
	}
	if (m_strPlantTag == "Tomato_Plant")
	{
		return Object::CreateObject<Tomato>(m_strPlantTag);
	}
	if (m_strPlantTag == "BlueBerry_Plant")
    {
		return Object::CreateObject<BlueBerry>(m_strPlantTag);
    }
	return nullptr;
}
