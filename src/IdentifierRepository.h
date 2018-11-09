#ifndef IdentifierRepository_H_Included
#define IdentifierRepository_H_Included

#pragma warning(push, 0)

#pragma warning(pop)
#include <string>
#include <vector>
#include <map>

namespace mc {

    struct IdentifierInfo {
        std::string name;
        bool defined = false;
    };

    class IdentifierRepository {
    public: 
        IdentifierRepository() = default;

        void loadFromFile(const std::string &file);
        void writeToFile(const std::string &file);

        bool isSymbolDefined(const std::string &name) const;
        void defineIdentifier(const std::string &name);
    private:

        std::string fileName;
        std::map<std::string, IdentifierInfo> identifierMap;
    };

}

#endif // IdentifierRepository_H_Included
