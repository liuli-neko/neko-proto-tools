/**
 * @file proto_base.cpp
 * @author llhsdmd (llhsdmd@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-06-18
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "../core/proto_base.hpp"

#include <functional>
#include <vector>

NEKO_BEGIN_NAMESPACE

void ProtoFactory::setVersion(int major, int minor, int patch) {
    mVersion = ((major & 0xFF) << 16 | (minor & 0xFF) << 8 | (patch & 0xFF));
}

uint32_t ProtoFactory::version() const { return mVersion; }

std::map<NEKO_STRING_VIEW, std::function<void(ProtoFactory*)>>&
static_init_funcs(const NEKO_STRING_VIEW& name = "", std::function<void(ProtoFactory*)> func = nullptr) {
    static std::map<NEKO_STRING_VIEW, std::function<void(ProtoFactory*)>> funcs = {};
    auto item = funcs.find(name);
    if (!name.empty() && item == funcs.end() && func) {
        funcs.insert(std::make_pair(name, func));
    }
    if (item != funcs.end()) {
        NEKO_LOG_WARN("Duplicate init function: {}", name);
    }
    return funcs;
}

void ProtoFactory::init() {
    for (auto& item : static_init_funcs()) {
        item.second(this);
    }
}

ProtoFactory::ProtoFactory(int major, int minor, int patch) {
    init();
    setVersion(major, minor, patch);
}

ProtoFactory::ProtoFactory(const ProtoFactory& other)
    : mProtoMap(other.mProtoMap), mProtoNameMap(other.mProtoNameMap), mVersion(other.mVersion) {}

ProtoFactory::ProtoFactory(ProtoFactory&& other)
    : mProtoMap(std::move(other.mProtoMap)), mProtoNameMap(std::move(other.mProtoNameMap)), mVersion(other.mVersion) {}

ProtoFactory& ProtoFactory::operator=(const ProtoFactory& other) {
    if (this != &other) {
        mProtoMap = other.mProtoMap;
        mProtoNameMap = other.mProtoNameMap;
        mVersion = other.mVersion;
    }
    return *this;
}

ProtoFactory& ProtoFactory::operator=(ProtoFactory&& other) {
    if (this != &other) {
        mProtoMap = std::move(other.mProtoMap);
        mProtoNameMap = std::move(other.mProtoNameMap);
        mVersion = other.mVersion;
    }
    return *this;
}

IProto* ProtoFactory::create(int type) const {
    auto item = mProtoMap.find(type);
    if (mProtoMap.end() != item) {
        return (item->second)();
    }
    return nullptr;
}

IProto* ProtoFactory::create(const char* name) const {
    auto item = mProtoNameMap.find(name);
    if (mProtoNameMap.end() == item) {
        return nullptr;
    }
    auto itemType = mProtoMap.find(item->second);
    if (mProtoMap.end() != itemType) {
        return (itemType->second)();
    }
    return nullptr;
}

ProtoFactory::~ProtoFactory() {
    // Nothing to do
}

int type_counter() {
    static int counter = 0;
    return ++counter;
}

NEKO_END_NAMESPACE