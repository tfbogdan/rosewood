#ifndef QualType_H_Included
#define QualType_H_Included

#include "MetalConfig.h"

namespace metal {
    class Type;

    class __MC(qual_type) QualType {
    public:
        QualType() = delete;
        __MC(ctor) QualType(bool isConst, bool isVolatile, bool isRestrict, const Type *t)
            :is_const(isConst),
            is_volatile(isVolatile),
            is_restrict(isRestrict),
            typ(t) {}


    private:
        bool is_const : 1;
        bool is_volatile : 1;
        bool is_restrict : 1;
        const Type *typ;

    };

}

#endif // QualType_H_Included
