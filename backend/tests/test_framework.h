#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>

class TestFramework {
public:
    struct TestResult {
        std::string name;
        bool passed;
        std::string message;
        double duration_ms;
    };

    static TestFramework& getInstance() {
        static TestFramework instance;
        return instance;
    }

    void addTest(const std::string& name, std::function<void()> test) {
        tests_.push_back({name, test});
    }

    void runAll() {
        std::cout << "\n=== Running Unit Tests ===\n\n";
        
        int passed = 0;
        int failed = 0;
        
        for (const auto& test : tests_) {
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                test.function();
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration<double, std::milli>(end - start).count();
                
                results_.push_back({test.name, true, "PASSED", duration});
                std::cout << "✓ " << test.name << " (" << duration << "ms)\n";
                passed++;
            } catch (const std::exception& e) {
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration<double, std::milli>(end - start).count();
                
                results_.push_back({test.name, false, std::string("FAILED: ") + e.what(), duration});
                std::cout << "✗ " << test.name << " - " << e.what() << " (" << duration << "ms)\n";
                failed++;
            }
        }
        
        std::cout << "\n=== Test Summary ===\n";
        std::cout << "Passed: " << passed << "\n";
        std::cout << "Failed: " << failed << "\n";
        std::cout << "Total: " << (passed + failed) << "\n\n";
        
        if (failed > 0) {
            std::cout << "Some tests failed!\n";
        } else {
            std::cout << "All tests passed! ✓\n";
        }
    }

    const std::vector<TestResult>& getResults() const {
        return results_;
    }

private:
    struct Test {
        std::string name;
        std::function<void()> function;
    };

    std::vector<Test> tests_;
    std::vector<TestResult> results_;
};

// Helper macros for testing
#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        throw std::runtime_error("Assertion failed: " #condition); \
    }

#define ASSERT_FALSE(condition) \
    if (condition) { \
        throw std::runtime_error("Assertion failed: " #condition " should be false"); \
    }

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        throw std::runtime_error("Assertion failed: expected " + std::to_string(expected) + " but got " + std::to_string(actual)); \
    }

#define ASSERT_STR_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        throw std::runtime_error("Assertion failed: expected '" + std::string(expected) + "' but got '" + std::string(actual) + "'"); \
    }

#define ASSERT_NOT_NULL(ptr) \
    if ((ptr) == nullptr) { \
        throw std::runtime_error("Assertion failed: pointer should not be null"); \
    }

#define ASSERT_NULL(ptr) \
    if ((ptr) != nullptr) { \
        throw std::runtime_error("Assertion failed: pointer should be null"); \
    }

#define TEST(name) \
    void test_##name(); \
    static bool registered_##name = []() { \
        TestFramework::getInstance().addTest(#name, test_##name); \
        return true; \
    }(); \
    void test_##name()

#define RUN_ALL_TESTS() \
    TestFramework::getInstance().runAll();