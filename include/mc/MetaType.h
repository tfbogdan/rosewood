#pragma once

#include "MetalConfig.h"
#include "QualType.h"

#include <cstdint>

namespace metal {



    class Type {
    public:
        
        typedef enum : std::uint8_t {
            TK_Builtin, 
            TK_Pointer, 
            TK_LValRef,
            TK_RValRef,
            TK_Typedef, 
            TK_DeclTyp,
            TK_EnumType,
            TK_Transparent
        } Kind;

        Type() = delete;
        Type(Kind k, const char *Name)
            :kind(k),
            name(Name) {}

        virtual ~Type();

        Kind getKind() const { return kind; }
    private:
        Kind kind;
        const char *name;
    };

    class TransparentType : public Type {
    public:
        TransparentType(const char *name)
            :Type(Type::Kind::TK_Transparent, name) {}

        virtual ~TransparentType() = default;

    };

    class BuiltinType : public Type {
    public:
        
        typedef enum {
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

        explicit BuiltinType(BuiltinKind k, const char *name)
            :Type(Type::TK_Builtin, name) {}

        virtual ~BuiltinType() = default;

        BuiltinKind getBuiltinKind() const { return kind; }
    private:
        BuiltinKind kind;
    };

    class IndirectionType : public Type {
    public:
        IndirectionType() = delete;
        IndirectionType(QualType t, Type::Kind k, const char *name) :
            Type(k, name),
            pointee(t) {}

        virtual ~IndirectionType() = default;

    private:
        QualType pointee;
    };

    class PointerType : public IndirectionType {
    public:
        PointerType() = delete;
        PointerType(QualType t, const char *name)
            :IndirectionType(t, Type::Kind::TK_Pointer, name) {}

        virtual ~PointerType() = default;

    private:
    };

    class LReferenceType : public IndirectionType {
    public:
        LReferenceType() = delete;
        LReferenceType(QualType t, const char *name)
            :IndirectionType(t, Type::Kind::TK_LValRef, name) {}

        virtual ~LReferenceType() = default;

    private:
    };

    class RReferenceType : public IndirectionType {
    public:
        RReferenceType() = delete;
        RReferenceType(QualType t, const char *name)
            :IndirectionType(t, Type::Kind::TK_RValRef, name) {}

        virtual ~RReferenceType() = default;

    private:
    };

    class TypedefType : public Type {
    public:
        TypedefType() = delete;
        TypedefType(QualType &syn, const char *name)
            :Type(Type::TK_Typedef, name),
            underlying(syn) {}

        virtual ~TypedefType() = default;

    private:
        QualType underlying;
    };


    class TagDecl;
    class RecordDecl;
    class RecordType : public Type {
    public:
        RecordType(const metal::RecordDecl *rec, const char *name)
            :Type(Type::Kind::TK_DeclTyp, name),
            record(rec) {}

        virtual ~RecordType() = default;

    private:
        const metal::RecordDecl *record;
    };

    class EnumDecl;
    class EnumType : public Type {
    public:
        EnumType(const metal::EnumDecl *dec, const char *name)
            :Type(Type::Kind::TK_EnumType, name),
            decl(dec) {}

        virtual ~EnumType() = default;
    private:
        const metal::EnumDecl *decl;
    };


}

