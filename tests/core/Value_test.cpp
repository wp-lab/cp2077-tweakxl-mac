#include <catch2/catch_test_macros.hpp>
#include "core/Value.hpp"

using namespace TweakXL;

TEST_CASE("Value primitive types", "[Value]") {
    SECTION("Bool value") {
        Value v(true);
        REQUIRE(v.IsBool());
        REQUIRE(v.AsBool() == true);
        REQUIRE(v.ToString() == "true");
    }

    SECTION("Int32 value") {
        Value v(42);
        REQUIRE(v.IsInt32());
        REQUIRE(v.AsInt32() == 42);
    }

    SECTION("Float value") {
        Value v(3.14f);
        REQUIRE(v.IsFloat());
        REQUIRE(v.AsFloat() == 3.14f);
    }

    SECTION("String value") {
        Value v("test string");
        REQUIRE(v.IsString());
        REQUIRE(v.AsString() == "test string");
        REQUIRE(v.ToString() == "\"test string\"");
    }
}

TEST_CASE("Value special types", "[Value]") {
    SECTION("CName value") {
        CName cname("Combat");
        Value v(cname);
        REQUIRE(v.IsCName());
        REQUIRE(v.AsCName() == cname);
    }

    SECTION("TweakDBID value") {
        TweakDBID id("Items.Preset_Katana_Default");
        Value v(id);
        REQUIRE(v.IsTweakDBID());
        REQUIRE(v.AsTweakDBID() == id);
    }

    SECTION("LocKey value") {
        LocKey key(12345);
        Value v(key);
        REQUIRE(v.IsLocKey());
        REQUIRE(v.AsLocKey() == key);
    }

    SECTION("Resource value") {
        Resource res("path/to/resource.tex");
        Value v(res);
        REQUIRE(v.IsResource());
        REQUIRE(v.AsResource() == res);
    }
}

TEST_CASE("Value array", "[Value]") {
    SECTION("Empty array") {
        std::vector<Value> arr;
        Value v(arr);
        REQUIRE(v.IsArray());
        REQUIRE(v.AsArray().empty());
        REQUIRE(v.ToString() == "[]");
    }

    SECTION("Array of integers") {
        std::vector<Value> arr;
        arr.push_back(Value(1));
        arr.push_back(Value(2));
        arr.push_back(Value(3));

        Value v(arr);
        REQUIRE(v.IsArray());
        REQUIRE(v.AsArray().size() == 3);
        REQUIRE(v.AsArray()[0].AsInt32() == 1);
        REQUIRE(v.AsArray()[1].AsInt32() == 2);
        REQUIRE(v.AsArray()[2].AsInt32() == 3);
    }

    SECTION("Array of mixed types") {
        std::vector<Value> arr;
        arr.push_back(Value(42));
        arr.push_back(Value("text"));
        arr.push_back(Value(true));

        Value v(arr);
        REQUIRE(v.IsArray());
        REQUIRE(v.AsArray().size() == 3);
        REQUIRE(v.AsArray()[0].IsInt32());
        REQUIRE(v.AsArray()[1].IsString());
        REQUIRE(v.AsArray()[2].IsBool());
    }
}

TEST_CASE("Value type checking", "[Value]") {
    Value intVal(42);

    SECTION("Correct type access succeeds") {
        REQUIRE_NOTHROW(intVal.AsInt32());
    }

    SECTION("Incorrect type access throws") {
        REQUIRE_THROWS(intVal.AsString());
        REQUIRE_THROWS(intVal.AsBool());
        REQUIRE_THROWS(intVal.AsFloat());
    }
}

TEST_CASE("Value comparison", "[Value]") {
    SECTION("Same values are equal") {
        Value v1(42);
        Value v2(42);
        REQUIRE(v1 == v2);
    }

    SECTION("Different values are not equal") {
        Value v1(42);
        Value v2(43);
        REQUIRE(v1 != v2);
    }

    SECTION("Different types are not equal") {
        Value v1(42);
        Value v2("42");
        REQUIRE(v1 != v2);
    }
}
