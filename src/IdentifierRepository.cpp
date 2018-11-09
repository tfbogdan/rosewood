#include "IdentifierRepository.h"

#pragma warning(push, 0)
#include <llvm/Support/YAMLParser.h>
#include <llvm/Support/YAMLTraits.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)

#include <fstream>


template <>
struct llvm::yaml::MappingTraits<mc::IdentifierInfo> {
    static void mapping(IO &io, mc::IdentifierInfo &info) {
        io.mapRequired("name", info.name);
        io.mapRequired("defined", info.defined);
    }
};


LLVM_YAML_IS_STRING_MAP(mc::IdentifierInfo)

namespace mc {

    void IdentifierRepository::loadFromFile(const std::string &file) {
        if (llvm::sys::fs::exists(file)) {
            std::ifstream fStream(file, std::ios::ate);
            std::string yamlText;
            yamlText.resize(fStream.tellg());
            fStream.seekg(std::ios::beg);
            fStream.read((char*)yamlText.data(), yamlText.size());
            llvm::yaml::Input ipt(yamlText);
            ipt >> identifierMap;
        }
    }

    void IdentifierRepository::writeToFile(const std::string &file) {
        std::error_code ec;
        llvm::raw_fd_ostream os(file, ec, llvm::sys::fs::OpenFlags::F_Text);

        llvm::yaml::Output output(os);
        output << identifierMap;
        os.flush();
        os.close();
    }

    bool IdentifierRepository::isSymbolDefined(const std::string &name) const {
        auto sRes(identifierMap.find(name));
        if (sRes != identifierMap.end()) {
            return sRes->second.defined;
        }
        return false;
    }

    void IdentifierRepository::defineIdentifier(const std::string &name) {
        if (identifierMap[name].name == std::string()) {
            identifierMap[name].name = name;
        }
        identifierMap[name].defined = true;
    }
}