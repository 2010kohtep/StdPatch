#pragma once

template <typename T> class CArrayHelper
{
private:
	T *m_pData;
public:
	CArrayHelper<T>()
	{
		m_pData = nullptr;
	}

	~CArrayHelper()
	{
		if (m_pData)
			free(m_pData);
	}

	void SetLength(int nNewLength)
	{
		if (m_pData)
		{
			free(m_pData);
			m_pData = nullptr;
		}

		if (nNewLength > 0)
		{
			auto nSize = nNewLength * sizeof(*m_pData);
			m_pData = (T *)malloc(nSize);
			memset(m_pData, 0, nSize);
		}
		else
		{
			m_pData = nullptr;
		}
	}

	inline T *GetData() { return m_pData; }
};