#include "Utils.hpp"

using namespace geode::prelude;

namespace VantaUtils {
    std::string getNodeName(CCNode* node) {
        if (!node) return "nullptr";
        if (!node->getID().empty()) return node->getID();
        return typeid(*node).name();
    }

    void setNodePosition(CCNode* node, CCPoint pos) {
        if (node) node->setPosition(pos);
    }

    void setNodeScale(CCNode* node, float scale) {
        if (node) node->setScale(scale);
    }

    void setNodeVisible(CCNode* node, bool visible) {
        if (node) node->setVisible(visible);
    }
}
