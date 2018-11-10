#pragma once

#pragma warning(push, 0)

#pragma warning(pop)

#include <string>
#include <vector>
#include <map>
#include <set>
#include <experimental/filesystem>

namespace mc {
    namespace fs = std::experimental::filesystem;

    struct IdentifierInfo {
        std::string name;
        bool defined = false;
    };

    /**
     * @brief The IdentifierRepository class has a purpose but it's still an abstract concept: must keep track of what a module exports and write that data to a json.
     */
    class IdentifierRepository {
    public: 
        explicit IdentifierRepository(const fs::path &source);
        IdentifierRepository() = default;

        void save(const fs::path &file);

        bool isDefined(const std::string &identifier) const;
        void defineIdentifier(const std::string &identifier);
        void expectExternalIdentifier(const std::string &identifier);

    private:

        std::set<std::string> definedIdents;
        std::set<std::string> externalIdents;
        std::map<std::string, IdentifierInfo> identifierMap;
    };

}

