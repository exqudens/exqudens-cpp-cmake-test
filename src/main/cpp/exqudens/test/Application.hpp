#pragma once

#include <string>
#include <any>
#include <vector>
#include <set>
#include <map>
#include <tuple>
#include <functional>
#include <exception>
#include <stdexcept>
#include <iostream>

namespace exqudens::test {

  class Application {

    public:

      template <typename T>             using Vector              = std::vector<T>;
      template <typename T>             using Set                 = std::set<T>;
      template <typename K, typename V> using Map                 = std::map<K, V>;
      template <typename T>             using TestMethodReference = void(T::*)();

      using String       = std::string;
      using Any          = std::any;
      using TestMethod   = std::function<void()>;
      using Exception    = std::exception;
      using RuntimeError = std::runtime_error;

    private:

      inline static const String SHORT_OPTION_HELP = "-h";
      inline static const String SHORT_OPTION_TESTS = "-t";
      inline static const String LONG_OPTION_HELP = "--help";
      inline static const String LONG_OPTION_TESTS = "--tests";
      inline static const String VALUE_ALL_TESTS = "all";

    public:

      static int run(int argc, char** argv) {
        return Application(argc, argv).run();
      }

    private:

      Vector<String> commandLineArguments;
      Map<String, std::tuple<String, Any, TestMethod>> testMap;

    private:

      Application(int argc, char** argv) {
        for (int i = 0; i < argc; i++) {
          commandLineArguments.push_back(String(argv[i]));
        }
      }

      void initialize();

      std::tuple<bool, Set<String>> parseCommandLineArguments(Vector<String>& args) {
        bool helpRequested = false;
        Set<String> testNames;
        if (!commandLineArguments.empty()) {
          bool testNamesStarted = false;
          for (const String& commandLineArgument : commandLineArguments) {
            if (SHORT_OPTION_HELP == commandLineArgument || LONG_OPTION_HELP == commandLineArgument) {
              helpRequested = true;
              break;
            }
            if (SHORT_OPTION_TESTS == commandLineArgument || LONG_OPTION_TESTS == commandLineArgument) {
              testNamesStarted = true;
              continue;
            }
            if (testNamesStarted) {
              if (VALUE_ALL_TESTS == commandLineArgument) {
                for (auto& [key, value] : testMap) {
                  testNames.insert(key);
                }
                break;
              } else {
                testNames.insert(commandLineArgument);
                continue;
              }
            }
          }
        }
        return std::make_tuple(helpRequested, testNames);
      }

      int run() {
        using std::cout;
        using std::cerr;
        using std::endl;

        try {
          cout << "initialize start" << endl;
          initialize();
          cout << "initialize done" << endl;

          std::tuple<bool, Set<String>> config;
          config = parseCommandLineArguments(commandLineArguments);

          bool helpRequested = std::get<0>(config);
          Set<String> testNames = std::get<1>(config);

          if (helpRequested) {
            cout << "Usage: -h" << endl;
          } else {
            for (auto& [key, value] : testMap) {
              if (testNames.contains(key)) {
                Any testObject = std::get<1>(value);
                TestMethod testMethod = std::get<2>(value);
                testMethod();
              }
            }
          }

          return 0;
        } catch (Exception& e) {
          cerr << "ERROR: " << e.what() << endl;
          throw e;
        }
      }

      template <typename T>
      void addTest(TestMethodReference<T> testMethodReference, String testName) {
        if (testMap.contains(testName)) {
          String message;
          message += "Test map contains 'testName': ";
          message += testName;
          message += " (repeat not implemented yet!)";
          throw RuntimeError(message);
        }
        String testTypeName = typeid(T).name();
        Any testObject;
        for (auto const& [key, value] : testMap) {
          String currentTestTypeName = std::get<0>(value);
          if (testTypeName == currentTestTypeName) {
            testObject = std::get<1>(value);
            break;
          }
        }
        if (!testObject.has_value()) {
          testObject = T();
        }
        TestMethod testFunction = std::bind(testMethodReference, std::any_cast<T>(testObject));
        testMap[testName] = std::make_tuple(testTypeName, testObject, testFunction);
      }

      template <typename T, typename... ARGS>
      void addTests(TestMethodReference<T> testMethodReference, String testName, ARGS... args) {
        addTest(testMethodReference, testName);
        addTests(args...);
      }

      void addTests() {
      }

  };

}
