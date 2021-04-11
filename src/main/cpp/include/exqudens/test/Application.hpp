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

    private:

      template <typename T>             using Vector              = std::vector<T>;
      template <typename T>             using Set                 = std::set<T>;
      template <typename K, typename V> using Map                 = std::map<K, V>;
      template <typename T>             using TestMethodReference = void(T::*)();

      using String                = std::string;
      using Any                   = std::any;
      using TestFunctionReference = void(*)();
      using TestMethod            = std::function<void()>;
      using Exception             = std::exception;
      using RuntimeError          = std::runtime_error;

    private:

      inline static const String SHORT_OPTION_HELP = "-h";
      inline static const String SHORT_OPTION_TESTS = "-t";
      inline static const String SHORT_OPTION_FAIL_FAST = "-ff";
      inline static const String LONG_OPTION_HELP = "--help";
      inline static const String LONG_OPTION_TESTS = "--tests";
      inline static const String LONG_OPTION_FAIL_FAST = "--fail-fast";
      inline static const String VALUE_ALL_TESTS = "all";

    public:

      static int run(
          int argc,
          char** argv,
          bool addHeader = true,
          bool addFooter = true,
          bool addNewLine = true,
          char delimiterChar = '=',
          int delimiterSize = 10,
          bool addSpace = true,
          bool addActionName = true
      ) {
        return Application(
            argc,
            argv,
            addHeader,
            addFooter,
            addNewLine,
            delimiterChar,
            delimiterSize,
            addSpace,
            addActionName
        ).run();
      }

    private:

      Vector<String> commandLineArguments;
      Map<String, std::tuple<String, Any, TestMethod>> testMap;
      bool addHeader;
      bool addFooter;
      bool addNewLine;
      char delimiterChar;
      int delimiterSize;
      bool addSpace;
      bool addActionName;

    private:

      Application(
          int argc,
          char** argv,
          bool addHeader,
          bool addFooter,
          bool addNewLine,
          char delimiterChar,
          int delimiterSize,
          bool addSpace,
          bool addActionName
      ) {
        for (int i = 0; i < argc; i++) {
          commandLineArguments.push_back(String(argv[i]));
        }
        this->addHeader = addHeader;
        this->addFooter = addFooter;
        this->addNewLine = addNewLine;
        this->delimiterChar = delimiterChar;
        this->delimiterSize = delimiterSize;
        this->addSpace = addSpace;
        this->addActionName = addActionName;
      }

      void initialize();

      std::tuple<bool, bool, Set<String>> parseCommandLineArguments(Vector<String>& args) {
        bool helpRequested = false;
        bool failFast = false;
        Set<String> testNames;
        if (!commandLineArguments.empty()) {
          bool testNamesStarted = false;
          for (const String& commandLineArgument : commandLineArguments) {
            if (SHORT_OPTION_HELP == commandLineArgument || LONG_OPTION_HELP == commandLineArgument) {
              helpRequested = true;
              break;
            }
            if (SHORT_OPTION_FAIL_FAST == commandLineArgument || LONG_OPTION_FAIL_FAST == commandLineArgument) {
              failFast = true;
              continue;
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
        return std::make_tuple(helpRequested, failFast, testNames);
      }

      int runTest(Any object, TestMethod function, String name) {
        using std::cout;
        using std::cerr;
        using std::endl;

        int result;

        String header = String();
        String footer = String();

        if (addHeader) {
          header.append(delimiterSize, delimiterChar);
          if (addSpace) {
            header.append(1, ' ');
          }
          if (addActionName) {
            header.append("run test:");
            header.append(1, ' ');
          }
          header.append(name);
          if (addSpace) {
            header.append(1, ' ');
          }
          header.append(delimiterSize, delimiterChar);

          cout << header << endl;
        }

        try {
          function();
          result = 0;
        } catch (Exception& e) {
          cerr << "ERROR: " << e.what() << endl;
          result = 1;
        }

        if (addFooter) {
          footer.append(addHeader ? header.size() : delimiterSize, delimiterChar);

          cout << footer << endl;
        }

        if (addNewLine) {
          cout << endl;
        }

        return result;
      }

      int run() {
        using std::cout;
        using std::cerr;
        using std::endl;

        int result = 0;

        initialize();

        std::tuple<bool, bool, Set<String>> config;
        config = parseCommandLineArguments(commandLineArguments);

        bool helpRequested = std::get<0>(config);
        bool failFast = std::get<1>(config);
        Set<String> testNames = std::get<2>(config);

        if (helpRequested) {
          cout << "Usage: -h" << endl;
        } else {
          for (auto& [name, value] : testMap) {
            if (testNames.contains(name)) {
              Any object = std::get<1>(value);
              TestMethod function = std::get<2>(value);
              if (result == 0) {
                result = runTest(object, function, name);
              } else {
                if (failFast) {
                  break;
                } else {
                  runTest(object, function, name);
                }
              }
            }
          }
        }

        return result;
      }

      template <typename T>
      std::tuple<String, Any> getOrCreateTestNameObject(String testName) {
        if (testMap.contains(testName)) {
          String message;
          message += "Test map contains 'testName': ";
          message += testName;
          message += " (repeat not implemented yet!)";
          throw RuntimeError(message);
        }
        String testTypeName = typeid(Any).name();
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
        return std::make_tuple(testTypeName, testObject);
      }

      void addTestFunction(TestFunctionReference testFunctionReference, String testName) {
        std::tuple<String, Any> nameObject = getOrCreateTestNameObject<Any>(testName);
        String testTypeName = std::get<0>(nameObject);
        Any testObject = std::get<1>(nameObject);
        TestMethod testFunction = testFunctionReference;
        testMap[testName] = std::make_tuple(testTypeName, testObject, testFunction);
      }

      template <typename T>
      void addTestMethod(TestMethodReference<T> testMethodReference, String testName) {
        std::tuple<String, Any> nameObject = getOrCreateTestNameObject<T>(testName);
        String testTypeName = std::get<0>(nameObject);
        Any testObject = std::get<1>(nameObject);
        TestMethod testFunction = std::bind(testMethodReference, std::any_cast<T>(testObject));
        testMap[testName] = std::make_tuple(testTypeName, testObject, testFunction);
      }

      template <typename... ARGS>
      void addTestFunctions(TestFunctionReference testFunctionReference, String testName, ARGS... args) {
        addTestFunction(testFunctionReference, testName);
        addTestFunctions(args...);
      }

      void addTestFunctions() {
      }

      template <typename T, typename... ARGS>
      void addTestMethods(TestMethodReference<T> testMethodReference, String testName, ARGS... args) {
        addTestMethod(testMethodReference, testName);
        addTestMethods(args...);
      }

      void addTestMethods() {
      }

  };

}
