#include <mbgl/style/layers/symbol_layer_impl.hpp>

#include <mbgl/style/expression/format_expression.hpp>
#include <mbgl/util/logging.hpp>

namespace mbgl {
namespace style {

// TODO: make generic and share with SymbolBucket or SymbolLayout.
template<class Properties>
bool hasPaintPropertyOverrides(const Properties& props) {
    return props.template get<TextField>().match(
            [] (const TextField::Type& t) {
                for(const auto& section : t.sections) {
                    if (section.textColor) {
                        return true;
                    }
                }
                return false;
            },
            [] (const PropertyExpression<TextField::Type>& t) {
                if (t.getExpression().getKind() == expression::Kind::FormatExpression) {
                    const auto* e = static_cast<const expression::FormatExpression*>(&t.getExpression());
                    for (const auto& section : e->getSections()) {
                        if (section.textColor) {
                            return true;
                        }
                    }
                }
                return false;
            },
            [] (const auto&) {
                return false;
            }
     );
}

bool hasOverriddenPaintPropertyDifference(const SymbolLayoutProperties::Unevaluated& props) {
    // if layout has overrides and paint properties with same property don't match, need to re-layout.
    // TODO: check that overriden paint property has changed && has layout override for same property.
    return hasPaintPropertyOverrides(props);
}

bool SymbolLayer::Impl::hasLayoutDifference(const Layer::Impl& other) const {
    assert(other.getTypeInfo() == getTypeInfo());
    const auto& impl = static_cast<const style::SymbolLayer::Impl&>(other);
    return filter     != impl.filter ||
           visibility != impl.visibility ||
           layout     != impl.layout ||
           paint.hasDataDrivenPropertyDifference(impl.paint) ||
           hasOverriddenPaintPropertyDifference(layout);
}

void SymbolLayer::Impl::populateFontStack(std::set<FontStack>& fontStack) const {
    if (layout.get<TextField>().isUndefined()) {
        return;
    }

    layout.get<TextFont>().match(
        [&] (Undefined) {
            fontStack.insert({"Open Sans Regular", "Arial Unicode MS Regular"});
        },
        [&] (const FontStack& constant) {
            fontStack.insert(constant);
        },
        [&] (const auto& function) {
            for (const auto& value : function.possibleOutputs()) {
                if (value) {
                    fontStack.insert(*value);
                } else {
                    Log::Warning(Event::ParseStyle, "Layer '%s' has an invalid value for text-font and will not render text. Output values must be contained as literals within the expression.", id.c_str());
                    break;
                }
            }
        }
    );
}

} // namespace style
} // namespace mbgl
