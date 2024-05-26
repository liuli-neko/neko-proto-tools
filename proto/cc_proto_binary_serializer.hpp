#pragma once

#include <stdint.h>

#include <vector>
#ifndef _WIN32
#include <arpa/inet.h>
#endif

#include "cc_proto_global.hpp"

CS_PROTO_BEGIN_NAMESPACE

#ifdef _WIN32
inline int16_t htobe16(const int16_t value) { return _byteswap_ushort(value); }
inline int16_t be16toh(const int16_t value) { return _byteswap_ushort(value); }
inline uint16_t htobe16(const uint16_t value) { return _byteswap_ushort(value); }
inline uint16_t be16toh(const uint16_t value) { return _byteswap_ushort(value); }
inline int32_t htobe32(const int32_t value) { return _byteswap_ulong(value); }
inline int32_t be32toh(const int32_t value) { return _byteswap_ulong(value); }
inline uint32_t htobe32(const uint32_t value) { return _byteswap_ulong(value); }
inline uint32_t be32toh(const uint32_t value) { return _byteswap_ulong(value); }
inline int64_t htobe64(const int64_t value) { return _byteswap_uint64(value); }
inline int64_t be64toh(const int64_t value) { return _byteswap_uint64(value); }
inline uint64_t htobe64(const uint64_t value) { return _byteswap_uint64(value); }
inline uint64_t be64toh(const uint64_t value) { return _byteswap_uint64(value); }
#endif

template <typename WriterT, typename ValueT, typename T, class enable = void>
struct BinaryConvert;

class BinarySerializer {
public:
    using WriterType = std::vector<char>;
    using ValueType = std::vector<char>;
public:
    ~BinarySerializer();
    void startSerialize(std::vector<char>* data);
    inline bool endSerialize();
    template <typename T>
    bool insert(const char* name, const size_t len, const T& value);
    bool startDeserialize(const std::vector<char>& data);
    bool endDeserialize();
    template <typename T>
    bool get(const char* name, const size_t len, T* value);

private:
    std::vector<char>* mData = nullptr;
    bool mDeserializing = false;
    int mOffset = 0;
};

inline BinarySerializer::~BinarySerializer()  {
    if (mDeserializing) {
        delete mData;
        mData = nullptr;
    }
}

inline void BinarySerializer::startSerialize(std::vector<char>* data) {
    mData = data;
}

inline bool BinarySerializer::endSerialize() {
    mData = nullptr;
    return true;
}

template <typename T>
bool BinarySerializer::insert(const char* name, const size_t len, const T& value) {
    return BinaryConvert<WriterType, ValueType, T>::toBinaryArray(*mData, value);
}

inline bool BinarySerializer::startDeserialize(const std::vector<char>& data) {
    mData = new std::vector<char>(data);
    mOffset = 0;
    mDeserializing = true;
    return true;
}

inline bool BinarySerializer::endDeserialize() {
    if (mOffset != mData->size() && mData->size() > 0 && mData->back() != '\0') {
        CS_LOG_WARN("binary data deserialize warning, read size{} != buf szie{}.", mOffset, mData->size());
    }
    mData->clear();
    delete mData;
    mData = nullptr;
    mDeserializing = false;
    return true;
}

template <typename T>
bool BinarySerializer::get(const char* name, const size_t len, T* value) {
    return BinaryConvert<WriterType, ValueType, T>::fromBinaryArray(value, *mData, mOffset);
}

template <typename WriterT, typename ValueT>
struct BinaryConvert<WriterT, ValueT, uint8_t, void> {
    static bool toBinaryArray(WriterT& buf, const uint8_t value) {
        buf.push_back(value);
        return true;
    }
    static bool fromBinaryArray(uint8_t* dst, const ValueT& buf, int& offset_byte) {
        *dst = buf[offset_byte];
        offset_byte += 1;
        return true;
    }
};

template <typename WriterT, typename ValueT>
struct BinaryConvert<WriterT, ValueT, int8_t, void> {
    static bool toBinaryArray(WriterT& buf, const int8_t value) {
        buf.push_back(value);
        return true;
    }
    static bool fromBinaryArray(int8_t* dst, const ValueT& buf, int& offset_byte) {
        if (buf.size() < offset_byte + 1) {
            return false;
        }
        *dst = buf[offset_byte];
        offset_byte += 1;
        return true;
    }
};

template <typename WriterT, typename ValueT>
struct BinaryConvert<WriterT, ValueT, uint16_t, void> {
    static bool toBinaryArray(WriterT& buf, const uint16_t value) {
        auto nv = htobe16(value);
        buf.push_back(nv & 0xFF);
        buf.push_back((nv >> 8) & 0xFF);
        return true;
    }
    static bool fromBinaryArray(uint16_t* dst, const ValueT& buf, int& offset_byte) {
        if (buf.size() < offset_byte + 2) {
            return false;
        }
        uint16_t value = 0;
        value |= buf[offset_byte] & 0xFF;
        value |= (buf[offset_byte + 1] & 0xFF) << 8;
        *dst = be16toh(value);
        offset_byte += 2;
        return true;
    }
};

template <typename WriterT, typename ValueT>
struct BinaryConvert<WriterT, ValueT, int16_t, void> {
    static bool toBinaryArray(WriterT& buf, const int16_t value) {
        auto nv = htobe16(value);
        buf.push_back(nv & 0xFF);
        buf.push_back((nv >> 8) & 0xFF);
        return true;
    }
    static bool fromBinaryArray(int16_t* dst, const ValueT& buf, int& offset_byte) {
        if (buf.size() < offset_byte + 2) {
            return false;
        }
        int16_t value = 0;
        value |= buf[offset_byte] & 0xFF;
        value |= (buf[offset_byte + 1] & 0xFF) << 8;
        *dst = be16toh(value);
        offset_byte += 2;
        return true;
    }
};

template <typename WriterT, typename ValueT>
struct BinaryConvert<WriterT, ValueT, uint32_t, void> {
    static bool toBinaryArray(WriterT& buf, const uint32_t value) {
        auto nv = htobe32(value);
        buf.resize(buf.size() + 4, 0);
        memcpy(buf.data() + buf.size() - 4, &nv, 4);
        return true;
    }
    static bool fromBinaryArray(uint32_t* dst, const ValueT& buf, int& offset_byte) {
        if (buf.size() < offset_byte + 4) {
            return false;
        }
        uint32_t value = 0;
        memcpy(&value, buf.data() + offset_byte, 4);
        *dst = be32toh(value);
        offset_byte += 4;
        return true;
    }
};

template <typename WriterT, typename ValueT>
struct BinaryConvert<WriterT, ValueT, int32_t, void> {
    static bool toBinaryArray(WriterT& buf, const int32_t value) {
        auto nv = htobe32(value);
        buf.resize(buf.size() + 4, 0);
        memcpy(buf.data() + buf.size() - 4, &nv, 4);
        return true;
    }
    static bool fromBinaryArray(int32_t* dst, const ValueT& buf, int& offset_byte) {
        if (buf.size() < offset_byte + 4) {
            return false;
        }
        int32_t value = 0;
        memcpy(&value, buf.data() + offset_byte, 4);
        *dst = be32toh(value);
        offset_byte += 4;
        return true;
    }
};

template <typename WriterT, typename ValueT>
struct BinaryConvert<WriterT, ValueT, uint64_t, void> {
    static bool toBinaryArray(WriterT& buf, const uint64_t value) {
        auto nv = htobe64(value);
        buf.resize(buf.size() + 8, 0);
        memcpy(buf.data() + buf.size() - 8, &nv, 8);
        return true;
    }
    static bool fromBinaryArray(uint64_t* dst, const ValueT& buf, int& offset_byte) {
        if (buf.size() < offset_byte + 8) {
            return false;
        }
        uint64_t value = 0;
        memcpy(&value, buf.data() + offset_byte, 8);
        *dst = be64toh(value);
        offset_byte += 8;
        return true;
    }
};

template <typename WriterT, typename ValueT>
struct BinaryConvert<WriterT, ValueT, int64_t, void> {
    static bool toBinaryArray(WriterT& buf, const int64_t value) {
        auto nv = htobe64(value);
        buf.resize(buf.size() + 8, 0);
        memcpy(buf.data() + buf.size() - 8, &nv, 8);
        return true;
    }
    static bool fromBinaryArray(int64_t* dst, const ValueT& buf, int& offset_byte) {
        if (buf.size() < offset_byte + 8) {
            return false;
        }
        int64_t value = 0;
        memcpy(&value, buf.data() + offset_byte, 8);
        *dst = be64toh(value);
        offset_byte += 8;
        return true;
    }
};

template <typename WriterT, typename ValueT>
struct BinaryConvert<WriterT, ValueT, std::string, void> {
    static bool toBinaryArray(WriterT& buf, const std::string& value) {
        buf.resize(buf.size() + value.size() + 8, 0);
        uint64_t len = htobe64((uint64_t)value.size());
        memcpy(buf.data() + buf.size() - 8 - value.size(), &len, 8);
        memcpy(buf.data() + buf.size() - value.size(), value.data(), value.size());
        return true;
    }
    static bool fromBinaryArray(std::string* dst, const ValueT& buf, int& offset_byte) {
        if (buf.size() < offset_byte + 8) {
            return false;
        }
        uint64_t len = 0;
        memcpy(&len, buf.data() + offset_byte, 8);
        len = be64toh(len);
        if (buf.size() < offset_byte + len + 8) {
            return false;
        }
        *dst = std::string(buf.data() + offset_byte + 4, len);
        offset_byte += len + 8;
        return true;
    }
};

#if CS_CPP_PLUS >= 20
template <typename WriterT, typename ValueT>
struct BinaryConvert<WriterT, ValueT, std::u8string, void> {
    static bool toBinaryArray(WriterT& buf, const std::u8string& value) {
        buf.resize(buf.size() + value.size() + 8, 0);
        uint64_t len = htobe64((uint64_t)value.size());
        memcpy(buf.data() + buf.size() - 8 - value.size(), &len, 8);
        memcpy(buf.data() + buf.size() - value.size(), value.data(), value.size());
        return true;
    }
    static bool fromBinaryArray(std::u8string* dst, const ValueT& buf, int& offset_byte) {
        if (buf.size() < offset_byte + 8) {
            return false;
        }
        uint64_t len = 0;
        memcpy(&len, buf.data() + offset_byte, 8);
        len = be64toh(len);
        if (buf.size() < offset_byte + len + 8) {
            return false;
        }
        *dst = std::u8string(reinterpret_cast<const char8_t*>(buf.data()) + offset_byte + 4, len);
        offset_byte += len + 8;
        return true;
    }
};
#endif

CS_PROTO_END_NAMESPACE