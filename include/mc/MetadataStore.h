#ifndef MetadataStore_h_Included
#define MetadataStore_h_Included

#include <vector>

namespace metal {
    class RecordDecl; 

    class Store {
    public:
        static void registerRecord(const RecordDecl *record);
    private:
        static std::vector<const RecordDecl*> recordDefs;

    };

}

#endif // MetadataStore_h_Included
