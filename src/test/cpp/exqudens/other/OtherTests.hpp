#pragma once

#include <iostream>
#include <stdexcept>

namespace exqudens::other {

  void testFunction1() {
    std::cout << "exqudens.other.testFunction1()" << std::endl;
  }

  void testFunction2() {
    std::cout << "exqudens.other.testFunction2()" << std::endl;
  }

  class OtherTests {

    public:

      OtherTests() {
        std::cout << "OtherTests.CONSTRUCTOR" << std::endl;
      }

      void testMethod1() {
        std::cout << "exqudens.other.OtherTests.testMethod1()" << std::endl;
      }

      void testMethod2() {
        std::cout << "exqudens.other.OtherTests.testMethod2()" << std::endl;
        //throw std::runtime_error("in 'exqudens.other.OtherTests.testMethod2'");
      }

  };

}
