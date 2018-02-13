#include "stdafx.h"
#include "Delay.h"
Delay *Delay::m_instance = 0;


Delay::Delay() :
	m_delay(DEFAULT_DELAY)
{
}


Delay::~Delay()
{
}

Delay* Delay::GetInstance()
{
	if (NULL == m_instance)
	{
		m_instance = new Delay;
	}
	return m_instance;
}

int Delay::GetDelay()
{
	return m_delay;
}

void Delay::SetDelay(int delay)
{
	m_delay = delay;
}
