#pragma once

#include "cc_proto_global.hpp"

#include <map>
#include <functional>
#include <string>
#include <fstream>

#if defined(_WIN32)
#ifdef GetObject
#undef GetObject
#endif
#endif

#include <rapidjson/istreamwrapper.h>

#include "cc_proto_json_serializer.hpp"

CS_PROTO_BEGIN_NAMESPACE

class CS_PROTO_API IDumpableObject {
public:
    virtual std::string dumpToString() const = 0;
    virtual bool loadFromString(const std::string& str) = 0;
    static bool dumpToFile(const std::string& filePath, IDumpableObject *obj);
    static bool loadFromFile(const std::string& filePath, IDumpableObject **obj);
    virtual ~IDumpableObject() = default;
    virtual CS_STRING_VIEW className() const = 0;
    static IDumpableObject *create(const CS_STRING_VIEW& className);

protected:
    static std::map<CS_STRING_VIEW, std::function<IDumpableObject *()>> mObjectMap;
};

inline std::map<CS_STRING_VIEW, std::function<IDumpableObject *()>> IDumpableObject::mObjectMap; 

inline IDumpableObject *IDumpableObject::create(const CS_STRING_VIEW& className) {
    auto it = mObjectMap.find(className);
    if (it != mObjectMap.end()) {
        return (it->second)();
    }
    return nullptr;
}

template <typename T, typename SerializerT>
class DumpableObject : public IDumpableObject {
public:
    using ObjectType = T;
    using SerializerType = SerializerT;

    virtual std::string dumpToString() const override;
    virtual bool loadFromString(const std::string& str) override;

    CS_STRING_VIEW className() const override;
private:
    static CS_STRING_VIEW _init_class_name__;
};

template <typename T, typename SerializerT>
CS_STRING_VIEW DumpableObject<T, SerializerT>::_init_class_name__ = [](){
    CS_STRING_VIEW name = _cs_class_name<T>();
    static_assert(std::is_base_of<DumpableObject<T, SerializerT>, T>::value, "T must be derived from DumpableObject<T, SerializerT>");
    CS_LOG_INFO("Dumpable Object {}({}) init", name, _cs_class_name<SerializerT>());
    mObjectMap.insert(std::make_pair(name, [](){
        return new T();
    }));
    return name;
}();

template <typename T, typename SerializerT>
CS_STRING_VIEW DumpableObject<T, SerializerT>::className() const {
    return _init_class_name__;
}

template <typename T, typename SerializerT>
std::string DumpableObject<T, SerializerT>::dumpToString() const {
    SerializerT serializer;
    serializer.startSerialize();
    auto self = const_cast<T *>(static_cast<const T *>(this));
    auto ret = self->serialize(serializer);
    std::vector<char> data;
    if (!serializer.endSerialize(&data) || !ret) {
        CS_LOG_ERROR("DumpableObject({}) dumpToString failed", className());
        return "";
    }
    return std::string(data.begin(), data.end());
}

template <typename T, typename SerializerT>
bool DumpableObject<T, SerializerT>::loadFromString(const std::string& str) {
    SerializerT serializer;
    if (!serializer.startDeserialize(std::vector<char>(str.begin(), str.end()))) {
        return false;
    }
    auto self = dynamic_cast<const T*>(this);
    bool ret = const_cast<T*>(self)->deserialize(serializer);
    if (!serializer.endDeserialize() || !ret) {
        return false;
    }
    return true;
}

template <>
struct JsonConvert<IDumpableObject *> {
    static bool toJsonValue(JsonWriter& writer, const IDumpableObject *value) {
        auto ret = writer.StartObject();
        ret = writer.Key("className") && ret;
        if (value == nullptr) {
            ret = writer.String("null") && ret;
            ret = writer.EndObject() && ret;
            return ret;
        }
        ret = writer.String(value->className().data(), value->className().length()) && ret;
        ret = writer.Key("value") && ret;
        auto str = value->dumpToString();
        ret = (!str.empty() && ret);
        ret = writer.RawValue(str.c_str(), str.length(), rapidjson::kObjectType) && ret;
        ret = writer.EndObject() && ret;
        return ret;
    }
    static bool fromJsonValue(IDumpableObject ** dst, const JsonValue& value) {
        if (!value.IsObject() || dst == nullptr) {
            return false;
        }
        if (!value.HasMember("className") || !value["className"].IsString()) {
            return false;
        }
        std::string className = value["className"].GetString();
        if (className == "null") {
            (*dst) = nullptr;
            return true;
        }
        auto d = IDumpableObject::create(className);
        CS_ASSERT(d != nullptr, "DumpableObject {} create failed", className);
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        const auto& o = value["value"];
        auto ret = o.Accept(writer);
        ret = d->loadFromString(buffer.GetString()) && ret;
        (*dst) =  d;
        return ret;
    }
};

inline bool IDumpableObject::dumpToFile(const std::string& filePath, IDumpableObject *obj) {
    auto buffer = rapidjson::StringBuffer();
    auto writer = std::make_shared<JsonWriter>(buffer);
    auto ret = JsonConvert<IDumpableObject *>::toJsonValue(*writer, obj);
    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    fprintf(stderr, "dump file: %s\n", filePath.c_str());
    file.write(buffer.GetString(), buffer.GetSize());
    file.close();
    return ret;

}

inline bool IDumpableObject::loadFromFile(const std::string& filePath, IDumpableObject **obj) {
    std::ifstream file(filePath, std::ios::in);
    if (!file.is_open()) {
        return false;
    }
    rapidjson::IStreamWrapper isw(file);
    rapidjson::Document document;
    document.ParseStream(isw);
    auto ret = JsonConvert<IDumpableObject *>::fromJsonValue(obj, document.GetObject());
    file.close();
    return ret;
}

CS_PROTO_END_NAMESPACE