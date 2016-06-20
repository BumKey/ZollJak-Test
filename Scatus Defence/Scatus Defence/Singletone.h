#pragma once

template <class T>
class Singletone
{
protected:
	Singletone() {}
	virtual ~Singletone() {}

public:
	static T*		GetInstance() { return mInstance; }

private:
	static T* mInstance;
};

template <class T> T* Singletone<T>::mInstance = new T();