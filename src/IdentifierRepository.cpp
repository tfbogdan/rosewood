#include "IdentifierRepository.h"

#pragma warning(push, 0)
#pragma warning(pop)

#include <fstream>
#include <iomanip>
// #include <nlohmann/json.hpp>

namespace mc {

IdentifierRepository::IdentifierRepository(const fs::path &source) {
    // nlohmann::json json;
    // json.parse(std::ifstream(source));
    // TDO definedIdents = json["defined"];
}

    void IdentifierRepository::save(const fs::path &file) {
        // nlohmann::json json;
        // json["defines"] = definedIdents;
        // json["expects"] = externalIdents;
        // std::ofstream(file) << std::setw(4) << json;
    }

    bool IdentifierRepository::isDefined([[maybe_unused]] const std::string &identifier) const {
        return definedIdents.find(identifier) != definedIdents.end();
    }

    void IdentifierRepository::defineIdentifier(const std::string &identifier) {
        definedIdents.insert(identifier);
    }

    void IdentifierRepository::expectExternalIdentifier(const std::string &identifier) {
        externalIdents.insert(identifier);
    }

}
