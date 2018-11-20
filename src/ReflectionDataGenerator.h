#pragma once

#include "IdentifierHelper.h"
#include "IdentifierRepository.h"

#pragma warning(push, 0)
#include <clang/AST/AST.h>
#include <llvm/Support/CommandLine.h>
#include <clang/Sema/Sema.h>
#pragma warning(pop)

#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>
#include <fmt/ostream.h>

inline std::ostream& operator<< (std::ostream& os, const llvm::StringRef& v) {
    for (auto c: v) {
        os << c;
    }
}

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

private:
    std::ostream &out;
protected:
    int indentation;
};

struct descriptor_scope {

    descriptor_scope(scope Outer, const std::string &Name, const std::string &Kind, const std::string &parentQualName = "")
        :outer(Outer),
         inner(outer.spawn()),
         name(Name),
         kind(Kind),
         qualifiedName(fmt::format("{}::meta_{}", parentQualName.empty() ? "rosewood" : parentQualName, Name)) {}

    descriptor_scope(const descriptor_scope &other) = delete;
    descriptor_scope(descriptor_scope &&other)
        :outer(other.outer),
        inner(other.inner),
        name(std::move(other.name)),
        kind(std::move(other.kind)),
        qualifiedName(std::move(other.kind)),
		printed_header(other.printed_header) {
        other.moved = true;
    }

    ~descriptor_scope() {
        if (!name.empty() && !moved) {
            print_header();
            outer.putline("}};", name);
        }
    }

    descriptor_scope spawn(const std::string& Name, const std::string &Kind) {
        print_header();
        return descriptor_scope(inner, Name, Kind, qualifiedName);
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
        if (!printed_header && !name.empty()) {
            outer.putline("");
            outer.putline("struct meta_{} : public {}<meta_{}> {{", name, kind, name);
            inner.putline("static constexpr std::string_view name = \"{}\";", name);
            printed_header = true;
        }
    }

    scope spawn() {
        print_header();
        return outer.spawn();
    }

    scope outer;
    scope inner;
    const std::string name;
    const std::string kind;
    const std::string qualifiedName; // holds a path that leads to this descriptor from the global namespace
private:
    bool moved = false;
    bool printed_header = false;
};


    class ReflectionDataGenerator {

    public:
        explicit ReflectionDataGenerator(clang::ASTContext &astContext, clang::Sema &sema);
        ~ReflectionDataGenerator();

        void Generate();

    private:
        clang::QualType getUnitType(clang::QualType T);
        void exportType(const std::string &exportAs, clang::QualType type, descriptor_scope &where);
        descriptor_scope exportDeclaration(const clang::Decl *Decl, descriptor_scope &where);

        descriptor_scope exportNamespace(const clang::NamespaceDecl *Namespace, descriptor_scope &where);
        descriptor_scope exportEnum(const clang::EnumDecl *Enum, descriptor_scope &where);
        descriptor_scope exportCxxRecord(const std::string &name, const clang::CXXRecordDecl *Record, descriptor_scope &where);

        descriptor_scope exportCxxMethod(const std::string &name, const clang::CXXRecordDecl *record, const clang::CXXMethodDecl* method, descriptor_scope &where);
        descriptor_scope exportCxxMethodGroup(const std::string &name, const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &overloads, descriptor_scope &where);
        descriptor_scope exportCxxOperator(const std::string &name, const clang::CXXRecordDecl *record, const std::vector<const clang::CXXMethodDecl*> &overloads, descriptor_scope &where);
        descriptor_scope exportCxxStaticOperator(const std::string &name, const std::vector<const clang::FunctionDecl*> &overloads, descriptor_scope &where);
        descriptor_scope exportFunctions(const std::string &name, const std::vector<const clang::FunctionDecl*> &overloads, descriptor_scope &where);
        descriptor_scope exportCxxConstructors(const std::vector<const clang::CXXConstructorDecl*> &overloads, const clang::CXXRecordDecl *record, descriptor_scope &where);
        descriptor_scope exportCxxDestructor(const clang::CXXDestructorDecl *Dtor, const clang::CXXRecordDecl *record, descriptor_scope &where);
        void exportFields(const std::vector<const clang::FieldDecl*> &fields, descriptor_scope &where);

        void genMethodCallUnpacker(const clang::CXXMethodDecl *method);

        std::ofstream out;
        mc::IdentifierHelper idman;
        mc::IdentifierRepository idrepo;

        scope global_scope = scope(out, 0);

        std::vector<std::tuple<std::string, std::string>> exportedMetaTypes; // all enums and classes get one of these. more to come

        clang::ASTContext &context;
        clang::Sema &sema;
        clang::PrintingPolicy printingPolicy;
    };


}

