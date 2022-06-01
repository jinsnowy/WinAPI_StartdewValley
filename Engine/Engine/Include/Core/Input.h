#pragma once
#include "../framework.h"

class Input
{
	DECLARE_SINGLE(Input)
private:
	friend class Window;
	class KeyInfo
	{
		friend class Input;
	private:
		bool	bDown;  // ���� ó�� ������ �� (true, false, false)
		bool	bPress; // Ű ��� ������ �� (false, true, false)
		bool	bUp;    // Ű ���� �� (false, false, true)
		vector<DWORD> vecKey; // ���� Ű
		string strName;
		KeyInfo():
			bDown(false),
			bPress(false),
			bUp(false)
		{
		}
	};
private:
	unordered_map<char, bool> m_unCatchedKeyMap;
	HWND							m_hWnd;
	unordered_map<string, KeyInfo*> m_mapKey;
	KeyInfo*						m_pCreateKey;
	POINT							m_tMousePos;
	POINT							m_tMouseMove;
	vector<int> m_vecScroll;
	int m_iMouseScroll = 0;
	class Mouse* m_pMouse;
private:
	void SetUnCatchedKey(char c) { m_unCatchedKeyMap[c] = true; }
	void RegisterUnCatchedKey(char c) { m_unCatchedKeyMap.insert(make_pair(c, false)); }
	void InitUnCatchedKeyState();
	bool IsUnCatchedKey(char c) { return m_unCatchedKeyMap.find(c) != m_unCatchedKeyMap.end(); }
public:
	bool Init(HWND hWnd);
	void Update(float dt);
	bool KeyDown(const string& strKey) const;
	bool KeyPress(const string& strKey) const;
	bool KeyUp(const string& strKey) const;
	class Mouse* GetMouse() const { return m_pMouse; } 
	Pos GetMouseClientPos() const;
	Pos GetMouseWorldPos() const;
	Pos GetMouseMove() const;
	int GetMouseScroll() const { return m_iMouseScroll; }
	void AddMouseScroll(int scroll) { m_vecScroll.push_back(scroll / 120); }
public:
	template<typename T>
	bool AddKey(const T& data)
	{
		const char* pTType = typeid(T).name();
		if (strcmp(pTType, "char") == 0 || strcmp(pTType, "int") == 0)
		{
			m_pCreateKey->vecKey.push_back((DWORD)data);
		}
		else 
		{
			m_pCreateKey->strName = data;
			m_mapKey.insert(make_pair(m_pCreateKey->strName, m_pCreateKey));
		}
		return true;
	}

	template<typename T, typename... Types> // ���� ���� ���ø�
	bool AddKey(const T& data, const Types&... args)
	{
		if (!m_pCreateKey)
		{
			m_pCreateKey = new KeyInfo;
		}

		const char* pTType = typeid(T).name();
		if (strcmp(pTType, "char") == 0 || strcmp(pTType, "int") == 0)
		{
			m_pCreateKey->vecKey.push_back((DWORD)data);
		}
		else 
		{
			m_pCreateKey->strName = data;
			m_mapKey.insert(make_pair(m_pCreateKey->strName, m_pCreateKey));
		}
		AddKey(args...);

		if (m_pCreateKey)
			m_pCreateKey = nullptr;

		return true;
	}
	void DeleteKey(const string& strKey);
	KeyInfo* FindKey(const string& strKey) const;
};