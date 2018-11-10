#pragma once

#include "CodeGeneratorBase.h"
#include "IdentifierRepository.h"

#pragma warning(push, 0)
#include <clang/AST/AST.h>
#include <llvm/Support/CommandLine.h>
#pragma warning(pop)

#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>
#include <fmt/ostream.h>

extern llvm::cl::opt<std::string> mcModuleName;

namespace mc {

struct scope {
    scope(const scope &other) = default;

    scope(std::ostream &Out, int Indentation)
        :out(Out),
          indentation(Indentation) {}

    void indent() {
        int ind(indentation);
        if (ind > 0) {
            while (ind > 0) {
                out << ' ' << ' ';
                --ind;
            }
        }
    }

    template<typename ...Args>
    void rawput(std::string_view format, Args &&...args) {
        fmt::print(out, format, std::forward<Args&&>(args)...);
    }

    template<typename ...Args>
    void put(std::string_view format, Args &&...args) {
        indent();
        rawput(format, std::forward<Args&&>(args)...);
    }

    template<typename ...Args>
    void putline(std::string_view format, Args &&...args) {
        put(format, std::forward<Args&&>(args)...);
        fmt::print(out, "\n");
    }

    scope spawn() {
        return scope(out, indentation+1);
    }

    /*template <typename scopeK, typename ...scopeArgs>
    scopeK spawn(scopeArgs&& ...args) const {
        return scopeK(*this, std::forward<scopeArgs>(args)...);
    }*/
private:
    std::ostream &out;
protected:
    int indentation;
};

struct descriptor_scope {
   /* descriptor_scope(const std::string &Name, const std::string &QualName, std::ostream& Out, int Indentation)
        :ownScope(Out, Indentation),
         name(Name),
         qualName(QualName) {
        putline("// descriptor for {}\n", qualName);
        putline("struct {} {{\n", name);
        ++indentation;
    }*/

    descriptor_scope(scope Outer, const std::string &Name, const std::string &QualName)
        :outer(Outer),
         inner(outer.spawn()),
         name(Name),
         qualName(QualName) {}

    descriptor_scope(const descriptor_scope &other) = delete;
    descriptor_scope(descriptor_scope &&other)
        :outer(other.outer),
        inner(other.inner),
        name(std::move(other.name)),
        qualName(std::move(other.qualName)) {
        moved = true;
    }

    ~descriptor_scope() {
        if (!moved) {
            print_header();
            outer.putline("}};", name);
        }
    }

    descriptor_scope spawn(const std::string& Name, const std::string& QualName) {
        print_header();
        return descriptor_scope(inner, Name, QualName);
    }

    template<typename ...Args>
    void put(std::string_view format, Args &&...args) {
        print_header();
        inner.put(format, std::forward<Args&&>(args)...);
    }

    template<typename ...Args>
    void putline(std::string_view format, Args &&...args) {
        print_header();
        inner.putline(format, std::forward<Args&&>(args)...);
    }

    void print_header() {
        if (!printed_header) {
            outer.putline("// descriptor for {}", qualName);
            outer.putline("struct {} {{", name);
            printed_header = true;
        }
    }

    scope outer;
    scope inner;
    const std::string name;
    const std::string qualName;
private:
    bool moved = false;
    bool printed_header = false;
};

    class ReflectionDataGenerator {

    public:
        explicit ReflectionDataGenerator(const clang::ASTContext &astContext);
        ~ReflectionDataGenerator();

        void Generate();

        std::vector<const clang::CXXRecordDecl*>    exportedRecords;
        std::vector<const clang::EnumDecl*>         exportedEnums;
        std::vector<const clang::FunctionDecl*>     exportedFunctions;

    private:
        void exportDeclaration(const clang::Decl *Decl, descriptor_scope &where);
        void exportNamespace(const clang::NamespaceDecl *Namespace, descriptor_scope &where);
        void exportEnum(const clang::EnumDecl *Enum, descriptor_scope &where);
        void exportCxxRecord(const clang::CXXRecordDecl *Record, descriptor_scope &where);
        void exportCxxConstructor(const clang::CXXConstructorDecl *Ctor, descriptor_scope &where);
        void exportCxxMethod(const clang::CXXMethodDecl *Method, descriptor_scope &where);

        void genTypeDescriptor(const clang::Type *ty);
        void genMethodCallUnpacker(const clang::CXXMethodDecl *method);
        void genMethodArgDescriptorList(const clang::CXXMethodDecl *method);
        void genMethodDescriptor(const clang::CXXMethodDecl *method);
        void genRecordDescriptor(const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &exportedMethods);
        void genRecordMethodDescriptorList(const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &exportedMethods);
        void genRecordDescBindingToVirtualFcn(const clang::CXXRecordDecl *record);

        std::ofstream out;
        mc::IdentifierHelper idman;
        mc::IdentifierRepository idrepo;

        scope global_scope = scope(out, 0);

        int nesting = 0;

        const clang::ASTContext &context;
        const clang::PrintingPolicy &printingPolicy;
    };


}
