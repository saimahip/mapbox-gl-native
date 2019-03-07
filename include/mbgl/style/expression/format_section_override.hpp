#pragma once

#include <mbgl/style/expression/expression.hpp>
#include <mbgl/renderer/possibly_evaluated_property_value.hpp>

namespace mbgl {
namespace style {
namespace expression {

template<class T>
class FormatSectionOveride final : public Expression {
public:
    FormatSectionOveride(const type::Type& type_,
                         PossiblyEvaluatedPropertyValue<T> defaultValue_,
                         std::string propertyName_) :
        Expression(Kind::FormatSectionOverride, type_),
        defaultValue(std::move(defaultValue_)),
        propertyName(propertyName_)
    {}

    EvaluationResult evaluate(const EvaluationContext& context) const {
        using Object = std::unordered_map<std::string, expression::Value>;
        if (context.formattedSection &&
            context.formattedSection->is<Object>()) {
            const auto& section = context.formattedSection->get<Object>();
            if (section.find(propertyName) != section.end()) {
                return section.at(propertyName);
            }
        }
        return defaultValue.evaluate(*context.feature, *context.zoom, T());
    }

    void eachChild(const std::function<void(const Expression&)>&) const {
    }

    bool operator==(const Expression&) const {
        return false;
    }

    std::vector<optional<Value>> possibleOutputs() const {
        return {nullopt};
    }

    std::string getOperator() const { return "format-section-override"; }

private:
    PossiblyEvaluatedPropertyValue<T> defaultValue;
    std::string propertyName;
};

} // namespace expression
} // namespace style
} // namespace mbgl
