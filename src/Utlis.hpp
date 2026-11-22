#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <typeinfo>
#include <sstream>
#include <type_traits>

namespace VantaUtils {
    std::string getNodeName(cocos2d::CCNode* node);
    void setNodePosition(cocos2d::CCNode* node, cocos2d::CCPoint pos);
    void setNodeScale(cocos2d::CCNode* node, float scale);
    void setNodeVisible(cocos2d::CCNode* node, bool visible);
}

#define public_cast(value, member) [](auto* v) -> auto& { \
    class FriendClass__; \
    using T = typename std::remove_pointer<decltype(v)>::type; \
    class FriendeeClass__: public T { \
    protected: \
        friend FriendClass__; \
    }; \
    class FriendClass__ { \
    public: \
        auto& get(FriendeeClass__* v) { return v->member; } \
    } c; \
    return c.get(reinterpret_cast<FriendeeClass__*>(v)); \
}(value)
