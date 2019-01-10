/*****************************************************************************************
 *                                                                                       *
 * OpenSpace                                                                             *
 *                                                                                       *
 * Copyright (c) 2014-2018                                                               *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/

#include <modules/base/dashboard/dashboarditempropertyvalue.h>

#include <openspace/documentation/documentation.h>
#include <openspace/documentation/verifier.h>
#include <openspace/engine/globals.h>
#include <openspace/util/timemanager.h>
#include <openspace/query/query.h>
#include <ghoul/font/font.h>
#include <ghoul/font/fontmanager.h>
#include <ghoul/font/fontrenderer.h>
#include <iostream>

namespace {
    constexpr const char* KeyFontMono = "Mono";
    constexpr const float DefaultFontSize = 10.f;

    constexpr openspace::properties::Property::PropertyInfo FontNameInfo = {
        "FontName",
        "Font Name",
        "This value is the name of the font that is used. It can either refer to an "
        "internal name registered previously, or it can refer to a path that is used."
    };

    constexpr openspace::properties::Property::PropertyInfo FontSizeInfo = {
        "FontSize",
        "Font Size",
        "This value determines the size of the font that is used to render the date."
    };

    constexpr openspace::properties::Property::PropertyInfo PropertyUriInfo = {
        "URI",
        "Property URI",
        "The URI of the property that is displayed in this dashboarditem"
    };

    constexpr openspace::properties::Property::PropertyInfo DisplayStringInfo = {
        "DisplayString",
        "Display String",
        "The String that is being displayed. It must either be empty (in which case only "
        "the value itself will be displayed), or it must contain extact one instance of "
        "{}, which will be replaced with the value of the property during rendering."
    };

    constexpr openspace::properties::Property::PropertyInfo IsColorPropertyInfo = {
        "IsColorProperty",
        "Is Color Property",
        "Can be set to true to show the property as a colored marker instead of text. "
    };
} // namespace

namespace openspace {

documentation::Documentation DashboardItemPropertyValue::Documentation() {
    using namespace documentation;
    return {
        "DashboardItem PropertyValue",
        "base_dashboarditem_propertyvalue",
        {
            {
                "Type",
                new StringEqualVerifier("DashboardItemPropertyValue"),
                Optional::No
            },
            {
                FontNameInfo.identifier,
                new StringVerifier,
                Optional::Yes,
                FontNameInfo.description
            },
            {
                FontSizeInfo.identifier,
                new IntVerifier,
                Optional::Yes,
                FontSizeInfo.description
            },
            {
                PropertyUriInfo.identifier,
                new StringVerifier,
                Optional::Yes,
                PropertyUriInfo.description
            },
            {
                DisplayStringInfo.identifier,
                new StringVerifier,
                Optional::Yes,
                DisplayStringInfo.description
            },
            {
                IsColorPropertyInfo.identifier,
                new BoolVerifier,
                Optional::Yes,
                IsColorPropertyInfo.description
            },
        }
    };
}

DashboardItemPropertyValue::DashboardItemPropertyValue(
                                                      const ghoul::Dictionary& dictionary)
    : DashboardItem(dictionary)
    , _fontName(FontNameInfo, KeyFontMono)
    , _fontSize(FontSizeInfo, DefaultFontSize, 6.f, 144.f, 1.f)
    , _propertyUri(PropertyUriInfo)
    , _displayString(DisplayStringInfo)
{
    documentation::testSpecificationAndThrow(
        Documentation(),
        dictionary,
        "DashboardItemPropertyValue"
    );

    if (dictionary.hasKey(IsColorPropertyInfo.identifier)) {
        _isColorProperty = dictionary.value<bool>(IsColorPropertyInfo.identifier);
    }

    if (dictionary.hasKey(FontNameInfo.identifier)) {
        _fontName = dictionary.value<std::string>(FontNameInfo.identifier);
    }
    _fontName.onChange([this](){
        _font = global::fontManager.font(_fontName, _fontSize);
    });
    addProperty(_fontName);

    if (dictionary.hasKey(FontSizeInfo.identifier)) {
        _fontSize = static_cast<float>(
            dictionary.value<double>(FontSizeInfo.identifier)
        );
    }
    _fontSize.onChange([this](){
        _font = global::fontManager.font(_fontName, _fontSize);
    });
    addProperty(_fontSize);

    if (dictionary.hasKey(PropertyUriInfo.identifier)) {
        _propertyUri = dictionary.value<std::string>(PropertyUriInfo.identifier);
    }
    _propertyUri.onChange([this]() {
        _propertyIsDirty = true;
    });
    addProperty(_propertyUri);

    if (dictionary.hasKey(DisplayStringInfo.identifier)) {
        _displayString = dictionary.value<std::string>(DisplayStringInfo.identifier);
    }
    addProperty(_displayString);

    _font = global::fontManager.font(_fontName, _fontSize);
}

void DashboardItemPropertyValue::render(glm::vec2& penPosition) {
    
    if (_propertyIsDirty) {
        _property = openspace::property(_propertyUri);
        _propertyIsDirty = false;
    }

    if (_property) {

        std::string valueStr;
        _property->getStringValue(valueStr);

        penPosition.y -= _font->height();
        RenderFont(
            *_font,
            penPosition,
            fmt::format(_displayString.value(), valueStr)
        );

        if (_isColorProperty) {

            glm::vec4 color(1.0f, 0.0f, 0.0f, 1.0f);

            std::string commaSeparatedValues = valueStr.substr(1, valueStr.size() - 2);
            std::vector<float> colorValVec;

            std::stringstream ss(commaSeparatedValues);

            float strVal;
            while (ss >> strVal)
            {
                if (ss.peek() == ',')
                    ss.ignore();
                colorValVec.push_back(strVal);
            }

            color.r = colorValVec.at(0);
            color.g = colorValVec.at(1);
            color.b = colorValVec.at(2);
            color.a = 0.6 * colorValVec.at(3);

            _fontColorDot = global::fontManager.font(_fontName, _fontSize * 4.0);

            std::string colorDot = ".";

            glm::vec2 offset = _font->boundingBox(fmt::format(_displayString.value() + " ", valueStr));

            RenderFont(
                *_fontColorDot,
                glm::vec2(offset.x, penPosition.y),
                colorDot,
                color
            );
        }
    }
}

glm::vec2 DashboardItemPropertyValue::size() const {
    return ghoul::fontrendering::FontRenderer::defaultRenderer().boundingBox(
        *_font,
        _displayString.value()
    ).boundingBox;
}

} // namespace openspace
