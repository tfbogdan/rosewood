#ifndef MetaContext_h_Included
#define MetaContext_h_Included

#include <set>
#include <string>
#include <vector>

#include "IdentifierRepository.h"


namespace clang {
    class CXXRecordDecl;
    class CXXMethodDecl;
    class FieldDecl;
}

namespace mc {

    /**
    *   A class is a record type that has invokable methods. 
    *   this requirement has to be satisfied by all base records as well
    */
    struct ClassExportInfo {

    };
    /**
    *   A strict is a record type that only exports field information. 
    *   
    */
    struct StructExportInfo {

    };

    struct Context {
        // std::set<std::string> definedIdentifiers;
        mc::IdentifierRepository identRepository;
    };

}

#endif // MetaContext_h_Included
