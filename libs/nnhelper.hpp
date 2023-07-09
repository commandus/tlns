#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <string>
#include <map>
#include <typeinfo>
#include <type_traits>


namespace nnhelper
{
	template <typename Ret, typename Fun, typename... Args>
	Ret run(Fun fun, Args... args) {
			std::cout << "run()" << std::endl;
			if(std::is_same<Ret, void>::value) {
				std::cout << "Ret is void" << std::endl;
				fun(args...);
			}
			else {
				std::cout << "Ret is NOT void" << std::endl;
				return fun(args...);
			}

	}

	template <typename Ret, typename Fun, typename... Args>
	typename std::enable_if<std::negation<std::is_same<Ret, void>>::value, Ret>::type
	zamer_vremeni(Fun fun, Args... args) {
	  std::cout << "zamer_vremeni()" << std::endl;

		auto begin = std::chrono::steady_clock::now();
	  Ret	ret = run<Ret>(fun, args...);
		auto end = std::chrono::steady_clock::now();
		auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
		std::cout << "delta: " << delta.count() << std::endl;
		return ret;
	}

	template <typename Ret, typename Fun, typename... Args>
	typename std::enable_if<std::is_same<Ret, void>::value, void>::type
	zamer_vremeni(Fun fun, Args... args) {
	  std::cout << "zamer_vremeni()" << std::endl;

		auto begin = std::chrono::steady_clock::now();
		//fun(args...);
	  run<Ret>(fun, args...);
		auto end = std::chrono::steady_clock::now();
		auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
		std::cout << "delta: " << delta.count() << std::endl;
	}

}

