#include "exqudens/test/Application.hpp"
#include "exqudens/other/OtherTests.hpp"

void exqudens::test::Application::initialize() {
  addTests<exqudens::other::OtherTests>(
      &exqudens::other::OtherTests::test1, "exqudens.other.OtherTests.test1",
      &exqudens::other::OtherTests::test2, "exqudens.other.OtherTests.test2"
  );
}

int main(int argc, char** argv) {
  return exqudens::test::Application::run(argc, argv);
}
