#include <gtest/gtest.h>

#include "../core/binary_serializer.hpp"
#include "../core/json_serializer.hpp"
#include "../core/json_serializer_binary.hpp"
#include "../core/json_serializer_container.hpp"
#include "../core/json_serializer_enum.hpp"
#include "../core/json_serializer_struct.hpp"
#include "../core/proto_base.hpp"
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

#if NEKO_CPP_PLUS < 17
NEKO_BEGIN_NAMESPACE
template <typename WriterT, typename ValueT>
struct JsonConvert<WriterT, ValueT, StructA, void> {
    static bool toJsonValue(WriterT& writer, const StructA& value) {
        auto ret = writer.StartArray();
        ret      = JsonConvert<WriterT, ValueT, int>::toJsonValue(writer, value.a) && ret;
        ret      = JsonConvert<WriterT, ValueT, std::string>::toJsonValue(writer, value.b) && ret;
        ret      = JsonConvert<WriterT, ValueT, bool>::toJsonValue(writer, value.c) && ret;
        ret      = JsonConvert<WriterT, ValueT, double>::toJsonValue(writer, value.d) && ret;
        ret      = JsonConvert<WriterT, ValueT, std::list<int>>::toJsonValue(writer, value.e) && ret;
        ret      = JsonConvert<WriterT, ValueT, std::map<std::string, int>>::toJsonValue(writer, value.f) && ret;
        ret      = JsonConvert<WriterT, ValueT, std::array<int, 5>>::toJsonValue(writer, value.g) && ret;
        ret      = JsonConvert<WriterT, ValueT, TEnum>::toJsonValue(writer, value.h) && ret;
        ret      = writer.EndArray() && ret;
        return ret;
    }
    static bool fromJsonValue(StructA* result, const ValueT& value) {
        if (result == nullptr || !value.IsArray()) {
            return false;
        }
        auto ret = JsonConvert<WriterT, ValueT, int>::fromJsonValue(&result->a, value[0]);
        ret      = JsonConvert<WriterT, ValueT, std::string>::fromJsonValue(&result->b, value[1]) && ret;
        ret      = JsonConvert<WriterT, ValueT, bool>::fromJsonValue(&result->c, value[2]) && ret;
        ret      = JsonConvert<WriterT, ValueT, double>::fromJsonValue(&result->d, value[3]) && ret;
        ret      = JsonConvert<WriterT, ValueT, std::list<int>>::fromJsonValue(&result->e, value[4]) && ret;
        ret      = JsonConvert<WriterT, ValueT, std::map<std::string, int>>::fromJsonValue(&result->f, value[5]) && ret;
        ret      = JsonConvert<WriterT, ValueT, std::array<int, 5>>::fromJsonValue(&result->g, value[6]) && ret;
        ret      = JsonConvert<WriterT, ValueT, TEnum>::fromJsonValue(&result->h, value[7]) && ret;
        return ret;
    }
};

NEKO_END_NAMESPACE
#endif

struct TestP {
    int a                        = 1;
    std::string b                = "hello";
    bool c                       = true;
    double d                     = 3.14;
    std::list<int> e             = {1, 2, 3, 4, 5};
    std::map<std::string, int> f = {{"a", 1}, {"b", 2}, {"c", 3}};
    std::array<int, 5> g         = {1, 2, 3, 4, 5};
    TEnum h                      = TEnum_A;
    StructA i = {1, "hello", true, 3.14, {1, 2, 3, 4, 5}, {{"a", 1}, {"b", 2}, {"c", 3}}, {1, 2, 3, 4, 5}, TEnum_A};
    std::tuple<int, std::string> j = {1, "hello"};
#if NEKO_CPP_PLUS >= 17
    std::optional<int> k;
    NEKO_SERIALIZER(a, b, c, d, e, f, g, h, i, j, k)
#else
    NEKO_SERIALIZER(a, b, c, d, e, f, g, h, i, j)
#endif
    NEKO_DECLARE_PROTOCOL(TestP, JsonSerializer)
};

struct UnusedProto {
    int a;
    NEKO_SERIALIZER(a)
    NEKO_DECLARE_PROTOCOL(UnusedProto, JsonSerializer)
};

struct BinaryProto {
    int32_t a     = 1;
    std::string b = "hello";
    uint32_t c    = 3;

    NEKO_SERIALIZER(a, b, c)
    NEKO_DECLARE_PROTOCOL(BinaryProto, BinarySerializer)
};

class ProtoTest : public testing::Test {
protected:
    virtual void SetUp() { factory.reset(new ProtoFactory()); }
    virtual void TearDown() {}
    std::unique_ptr<ProtoFactory> factory;
};

TEST_F(ProtoTest, StructSerialize) {
    TestP testp;
    testp.a = 3;
    testp.b = "Struct test";
    testp.c = true;
    testp.d = 3.141592654;
    testp.e = std::list<int>{1, 2, 3};
    testp.f = std::map<std::string, int>{{"a", 1}, {"b", 2}};
    testp.g = std::array<int, 5>{1, 2, 3, 0, 0};
    testp.h = TEnum_A;
    testp.i = StructA{1, "hello", true, 3.141592654, {1, 2, 3}, {{"a", 1}, {"b", 2}}, {1, 2, 3}, TEnum_A};
    testp.j = std::make_tuple(1, "hello");

    std::vector<char> data;
    data = testp.makeProto().toData();
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

TEST_F(ProtoTest, StructDeserialize) {
    std::string str = "{\"a\":3,\"b\":\"Struct "
                      "test\",\"c\":true,\"d\":3.141592654,\"e\":[1,2,3],\"f\":{\"a\":1,\"b\":2},\"g\":[1,2,3,0,0],"
                      "\"h\":\"TEnum_A(1)\",\"i\":[1,\"hello\",true,3.141592654,[1,2,3],{\"a\":1,\"b\":2},[1,2,3,0,0],"
                      "\"TEnum_A(1)\"],\"j\":[1,\"hello\"],\"k\":1}";
    std::vector<char> data(str.begin(), str.end());
    TestP testp;
    EXPECT_TRUE(testp.makeProto().formData(data));
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
#if NEKO_CPP_PLUS >= 17
    EXPECT_EQ(testp.k.value_or(-1), 1);
#endif
    NEKO_LOG_INFO("{}", SerializableToString(testp));
}

TEST_F(ProtoTest, Base64Covert) {
    const char* str   = "this is a test string";
    auto base64string = Base64Covert::Encode(str);
    base64string.push_back('\0');
    EXPECT_STREQ(base64string.data(), "dGhpcyBpcyBhIHRlc3Qgc3RyaW5n");
    base64string.pop_back();
    auto str2 = Base64Covert::Decode(base64string);
    str2.push_back('\0');
    EXPECT_STREQ(str2.data(), str);

    const char* str3   = "this is a test string2";
    auto base64string2 = Base64Covert::Encode(str3);
    base64string2.push_back('\0');
    EXPECT_STREQ(base64string2.data(), "dGhpcyBpcyBhIHRlc3Qgc3RyaW5nMg==");
    base64string2.pop_back();
    auto str4 = Base64Covert::Decode(base64string2);
    str4.push_back('\0');
    EXPECT_STREQ(str4.data(), str3);

    const char* str5   = "this is a test string21";
    auto base64string3 = Base64Covert::Encode(str5);
    base64string3.push_back('\0');
    EXPECT_STREQ(base64string3.data(), "dGhpcyBpcyBhIHRlc3Qgc3RyaW5nMjE=");
    base64string3.pop_back();
    auto str6 = Base64Covert::Decode(base64string3);
    str6.push_back('\0');
    EXPECT_STREQ(str6.data(), str5);

    const char* str7   = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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

TEST_F(ProtoTest, JsonProtoRef) {
    std::string str = "{\"a\":3,\"b\":\"Struct "
                      "test\",\"c\":true,\"d\":3.141592654,\"e\":[1,2,3],\"f\":{\"a\":1,\"b\":2},\"g\":[1,2,3,0,0],"
                      "\"h\":\"TEnum_A(1)\",\"i\":[1,\"hello\",true,3.141592654,[1,2,3],{\"a\":1,\"b\":2},[1,2,3,0,0],"
                      "\"TEnum_A(1)\"],\"j\":[1,\"hello\"]}";
    auto proto      = factory->create("TestP");
    proto->formData(std::vector<char>(str.data(), str.data() + str.length()));
    auto rawp = proto->cast<TestP>(); // success cast

    EXPECT_TRUE(proto->cast<BinaryProto>() == nullptr); // failed cast

    // get field test
    int a = {};
    EXPECT_TRUE(proto->getField("a", &a));                                      // success get field
    EXPECT_FALSE(proto->getField("b", &a));                                     // get field by wrong type
    EXPECT_FALSE(proto->getField("unexist field", &a));                         // get unexist field
    EXPECT_STREQ(proto->getField<std::string>("b", "").c_str(), "Struct test"); // success get field
    EXPECT_TRUE(proto->getField<bool>("c", false));                             // success get field
    EXPECT_FALSE(proto->getField<bool>("b", false));                            // get field by wrong type
    EXPECT_FALSE(proto->getField<bool>("unexist field", false));                // get unexist field
    EXPECT_EQ(a, 3);

    // set field test
    EXPECT_TRUE(proto->setField("a", 14));                            // success set field
    EXPECT_TRUE(proto->setField("b", std::string("field set test"))); // success set field
    EXPECT_TRUE(proto->setField("c", false));                         // success set field
    EXPECT_FALSE(proto->setField("a", 3.1234));                       // set field by wrong type
    EXPECT_FALSE(proto->setField("unexist field", 3.1234));           // set unexist field
    EXPECT_EQ(rawp->a, 14);
    EXPECT_STREQ(rawp->b.c_str(), "field set test");
    EXPECT_FALSE(rawp->c);

    EXPECT_DOUBLE_EQ(rawp->d, 3.141592654);
    EXPECT_EQ(rawp->e.size(), 3);
    EXPECT_EQ(rawp->f.size(), 2);
    EXPECT_EQ(rawp->f["a"], 1);
    EXPECT_EQ(rawp->f["b"], 2);
    EXPECT_EQ(rawp->g.size(), 5);
    EXPECT_EQ(rawp->g[0], 1);
    EXPECT_EQ(rawp->g[1], 2);
    EXPECT_EQ(rawp->g[2], 3);
    EXPECT_EQ(rawp->h, TEnum_A);
    EXPECT_EQ(rawp->i.a, 1);
    EXPECT_STREQ(rawp->i.b.c_str(), "hello");
    EXPECT_TRUE(rawp->i.c);
    EXPECT_DOUBLE_EQ(rawp->i.d, 3.141592654);
    EXPECT_EQ(rawp->i.e.size(), 3);
    EXPECT_EQ(rawp->i.f.size(), 2);
    EXPECT_EQ(rawp->i.f["a"], 1);
    EXPECT_EQ(rawp->i.f["b"], 2);
    EXPECT_EQ(rawp->i.g.size(), 5);
    EXPECT_EQ(rawp->i.g[0], 1);
    EXPECT_EQ(rawp->i.g[1], 2);
    EXPECT_EQ(rawp->i.g[2], 3);
    EXPECT_EQ(rawp->i.g[3], 0);
    EXPECT_EQ(rawp->i.g[4], 0);
    EXPECT_EQ(rawp->i.h, TEnum_A);
    EXPECT_EQ(std::get<0>(rawp->j), 1);
    EXPECT_STREQ(std::get<1>(rawp->j).c_str(), "hello");
    NEKO_LOG_INFO("{}", SerializableToString(*rawp));
}

TEST_F(ProtoTest, InvalidParams) {
    EXPECT_TRUE(factory->create("InvalidP") == nullptr);
    EXPECT_TRUE(factory->create(-1) == nullptr);
}

TEST_F(ProtoTest, BinaryProto) {
    BinaryProto proto;
    proto.a        = 24;
    proto.b        = "hello Neko Proto";
    proto.c        = 0x3f3f3f;
    auto data      = proto.makeProto().toData();
    auto base64str = Base64Covert::Encode(data);
    base64str.push_back('\0');
    EXPECT_STREQ(base64str.data(), "AAAAGAAAAAAAAAAQaGVsbG8gTmVrbyBQcm90bwA/Pz8=");
    NEKO_LOG_INFO("{}", SerializableToString(proto));
}

int main(int argc, char** argv) {
    std::cout << "NEKO_CPP_PLUS: " << NEKO_CPP_PLUS << std::endl;
    testing::InitGoogleTest(&argc, argv);
    spdlog::set_level(spdlog::level::debug);
    return RUN_ALL_TESTS();
}