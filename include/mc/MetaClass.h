#pragma once


#include "MetalConfig.h"
#include "QualType.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace metal {
    class Function;
    class Enum;
    class Property;
    class Property;

    class Decl {
    public:
        typedef enum : int8_t {
            Record, 
            Enum, 
            Method,
            Property,
            Namespace
        } Kind;

        constexpr Decl(Kind k, std::string_view Id)
            :identifier(Id),
            kind(k) {}

        constexpr inline Kind getKind() const { return kind; }
    private:
        const std::string_view identifier;
        const Kind kind;
    };

    class NamespaceDecl : public Decl {
    public:
        constexpr NamespaceDecl(std::string_view Name)
            :Decl(Decl::Kind::Namespace, Name) {}

    private:
    };

    struct Argument {
        const std::string_view name;
        const metal::QualType type;
    };

    template< unsigned numArgs>
    class MethodDecl : public Decl {
    public:
        constexpr MethodDecl(std::string_view Signature, std::string_view Name, const Type& Ret ,std::array<Argument, numArgs> &&Args, bool isConst, bool isVirtual)
            :Decl(Decl::Kind::Method, Name),
             signature(Signature),
             returns(Ret),
             args(Args),
             is_const (isConst),
             is_virtual(isVirtual) {}

        const std::string_view signature;
        const Type& returns;
        std::array<Argument, numArgs> args;
        const bool is_const : 1;
        const bool is_virtual : 1;
    };

    class RecordDecl : public Decl {
    public:
        constexpr RecordDecl(std::string_view name)
            :Decl(Decl::Kind::Record, name) {}

    private:
    };

    class FieldDecl {
        const std::string_view name;
        const metal::QualType type;
    };

    template<unsigned numCtors, unsigned numMethods, unsigned numFields>
    class RecordDeclImpl : public RecordDecl {
    public:

    };

    struct Enumerator {
        constexpr Enumerator(std::string_view Name, std::int64_t V)
            :name(Name), value(V) {}

        const std::string_view name;
        const std::int64_t    value;
    };

    template<unsigned numEnumerators>
    class EnumDecl : public Decl {
    public:
        constexpr EnumDecl(std::string_view Name, std::array<Enumerator, numEnumerators> &&enums)
            :Decl(Decl::Kind::Enum, Name),
            enumerators(enums) {}

        const std::array<Enumerator, numEnumerators> enumerators;
    };


}

