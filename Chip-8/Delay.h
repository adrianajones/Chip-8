#pragma once
class Delay
{
public:
	static Delay* GetInstance();
	~Delay();

	int GetDelay();
	void SetDelay(int delay);

private:
	static Delay* m_instance;
	Delay();
	int m_delay;

	static constexpr int DEFAULT_DELAY = 17;
};

