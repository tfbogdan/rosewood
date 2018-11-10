#pragma once

#include "MetalConfig.h"
#include "QualType.h"

#include <cstdint>
#include <functional>
#include <string_view>

namespace metal {



    class Type {
    public:
        
        typedef enum : int8_t {
            TK_Builtin, 
            TK_Pointer, 
            TK_LValRef,
            TK_RValRef,
            TK_Typedef, 
            TK_DeclTyp,
            TK_EnumType,
            TK_Transparent
        } Kind;

        constexpr Type(Kind k, std::string_view Name)
            :kind(k),
            name(Name) {}

        constexpr inline Kind getKind() const { return kind; }
        constexpr inline std::string_view getName() const { return name; }
    private:
        const Kind kind;
        const std::string_view name;
    };

    class TransparentType : public Type {
    public:
        constexpr TransparentType(std::string_view name)
            :Type(Type::Kind::TK_Transparent, name) {}

    };

    class BuiltinType : public Type {
    public:
        
        typedef enum : int8_t {
            BK_Void, 
            BK_Char,
            BK_UChar, 
            BK_WChar,
            BK_UWChar, 
            BK_Short, 
            BK_UShort, 
            BK_Int, 
            BK_UInt, 
            BK_Long,
            BK_ULong, 
            BK_LongLong,
            BK_ULongLong, 
            BK_Float, 
            BK_Double, 
            BK_Bool
        } BuiltinKind;    

        constexpr BuiltinType(BuiltinKind k, std::string_view name)
            :Type(Type::TK_Builtin, name),
            kind(k) {}

        BuiltinKind getBuiltinKind() const { return kind; }
    private:
        const BuiltinKind kind;
    };

    class IndirectionType : public Type {
    public:
        constexpr IndirectionType(QualType t, Type::Kind k, std::string_view name) :
            Type(k, name),
            pointee(t) {}

    private:
        const QualType pointee;
    };

    class PointerType : public IndirectionType {
    public:
        constexpr PointerType(QualType t, std::string_view name)
            :IndirectionType(t, Type::Kind::TK_Pointer, name) {}

    private:
    };

    class LReferenceType : public IndirectionType {
    public:
        constexpr LReferenceType(QualType t, std::string_view name)
            :IndirectionType(t, Type::Kind::TK_LValRef, name) {}

    private:
    };

    class RReferenceType : public IndirectionType {
    public:
        RReferenceType(QualType t, std::string_view name)
            :IndirectionType(t, Type::Kind::TK_RValRef, name) {}
    private:
    };

    class TypedefType : public Type {
    public:
        constexpr TypedefType(QualType syn, std::string_view name)
            :Type(Type::TK_Typedef, name),
            underlying(syn) {}
    private:
        const QualType underlying;
    };


    class TagDecl;
    class RecordDecl;

    class RecordType : public Type {
    public:
        constexpr RecordType(const metal::RecordDecl &rec, std::string_view name)
            :Type(Type::Kind::TK_DeclTyp, name),
            record(rec) {}

    private:
        const metal::RecordDecl &record;
    };

    template<typename EnumDeclType>
    class EnumType : public Type {
    public:
        constexpr EnumType(const EnumDeclType &dec, std::string_view name)
            :Type(Type::Kind::TK_EnumType, name),
            decl(dec) {}

    private:
        const EnumDeclType &decl;
    };


}


namespace mc {

    struct nil_t {};
    template <typename T>
    struct type_map {};

    template <typename tuple_t, unsigned tuple_pos>
    struct inspect_element {

    };

    template<typename tuple_t>
    struct descent_into_tuple {
        template <typename query>
        constexpr bool any_matches(query q);
    };

    template <typename T>
    bool constexpr name_matches([[maybe_unused]]T val, std::string_view name) {
        return T::name == name;
    }

    template<typename MetaType>
    bool constexpr has_method([[maybe_unused]] MetaType metaType, std::string_view name) {
        using meta_type = MetaType;
        using methods_tuple_type = typename meta_type::methods;
        bool found_method = false;
        std::apply([&name, &found_method](auto ...meth) {
            found_method = found_method || ((name_matches(meth, name), ...));
        }, methods_tuple_type());
        return found_method;
    }

    struct type {
        static constexpr bool is_type = true;
    };

    struct Namespace {
        static constexpr bool is_namespace = true;
    };

    struct builtin_void : public type {
        static constexpr std::string_view name = "void";
    };

    struct builtin_char : public type {
        static constexpr std::string_view name = "char";
    };

    struct builtin_uchar : public type {
        static constexpr std::string_view name = "unsigned char";
    };

    struct builtin_wchar : public type {
        static constexpr std::string_view name = "wchar_t";
    };

    struct builtin_short : public type {
        static constexpr std::string_view name = "short";
    };

    struct builtin_ushort : public type {
        static constexpr std::string_view name = "unsigned short";
    };

    struct builtin_int : public type {
        static constexpr std::string_view name = "int";
    };

    struct builtin_uint : public type {
        static constexpr std::string_view name = "unsigned int";
    };

    struct builtin_long : public type {
        static constexpr std::string_view name = "long";
    };

    struct builtin_ulong : public type {
        static constexpr std::string_view name = "unsigned long";
    };

    struct builtin_longlong : public type {
        static constexpr std::string_view name = "long long";
    };

    struct builtin_ulonglong : public type {
        static constexpr std::string_view name = "unsigned long long";
    };

    struct builtin_float : public type {
        static constexpr std::string_view name = "float";
    };

    struct builtin_double : public type {
        static constexpr std::string_view name = "double";
    };

    struct builtin_bool : public type {
        static constexpr std::string_view name = "bool";
    };

}
