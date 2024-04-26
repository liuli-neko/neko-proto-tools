#include "../proto/cc_proto_base.hpp"

#include <functional>
#include <vector>

CS_PROTO_BEGIN_NAMESPACE

void ProtoFactory::setVersion(int major, int minor, int patch) {
    mVersion = ((major & 0xFF) << 16 |
                  (minor & 0xFF) << 8 |
                  (patch & 0xFF));
}

uint32_t ProtoFactory::version() const {
    return mVersion;
}

std::map<const char *, std::function<void(ProtoFactory*)>> 
static_init_funcs(const char *name = nullptr, std::function<void(ProtoFactory*)> func = nullptr) {
    static std::map<const char *, std::function<void(ProtoFactory*)>> funcs = {};
    auto item = funcs.find(name);
    if (name != nullptr && item == funcs.end() && func) {
        funcs.insert(std::make_pair(name, func));
    }
    if (item != funcs.end()) {
        CS_LOG_WARN("Duplicate init function: {}", name);
    }
    return funcs;
}

void ProtoFactory::init() {
    for (auto& item : static_init_funcs()) {
        CS_LOG_INFO("Init proto {} for factory({})", item.first, (void*)this);
        item.second(this);
    }
}

ProtoFactory::ProtoFactory(int major, int minor, int patch) {
    init();
    setVersion(major, minor, patch);
}

ProtoFactory::ProtoFactory(const ProtoFactory &other) : 
     mProtoMap(other.mProtoMap)
    ,mProtoNameMap(other.mProtoNameMap)
    ,mVersion(other.mVersion) { }

ProtoFactory::ProtoFactory(ProtoFactory &&other) :
    mProtoMap(std::move(other.mProtoMap))
    ,mProtoNameMap(std::move(other.mProtoNameMap))
    ,mVersion(other.mVersion) { }

ProtoFactory& ProtoFactory::operator=(const ProtoFactory &other) {
    if (this != &other) {
        mProtoMap = other.mProtoMap;
        mProtoNameMap = other.mProtoNameMap;
        mVersion = other.mVersion;
    }
    return *this;
}

ProtoFactory& ProtoFactory::operator=(ProtoFactory &&other) {
    if (this != &other) {
        mProtoMap = std::move(other.mProtoMap);
        mProtoNameMap = std::move(other.mProtoNameMap);
        mVersion = other.mVersion;
    }
    return *this;
}

ProtoFactory::~ProtoFactory() {
  // Nothing to do
}

CS_PROTO_END_NAMESPACE