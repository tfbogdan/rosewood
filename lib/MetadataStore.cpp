#include <mc/MetadataStore.h>
#include <mc/MetaClass.h>

#include <iostream>

namespace metal {
    std::vector<const RecordDecl*> metal::Store::recordDefs;

    void Store::registerRecord(const RecordDecl *record) {
        recordDefs.push_back(record);
        // std::cout << "RecordDecl " << record->qualName << " registered to the metadata store;\n";
    }

    
}
