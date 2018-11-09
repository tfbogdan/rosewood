#include "SourceElements.h"


#pragma warning(push, 0)

#pragma warning(pop)


namespace mc {

    void IncludeDirective::toCode(llvm::raw_ostream &os) const {
            llvm::SmallVector<char, 128> directive;
            llvm::raw_svector_ostream o(directive);
            o << "#include ";
            if (is_local) {
                o << "\"";
            }
            else {
                o << "<";
            }
            o << file;
            if (is_local) {
                o << "\"";
            }
            else {
                o << ">";
            }
            o << "\n";
            os << directive;
        }

    void VariableDeclaration::toCode(llvm::raw_ostream &os) const {
        llvm::SmallVector<char, 512> vcode;
        llvm::raw_svector_ostream o(vcode);
        
        o << "extern "; 
        o << type;
        o << " ";
        o << identifier;
        o << ";\n";

        os << vcode;
    }

    void VariableDefinition::toCode(llvm::raw_ostream &os) const {
        llvm::SmallVector<char, 512> vcode;
        llvm::raw_svector_ostream o(vcode);

        o << type;
        o << " ";
        o << identifier;
        o << "(\n";
        for (unsigned idx(0); idx < initializers.size(); ++idx) {
            o << "\t";
            initializers[idx]->toCode(o);
            if (idx < initializers.size() - 1) {
                o << ", \n";
            }
        }
        o << ");\n";
        os << vcode;
    }

    void AnonymousVariable::toCode(llvm::raw_ostream &o) const {
        o << type;
        o << "(\n";
        for (unsigned idx(0); idx < initializers.size(); ++idx) {
            o << "\t";
            initializers[idx]->toCode(o);
            if (idx < initializers.size() - 1) {
                o << ", \n";
            }
        }
        o << ")";
    }

    void LiteralExpression::toCode(llvm::raw_ostream &o) const {
        o << expression;
    }

    void BoolLiteral::toCode(llvm::raw_ostream &o) const {
        if (value) o << "true";
        else o << "false";
    }

    void AddressOf::toCode(llvm::raw_ostream &o) const {
        o << "&" << ident;
    }

    void CStringLiteral::toCode(llvm::raw_ostream &o) const {
        o << "\"" << value << "\"";
    }
}
