#pragma once

#include <iostream>
#include <stdexcept>

namespace exqudens::other {

  class OtherTests {

    public:

      OtherTests() {
        std::cout << "OtherTests.CONSTRUCTOR" << std::endl;
      }

      void test1() {
        std::cout << "===" << std::endl;
        std::cout << "OtherTests.test1()" << std::endl;
        std::cout << "===" << std::endl;
      }

      void test2() {
        std::cout << "===" << std::endl;
        std::cout << "OtherTests.test2()" << std::endl;
        std::cout << "===" << std::endl;
        //throw std::runtime_error("AAA: aaa!!!");
      }

  };

}
