#pragma once

#include <mc/mc.hpp>

namespace mc {
    class DynamicClass {
    public:
        virtual ~DynamicClass() = 0;
        virtual bool hasMethod(std::string_view name) const = 0;
        // virtual void call(std::string_view method, const void *obj, int argc, void **argv) const = 0;
        // virtual void call(std::string_view method, void *obj, int argc, void **argv) = 0;
    private:
    };

    template <typename WrappedType>
    class DynamicClassWrapper : public DynamicClass, public mc::meta<WrappedType>{
        using descriptor = mc::meta<WrappedType>;

        inline virtual ~DynamicClassWrapper() = default;
        inline virtual bool hasMethod(std::string_view name) const {
            return descriptor::has_overload_set(name);
        }

        // inline virtual void call(std::string_view method, const void *obj, int argc, void **argv) const {
            // const auto& Object = *static_cast<const typename descriptor::type*>(obj);
        // }
    };
}
