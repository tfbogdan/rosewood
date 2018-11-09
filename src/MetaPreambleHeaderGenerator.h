#ifndef MetaPreambleHeaderGenerator_h_Included
#define MetaPreambleHeaderGenerator_h_Included

#include "CodeGeneratorBase.h"
namespace mc {

    class MetaPreambleCodeGenerator  {
    public:
        void BeginFile();
        void EndFile();
        void process(const clang::CXXRecordDecl *record);
        void process(const clang::EnumDecl *en);
        void process(const clang::FunctionDecl *fct);
    };
}

#endif // MetaPreambleHeaderGenerator_h_Included
