#include <gtest/gtest.h>

#include "../core/proto_base.hpp"
#include "../core/proto_json_serializer.hpp"
#include "../core/proto_json_serializer_binary.hpp"
#include "../core/proto_json_serializer_contrain.hpp"
#include "../core/proto_json_serializer_enum.hpp"
#include "../core/proto_json_serializer_struct.hpp"
#include "../core/serializer_base.hpp"

NEKO_USE_NAMESPACE

enum TEnum { TEnum_A = 1, TEnum_B = 2, TEnum_C = 3 };

struct StructA {
    int a;
    std::string b;
    bool c;
    double d;
    std::list<int> e;
    std::map<std::string, int> f;
    std::array<int, 5> g;
    TEnum h;
};

#if (defined(_MSC_VER) && _MSVC_LANG <= 201402L) || (defined(__GNUC__) && __cplusplus < 201703L)
NEKO_BEGIN_NAMESPACE
template <>
struct JsonConvert<StructA, void> {
    static bool toJsonValue(JsonWriter& writer, const StructA& value) {
        auto ret = writer.StartArray();
        ret = JsonConvert<int>::toJsonValue(writer, value.a) && ret;
        ret = JsonConvert<std::string>::toJsonValue(writer, value.b) && ret;
        ret = JsonConvert<bool>::toJsonValue(writer, value.c) && ret;
        ret = JsonConvert<double>::toJsonValue(writer, value.d) && ret;
        ret = JsonConvert<std::list<int>>::toJsonValue(writer, value.e) && ret;
        ret = JsonConvert<std::map<std::string, int>>::toJsonValue(writer, value.f) && ret;
        ret = JsonConvert<std::array<int, 5>>::toJsonValue(writer, value.g) && ret;
        ret = JsonConvert<TEnum>::toJsonValue(writer, value.h) && ret;
        ret = writer.EndArray() && ret;
        return ret;
    }
    static bool fromJsonValue(StructA* result, const JsonValue& value) {
        if (result == nullptr || !value.IsArray()) {
            return false;
        }
        auto ret = JsonConvert<int>::fromJsonValue(&result->a, value[0]);
        ret = JsonConvert<std::string>::fromJsonValue(&result->b, value[1]) && ret;
        ret = JsonConvert<bool>::fromJsonValue(&result->c, value[2]) && ret;
        ret = JsonConvert<double>::fromJsonValue(&result->d, value[3]) && ret;
        ret = JsonConvert<std::list<int>>::fromJsonValue(&result->e, value[4]) && ret;
        ret = JsonConvert<std::map<std::string, int>>::fromJsonValue(&result->f, value[5]) && ret;
        ret = JsonConvert<std::array<int, 5>>::fromJsonValue(&result->g, value[6]) && ret;
        ret = JsonConvert<TEnum>::fromJsonValue(&result->h, value[7]) && ret;
        return ret;
    }
};

NEKO_END_NAMESPACE
#endif

struct TestP : public ProtoBase<TestP, JsonSerializer> {
    int a = 1;
    std::string b = "hello";
    bool c = true;
    double d = 3.14;
    std::list<int> e = {1, 2, 3, 4, 5};
    std::map<std::string, int> f = {{"a", 1}, {"b", 2}, {"c", 3}};
    std::array<int, 5> g = {1, 2, 3, 4, 5};
    TEnum h = TEnum_A;
    StructA i = {1, "hello", true, 3.14, {1, 2, 3, 4, 5}, {{"a", 1}, {"b", 2}, {"c", 3}}, {1, 2, 3, 4, 5}, TEnum_A};
    std::tuple<int, std::string> j = {1, "hello"};

    NEKO_SERIALIZER(a, b, c, d, e, f, g, h, i, j)
};

TEST(JsonSerializerTest, StructSerialize) {
    TestP testp;
    testp.a = 3;
    testp.b = "Struct test";
    testp.c = true;
    testp.d = 3.141592654;
    testp.e = {1, 2, 3};
    testp.f = {{"a", 1}, {"b", 2}};
    testp.g = {1, 2, 3};
    testp.h = TEnum_A;
    testp.i = {1, "hello", true, 3.141592654, {1, 2, 3}, {{"a", 1}, {"b", 2}}, {1, 2, 3}, TEnum_A};
    testp.j = {1, "hello"};
    std::vector<char> data;
    data = testp.toData();
    data.push_back('\0');
#if NEKO_CPP_PLUS >= 17
    EXPECT_STREQ(data.data(), "{\"a\":3,\"b\":\"Struct "
                              "test\",\"c\":true,\"d\":3.141592654,\"e\":[1,2,3],\"f\":{\"a\":1,\"b\":2},\"g\":[1,2,3,"
                              "0,0],\"h\":\"TEnum_A(1)\",\"i\":[1,\"hello\",true,3.141592654,[1,2,3],{\"a\":1,\"b\":2},"
                              "[1,2,3,0,0],\"TEnum_A(1)\"],\"j\":[1,\"hello\"]}");
#else
    EXPECT_STREQ(data.data(),
                 "{\"a\":3,\"b\":\"Struct "
                 "test\",\"c\":true,\"d\":3.141592654,\"e\":[1,2,3],\"f\":{\"a\":1,\"b\":2},\"g\":[1,2,3,0,0],\"h\":1,"
                 "\"i\":[1,\"hello\",true,3.141592654,[1,2,3],{\"a\":1,\"b\":2},[1,2,3,0,0],1],\"j\":[1,\"hello\"]}");
#endif
}

TEST(JsonSerializerTest, StructDeserialize) {
    TestP testp;
    std::string str = "{\"a\":3,\"b\":\"Struct "
                      "test\",\"c\":true,\"d\":3.141592654,\"e\":[1,2,3],\"f\":{\"a\":1,\"b\":2},\"g\":[1,2,3,0,0],"
                      "\"h\":\"TEnum_A(1)\",\"i\":[1,\"hello\",true,3.141592654,[1,2,3],{\"a\":1,\"b\":2},[1,2,3,0,0],"
                      "\"TEnum_A(1)\"],\"j\":[1,\"hello\"]}";
    std::vector<char> data(str.begin(), str.end());
    testp.formData(data);
    EXPECT_EQ(testp.a, 3);
    EXPECT_STREQ(testp.b.c_str(), "Struct test");
    EXPECT_TRUE(testp.c);
    EXPECT_DOUBLE_EQ(testp.d, 3.141592654);
    EXPECT_EQ(testp.e.size(), 3);
    EXPECT_EQ(testp.f.size(), 2);
    EXPECT_EQ(testp.f["a"], 1);
    EXPECT_EQ(testp.f["b"], 2);
    EXPECT_EQ(testp.g.size(), 5);
    EXPECT_EQ(testp.g[0], 1);
    EXPECT_EQ(testp.g[1], 2);
    EXPECT_EQ(testp.g[2], 3);
    EXPECT_EQ(testp.h, TEnum_A);
    EXPECT_EQ(testp.i.a, 1);
    EXPECT_STREQ(testp.i.b.c_str(), "hello");
    EXPECT_TRUE(testp.i.c);
    EXPECT_DOUBLE_EQ(testp.i.d, 3.141592654);
    EXPECT_EQ(testp.i.e.size(), 3);
    EXPECT_EQ(testp.i.f.size(), 2);
    EXPECT_EQ(testp.i.f["a"], 1);
    EXPECT_EQ(testp.i.f["b"], 2);
    EXPECT_EQ(testp.i.g.size(), 5);
    EXPECT_EQ(testp.i.g[0], 1);
    EXPECT_EQ(testp.i.g[1], 2);
    EXPECT_EQ(testp.i.g[2], 3);
    EXPECT_EQ(testp.i.g[3], 0);
    EXPECT_EQ(testp.i.g[4], 0);
    EXPECT_EQ(testp.i.h, TEnum_A);
    EXPECT_EQ(std::get<0>(testp.j), 1);
    EXPECT_STREQ(std::get<1>(testp.j).c_str(), "hello");
}

TEST(JsonSerializerTest, Base64Covert) {
    const char* str = "this is a test string";
    auto base64string = Base64Covert::Encode(str);
    base64string.push_back('\0');
    EXPECT_STREQ(base64string.data(), "dGhpcyBpcyBhIHRlc3Qgc3RyaW5n");
    base64string.pop_back();
    auto str2 = Base64Covert::Decode(base64string);
    str2.push_back('\0');
    EXPECT_STREQ(str2.data(), str);

    const char* str3 = "this is a test string2";
    auto base64string2 = Base64Covert::Encode(str3);
    base64string2.push_back('\0');
    EXPECT_STREQ(base64string2.data(), "dGhpcyBpcyBhIHRlc3Qgc3RyaW5nMg==");
    base64string2.pop_back();
    auto str4 = Base64Covert::Decode(base64string2);
    str4.push_back('\0');
    EXPECT_STREQ(str4.data(), str3);

    const char* str5 = "this is a test string21";
    auto base64string3 = Base64Covert::Encode(str5);
    base64string3.push_back('\0');
    EXPECT_STREQ(base64string3.data(), "dGhpcyBpcyBhIHRlc3Qgc3RyaW5nMjE=");
    base64string3.pop_back();
    auto str6 = Base64Covert::Decode(base64string3);
    str6.push_back('\0');
    EXPECT_STREQ(str6.data(), str5);

    const char* str7 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    auto base64string4 = Base64Covert::Encode(str7);
    base64string4.push_back('\0');
    EXPECT_STREQ(base64string4.data(),
                 "QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVphYmNkZWZnaGlqa2xtbm9wcXJzdHV2d3h5ejAxMjM0NTY3ODkrLw==");
    base64string4.pop_back();
    auto str8 = Base64Covert::Decode(base64string4);
    str8.push_back('\0');
    EXPECT_STREQ(str8.data(), str7);

    auto str9 = Base64Covert::Decode(str7);
    str9.push_back('\0');
    EXPECT_STREQ(str9.data(), "");

    auto str10 = Base64Covert::Decode("");
    str10.push_back('\0');
    EXPECT_STREQ(str10.data(), "");
}

int main(int argc, char** argv) {
    std::cout << "NEKO_CPP_PLUS: " << NEKO_CPP_PLUS << std::endl;
    testing::InitGoogleTest(&argc, argv);
    spdlog::set_level(spdlog::level::debug);
    return RUN_ALL_TESTS();
}