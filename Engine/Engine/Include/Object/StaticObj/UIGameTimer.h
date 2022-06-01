#pragma once
#include "UI.h"
class UIGameTimer : public UI
{
private:
	friend class GameManager;
	class GameClock {
	private:
		float m_fTimeCur = 0.f;
		static constexpr float m_fMaxDarkness = 0.7f;
	public:
		bool m_bAfterNoon = false;
		int m_iHours = 6;
		int m_iMinutes = 0;
		int m_iDays = 1;
		int m_iWeekdays = 0;
		unsigned long long m_iWorldTime = 0;
	public:
		GameClock() {};
		GameClock(const GameClock&) = delete;
		~GameClock() {};
		bool Tick(float dt);
		bool IsAfterNoon() { return m_bAfterNoon; }
		int GetWeekDays() const { return m_iWeekdays; }
		int GetDays() const { return m_iDays; }
		int GetHours()const { return m_iHours; }
		int GetMinutes() const { return m_iMinutes; }
		float GetDayDarkness() const;
	};
public:
	UIGameTimer();
	UIGameTimer(const UIGameTimer& ui) = delete;
	virtual ~UIGameTimer();
private:
	virtual bool Init();
	virtual void Input(float dt);
	virtual int Update(float dt);
	virtual int LateUpdate(float dt);
	virtual void Collision(float dt);
	virtual void Draw(HDC hdc, float dt);
	virtual UIGameTimer* Clone() { throw EXCEPT(L"UIGameTimer cloned"); return nullptr; }
private:
	class UIPanel* m_pPlayerMPPanel = nullptr;
	static constexpr float m_iMPShakeStart = 0.25f;
	static constexpr int m_iMPPanelStartX = 13;
	static constexpr int m_iMPPanelStartY = 56;
	static constexpr float m_iMPPanelSizeX = 25.f;
	static constexpr float m_iMPPanelSizeY = 225.f;
private:
	static constexpr int m_iMoneyItemMargin = 6;
	static constexpr int m_iNumberWidth = 18;
	static constexpr int m_iMaxDigits = 8;
	static constexpr int m_iMoneyDrawStartX = 64;
	static constexpr int m_iMoneyDrawStartY = 192;
	static constexpr int m_iUpperDisplayStartY = 20;
	static constexpr float m_fUpperMargin = 20.f;

	static constexpr float m_fTimeUnit = TIME_UNIT; // ���� �ð� 10 �п� ���� �ð� �� ����
	static constexpr int m_iMaxHours = 26;
	static constexpr float interval = float(m_iMaxHours - 6) / 7.f;
	static constexpr float m_fMoneyPrintPeriod = 0.001f;
	int m_PlayerMoney = 0;
private:
	unique_ptr<GameClock> m_clock = make_unique<GameClock>();
	vector<class Texture*> m_vecNeedleTex;
	vector<class Texture*> m_vecMoneyTex;
	vector<class Texture*> m_vecWeekDays;
	vector<class Texture*> m_vecNoon;
	void SetNormalPos();
	void GameTimerFastForward() { m_clock->Tick(m_fTimeUnit); }
public:
	unsigned long long GetWorldTime() const;
	float GetDayDarkNess() const { return m_clock->GetDayDarkness(); }
	bool IsMorning() const { return m_clock->GetHours() == 5 && m_clock->GetMinutes() == 50; }
};