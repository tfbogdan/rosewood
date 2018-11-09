#ifndef MetaClass_H_Included
#define MetaClass_H_Included



#include "MetalConfig.h"
#include "QualType.h"

#include <vector>
#include <cstdint>

namespace metal {
    class MethodDecl;
    class Function;
    class Enum;
    class Property;
    class Argument;
    class Property;

    class Decl {
    public:
        typedef enum {
            Record, 
            Enum, 
            Method,
            Property,
            Namespace
        } Kind;

        Decl() = delete;
        Decl(const Decl &) = delete;
        Decl(Decl &&) = delete;
        Decl &operator=(const Decl &) = delete;
        Decl &operator=(Decl &&) = delete;

        Decl(Kind k, const char * Id)
            :kind(k),
            identifier(Id) {}

        virtual ~Decl() = default;

        Kind getKind() const { return kind; }
    private:

        const char *identifier;
        Kind kind;
    };

    class NamespaceDecl : public Decl {
    public:
        NamespaceDecl() = delete;
        NamespaceDecl(const char *Name);
        virtual ~NamespaceDecl() = default;


    private:

    };

    class TypeDecl : public Decl {
    public:
        TypeDecl() = delete;
        TypeDecl(const Type *t, Decl::Kind k, const char *Id);

        virtual ~TypeDecl() = default;


    private:
        const Type *type;
    };


    class RecordDecl : public TypeDecl {
    public:
        RecordDecl( const char *name,
                    const Type *type);

        virtual ~RecordDecl() = default;


    private:
    };

    class Argument {
        const char *name;
        metal::QualType type;
    };

    class MethodDecl : public Decl {
    public:
        MethodDecl() = delete;
        MethodDecl(const char *Signature, const char *Name, std::initializer_list<Argument> &&Args, metal::methodCallUnpacker_t unpacker, bool isConst, bool isVirtual);

        virtual ~MethodDecl() = default;


    private:

        const char *name;
        std::vector<Argument> args;
        metal::methodCallUnpacker_t unpacker;
        bool is_const : 1;
        bool is_virtual : 1;
    };

    class PropertyDecl {
        const char *name;
        metal::QualType type;
        MethodDecl *getter;
        MethodDecl *setter;
        MethodDecl *notify;
    };

    class Enumerator {
        const char *name;
        std::int64_t    val;
    };

    class EnumDecl : public TypeDecl {
    public:
        EnumDecl() = delete;
        EnumDecl(const char *Name, std::initializer_list<Enumerator> &&Enumerators, const Type *type);

        virtual ~EnumDecl() = default;
    private:
        std::vector<Enumerator> enumerators;
    };


}

#endif // MetaClass_H_Included
