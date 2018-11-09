#ifndef StaticReflectionDataHeaderGenerator_h_Included
#define StaticReflectionDataHeaderGenerator_h_Included

#include "CodeGeneratorBase.h"

#include <string>
#include <fstream>
namespace mc {

    class StaticReflectionDataHeaderGenerator  {
    public:

        void BeginFile();
        void EndFile();
        void process(const clang::CXXRecordDecl *record);
        void process(const clang::EnumDecl *en);
        void process(const clang::FunctionDecl *fct);

        // hack an oiut
        std::ofstream out;
    };
}

#endif // StaticReflectionDataHeaderGenerator_h_Included
