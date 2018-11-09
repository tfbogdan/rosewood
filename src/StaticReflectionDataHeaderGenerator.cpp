#include "StaticReflectionDataHeaderGenerator.h"

#pragma warning(push,0)
#include <clang/AST/AST.h>
#pragma warning(pop)

#include <stack>


namespace mc {/*

    void StaticReflectionDataHeaderGenerator::BeginFile() {
        out << "#include <Reflection.h>" << std::endl;
        out << "#include <MetaClass.h>" << std::endl;
        out << std::endl << std::endl << std::endl;
        // out << "#include <" << inFileName << ">" << std::endl;
    }

    void StaticReflectionDataHeaderGenerator::EndFile() {

    }

    void StaticReflectionDataHeaderGenerator::process(const clang::CXXRecordDecl *record) {
        // for this class, gather all methods
        // gather all properties
        std::vector<const clang::CXXMethodDecl*> methods;
        for (const auto &decl : record->decls()) {
            if (decl->hasAttr<clang::AnnotateAttr>() && decl->getAttr<clang::AnnotateAttr>()->getAnnotation() == "mc_export") {
                if (decl->getKind() == clang::Decl::Kind::CXXMethod) {
                    methods.push_back(static_cast<const clang::CXXMethodDecl*>(decl));
                }
            }
        }
        
        out << "template <> struct metal::InvokableDispatcher<" << record->getQualifiedNameAsString() << "> { " << std::endl;
        out << "// method unpackers" << std::endl;
        for (const auto &method : methods) {
            std::string unpackerName("unpackCall_");
            unpackerName += genIdentifier(static_cast<const clang::Decl*>(method));
            out << "\t static void " << unpackerName << "(void *o, int argc, void **argv, void *ret);" << std::endl;
        }
        out << "}; " << std::endl << std::endl << std::endl;
        std::stack<const clang::DeclContext *> resolution_path;
        const clang::DeclContext *c(record);
        while (c->getParent() != nullptr) {
            c = c->getParent();
            resolution_path.push(c);
        }
        auto doIndent = [&](int i){
            while (i > 0) {
                out << "\t";
                --i;
            }
        };
        int indent(0);
        while (!resolution_path.empty()) {
            switch (resolution_path.top()->getDeclKind()) {
            case clang::Decl::Kind::Namespace:
            {
                const clang::NamespaceDecl *n = static_cast<const clang::NamespaceDecl*>(resolution_path.top());
                doIndent(indent);
                out << "namespace " << n->getNameAsString() << "{ " << std::endl;
                ++indent;
            } break;
            default:
                break;
            }
            resolution_path.pop();
        }

        while (indent) {
            doIndent(indent - 1);
            out << "}" << std::endl;
            --indent;
        }
        out << std::endl << std::endl;
    }

    void StaticReflectionDataHeaderGenerator::process(const clang::EnumDecl *en) {

    }

    void StaticReflectionDataHeaderGenerator::process(const clang::FunctionDecl *fct) {

    }
*/

}
