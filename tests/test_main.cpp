// Catch2 main
// This file provides the main() function for Catch2 tests
#include <catch2/catch_session.hpp>

int main(int argc, char* argv[]) {
    return Catch::Session().run(argc, argv);
}
