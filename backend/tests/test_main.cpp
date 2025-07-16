#include "test_framework.h"

// Include all test files
#include "test_database.cpp"
#include "test_auth_service.cpp"
#include "test_todo_service.cpp"
#include "test_integration.cpp"

int main() {
    std::cout << "=== Todo Backend Unit Tests ===\n";
    std::cout << "Running comprehensive test suite...\n";
    
    RUN_ALL_TESTS();
    
    const auto& results = TestFramework::getInstance().getResults();
    
    // Check if all tests passed
    bool all_passed = true;
    for (const auto& result : results) {
        if (!result.passed) {
            all_passed = false;
            break;
        }
    }
    
    return all_passed ? 0 : 1;
}