#include "Value.hpp"
#include <stdexcept>
#include <sstream>

namespace TweakXL {

// Value getters with type checking
bool Value::AsBool() const {
    if (!IsBool()) {
        throw std::runtime_error("Value is not a Bool");
    }
    return std::get<bool>(data_);
}

int32 Value::AsInt32() const {
    if (!IsInt32()) {
        throw std::runtime_error("Value is not an Int32");
    }
    return std::get<int32>(data_);
}

float Value::AsFloat() const {
    if (!IsFloat()) {
        throw std::runtime_error("Value is not a Float");
    }
    return std::get<float>(data_);
}

const String& Value::AsString() const {
    if (!IsString()) {
        throw std::runtime_error("Value is not a String");
    }
    return std::get<String>(data_);
}

const CName& Value::AsCName() const {
    if (!IsCName()) {
        throw std::runtime_error("Value is not a CName");
    }
    return std::get<CName>(data_);
}

const TweakDBID& Value::AsTweakDBID() const {
    if (!IsTweakDBID()) {
        throw std::runtime_error("Value is not a TweakDBID");
    }
    return std::get<TweakDBID>(data_);
}

const LocKey& Value::AsLocKey() const {
    if (!IsLocKey()) {
        throw std::runtime_error("Value is not a LocKey");
    }
    return std::get<LocKey>(data_);
}

const Resource& Value::AsResource() const {
    if (!IsResource()) {
        throw std::runtime_error("Value is not a Resource");
    }
    return std::get<Resource>(data_);
}

const std::vector<Value>& Value::AsArray() const {
    if (!IsArray()) {
        throw std::runtime_error("Value is not an Array");
    }
    return std::get<std::vector<Value>>(data_);
}

String Value::ToString() const {
    std::ostringstream oss;

    if (IsEmpty()) {
        return "(empty)";
    } else if (IsBool()) {
        return AsBool() ? "true" : "false";
    } else if (IsInt32()) {
        return std::to_string(AsInt32());
    } else if (IsFloat()) {
        return std::to_string(AsFloat());
    } else if (IsString()) {
        return "\"" + AsString() + "\"";
    } else if (IsCName()) {
        const auto& cname = AsCName();
        if (cname.value) {
            return "n\"" + *cname.value + "\"";
        }
        oss << "n\"0x" << std::hex << cname.hash << "\"";
        return oss.str();
    } else if (IsTweakDBID()) {
        return "t\"" + AsTweakDBID().ToString() + "\"";
    } else if (IsLocKey()) {
        oss << "LocKey#" << AsLocKey().key;
        return oss.str();
    } else if (IsResource()) {
        return "r\"" + AsResource().path + "\"";
    } else if (IsArray()) {
        const auto& arr = AsArray();
        oss << "[";
        for (size_t i = 0; i < arr.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << arr[i].ToString();
        }
        oss << "]";
        return oss.str();
    }

    return "(unknown)";
}

} // namespace TweakXL
