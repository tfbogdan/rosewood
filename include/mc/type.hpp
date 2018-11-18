#pragma once



namespace mc {

template <typename ty>
struct plain_type {
    using type = ty;
    using atomic_type = ty;
};

template <typename ty>
struct pointer_type {
    using type = typename ty::type*;
    using undecorated_type = typename ty::type;
    using atomic_type = typename ty::atomic_type;
};

template <typename ty>
struct reference_type {
    using type = typename ty::type&;
    using undecorated_type = typename ty::type;
    using atomic_type = typename ty::atomic_type;
};

template <typename ty>
struct rvalueref_type {
    using type = typename ty::type&&;
    using undecorated_type = typename ty::type;
    using atomic_type = typename ty::atomic_type;
};

template <typename ty>
struct const_type {
    using type = const typename ty::type;
    using undecorated_type = typename ty::type;
    using atomic_type = typename ty::atomic_type;
};

template <typename ty>
struct volatile_type {
    using type = volatile typename ty::type;
    using undecorated_type = typename ty::type;
    using atomic_type = typename ty::atomic_type;
};


template<typename T>
using const_ref = reference_type<const_type<plain_type<T>>>;

}
