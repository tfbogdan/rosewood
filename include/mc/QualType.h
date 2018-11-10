#ifndef QualType_H_Included
#define QualType_H_Included

#include "MetalConfig.h"

namespace metal {
    class Type;

    class QualType {
    public:
        constexpr QualType(bool isConst, bool isVolatile, bool isRestrict, const Type &t)
            :is_const(isConst),
            is_volatile(isVolatile),
            is_restrict(isRestrict),
            typ(t) {}

        constexpr inline bool isConst() const { return is_const; }
        constexpr inline bool isVolatile() const { return is_volatile; }
        constexpr inline bool isRestrict() const { return is_restrict; }
        constexpr inline const Type &getUnderlyingType() const { return typ; }

    private:
        const bool is_const : 1;
        const bool is_volatile : 1;
        const bool is_restrict : 1;
        const Type &typ;

    };

}

#endif // QualType_H_Included
