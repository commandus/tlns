#include <iostream>
#include <vector>
#include <chrono>
#include <cstring>
#include <string>
#include <map>
#include <typeinfo>
#include <type_traits>

#include "nnhelper.hpp"


void fun1() {
	  std::cout << "fun1()" << std::endl;
}

void print_fun(char* str) {
	  std::cout << "print_fun(): " << str << std::endl;
}

int add_fun(int first, int second) {
	  std::cout << "add_fun(): " << std::endl;
		return first + second;
}


int main() {
    std::cout << "main" << std::endl;


		int i = nnhelper::run<decltype(add_fun(5, 7))>(&add_fun, 5, 7);
		std::cout << "i: " << i << std::endl;
		std::cout << std::endl;

		i = nnhelper::zamer_vremeni<decltype(add_fun(5, 7))>(&add_fun, 15, 17);
		std::cout << "i: " << i << std::endl;
		std::cout << std::endl;

		char* str = "asdadsasdasd";
		nnhelper::zamer_vremeni<decltype(print_fun(str))>(&print_fun, str);
		//nnhelper::run<decltype(print_fun(str))>(&print_fun, str);
		//std::cout << typeid(print_fun).name() << std::endl;

		//void k = 5;


    return 0;
}

