#ifndef RuntimeReflectionDataSourceGenerator_h_Included
#define RuntimeReflectionDataSourceGenerator_h_Included

#include "CodeGeneratorBase.h"

namespace clang {
    class CXXMethodDecl;
}

namespace mc {

    class RuntimeReflectionDataSourceGenerator {
    public:

        void BeginFile();
        void EndFile();
        void process(const clang::CXXRecordDecl *record);
        void process(const clang::EnumDecl *en);
        void process(const clang::FunctionDecl *fct);

    private:
        void genTypeDescriptor(const clang::Type *ty);
        void genMethodCallUnpacker(const clang::CXXMethodDecl *method);
        void genMethodArgDescriptorList(const clang::CXXMethodDecl *method);
        void genMethodDescriptor(const clang::CXXMethodDecl *method);
        void genRecordDescriptor(const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &exportedMethods);
        void genRecordMethodDescriptorList(const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &exportedMethods);
        void genRecordDescBindingToVirtualFcn(const clang::CXXRecordDecl *record);
    };

}

#endif // RuntimeReflectionDataSourceGenerator_h_Included
