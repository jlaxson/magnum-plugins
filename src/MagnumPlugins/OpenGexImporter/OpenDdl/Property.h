#ifndef Magnum_OpenDdl_Property_h
#define Magnum_OpenDdl_Property_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

/** @file
 * @brief Class @ref Magnum::OpenDdl::Property
 */

#include <functional>
#include <Corrade/Utility/Assert.h>
#include <Magnum/Magnum.h>

#include "MagnumPlugins/OpenGexImporter/OpenDdl/Document.h"

namespace Magnum { namespace OpenDdl {

namespace Implementation { class PropertyIterator; }

/**
@brief OpenDDL property

See @ref Document for more information.

@attention The class consists just of reference to internal data in originating
    @ref Document instance, thus you must ensure that the document is available
    for whole instance lifetime. On the other hand you can copy the instance
    however you like without worrying about performance.

@see @ref Structure::properties()
*/
class Property {
    /* GCC 4.6 needs the class keyword */
    friend class Structure;
    friend class Implementation::PropertyIterator;

    public:
        /**
         * @brief Property identifier
         *
         * @see @ref UnknownIdentifier
         */
        Int identifier() const { return _data.get().identifier; }

        /** @brief Whether property type is compatible with given type */
        bool isTypeCompatibleWith(PropertyType type) const;

        /**
         * @brief Property data
         *
         * The property type must be compatible with desired type.
         * @see @ref isTypeCompatibleWith(), @ref asReference()
         */
        template<class T>
        #ifndef DOXYGEN_GENERATING_OUTPUT
        typename Implementation::ReturnTypeFor<T>::Type
        #else
        const T&
        #endif
        as() const;

        /**
         * @brief Reference property data
         *
         * The property type must be @ref Type::Reference. Returns referenced
         * structure or `std::nullopt` if the reference is `null`.
         * @see @ref isTypeCompatibleWith()
         */
        std::optional<Structure> asReference() const;

    private:
        explicit Property(const Document& document, std::size_t i) noexcept: _document{document}, _data{document._properties[i]} {}

        std::reference_wrapper<const Document> _document;
        std::reference_wrapper<const Document::PropertyData> _data;
};

namespace Implementation {
    template<class T> constexpr bool isPropertyType(InternalPropertyType) {
        static_assert(sizeof(T) == 0, "Invalid property type"); return false;
    }
    template<> constexpr bool isPropertyType<bool>(InternalPropertyType type) {
        return type == InternalPropertyType::Bool;
    }
    template<> constexpr bool isPropertyType<Int>(InternalPropertyType type) {
        return type == InternalPropertyType::Integral || type == InternalPropertyType::Binary || type == InternalPropertyType::Character;
    }
    template<> constexpr bool isPropertyType<Float>(InternalPropertyType type) {
        return type == InternalPropertyType::Float;
    }
    template<> constexpr bool isPropertyType<std::string>(InternalPropertyType type) {
        return type == InternalPropertyType::String || type == InternalPropertyType::Reference;
    }
}

template<class T>
#ifndef DOXYGEN_GENERATING_OUTPUT
typename Implementation::ReturnTypeFor<T>::Type
#else
const T&
#endif
Property::as() const {
    CORRADE_ASSERT(Implementation::isPropertyType<T>(_data.get().type),
        "OpenDdl::Property::as(): not compatible with given type", _document.get().data<T>().front());
    return _document.get().data<T>()[_data.get().position];
}

namespace Implementation {

class PropertyIterator {
    public:
        explicit PropertyIterator(const Document& document, std::size_t i) noexcept: _document{document}, _i{i} {}

        Property operator*() const {
            return Property{_document, _i};
        }
        bool operator!=(const PropertyIterator& other) const {
            return _i != other._i || &_document.get() != &other._document.get();
        }
        PropertyIterator& operator++() {
            ++_i;
            return *this;
        }

    private:
        std::reference_wrapper<const Document> _document;
        std::size_t _i;
};

class PropertyList {
    public:
        explicit PropertyList(const Document& document, std::size_t begin, std::size_t size) noexcept: _document{document}, _begin{begin}, _end{begin + size} {}

        PropertyIterator begin() const { return PropertyIterator{_document, _begin}; }
        PropertyIterator cbegin() const { return begin(); }
        PropertyIterator end() const { return PropertyIterator{_document, _end}; }
        PropertyIterator cend() const { return end(); }

    private:
        std::reference_wrapper<const Document> _document;
        std::size_t _begin, _end;
};

}

}}

#endif
