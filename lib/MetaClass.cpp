#include <mc/MetaClass.h>
#include <mc/MetadataStore.h>

namespace metal {

    RecordDecl::RecordDecl( const char *name,
                            const Type* type)
        : metal::TypeDecl(type, Decl::Kind::Record, name) {
    
        metal::Store::registerRecord(this);
    }

    TypeDecl::TypeDecl(const Type *t, Decl::Kind k, const char *Id)
        :Decl(k, Id),
        type(t) {

    }

    MethodDecl::MethodDecl( const char *Signature,
                            const char *Name,
                            std::initializer_list<Argument> &&Args, 
                            metal::methodCallUnpacker_t unpacker, 
                            bool isConst, 
                            bool isVirtual)
        :Decl(Decl::Kind::Method, Signature),
        name(Name),
        args(std::forward<std::initializer_list<Argument> &&>(Args)),
        unpacker(unpacker),
        is_const(isConst),
        is_virtual(isVirtual) {}

    EnumDecl::EnumDecl(const char *Name, std::initializer_list<Enumerator> &&Enumerators, const Type *type)
        :TypeDecl(type, Decl::Kind::Enum, Name),
        enumerators(std::forward<std::initializer_list<Enumerator> &&>(Enumerators)) {
    }

    NamespaceDecl::NamespaceDecl(const char *Name) 
        : Decl(Decl::Kind::Namespace, Name) {
    }

}