#pragma once
/*****************************************************************//**
 * \file   Singleton.h
 * \brief  单例类的基类
 * 
 * \author txy
 * \date   April 2025
 *********************************************************************/
#include"global.h"
//类的static变量一定要被初始化。不是模板类要放在cpp。是模板类放在.h
template<typename T>
class Singleton
{protected:
	Singleton() = default;
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator=(const Singleton<T>&) = delete;
	
	static std::shared_ptr<T> _instance;
public:  
	static std::shared_ptr<T> GetInstance()
	{
		static std::once_flag s_flag;
		std::call_once(s_flag, [&]() {
			//这里为什么不用std::make_shared<T> ()
			_instance = std::shared_ptr<T>(new T());
		});
		
		return _instance;
	}
	
	void PrintAddress() {
		std::cout << _instance.get() << std::endl;
	}
	//析构函数
	 ~Singleton()
	{
		 std::cout << "this is singleton destruct" << std::endl;
	}

};

template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;

