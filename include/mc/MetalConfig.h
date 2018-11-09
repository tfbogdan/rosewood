#ifndef MetalConfig_h_Included
#define MetalConfig_h_Included

#include <vector>

#ifdef __MC__PARSER_LIBRARY

#include <string>
#include <memory>

namespace metal {
    typedef std::string mc_string;
    typedef const std::string &mc_str_ref;

    template < typename T >
    using mc_owning_ptr = std::shared_ptr<T>;

    template < typename T >
    using mc_ref_ptr = T*;

}

#else 

namespace metal {
    typedef const char *mc_string;
    typedef const char *mc_str_ref;

    template < typename T >
    using mc_owning_pointer = T*;

    template < typename T >
    using mc_ref_ptr = T*;

}

#endif // __MC__PARSER_LIBRARY


#ifdef __MC__PARSER_RUNNING
#define mc_export __attribute__((annotate("mc_export")))
#else 
#define mc_export
#endif // __MC__PARSER_RUNNING

#ifdef __MC__PARSER_RUNNING
#define mc_property __attribute__((annotate("mc_property")))
#else 
#define mc_property
#endif // __MC__PARSER_RUNNING

#ifdef __MC__PARSER_RUNNING
#define mc_getter(prop) __attribute__((annotate("mc_getter:"#prop)))
#else 
#define mc_getter(prop)
#endif // __MC__PARSER_RUNNING

#ifdef __MC__PARSER_RUNNING
#define mc_setter(prop) __attribute__((annotate("mc_setter:"#prop)))
#else 
#define mc_setter(prop)
#endif // __MC__PARSER_RUNNING

#ifdef __MC__PARSER_RUNNING
#define mc_notify(prop) __attribute__((annotate("mc_notify:"#prop)))
#else 
#define mc_notify(prop)
#endif // __MC__PARSER_RUNNING


#ifdef __MC__PARSER_RUNNING
#define MC_GENERATED_CLASS_BODY(a) 
#else 
#define MC_GENERATED_CLASS_BODY(a) 
#endif // __MC__PARSER_RUNNING

#ifdef __MC__PARSER_RUNNING
#define __MC(a) __attribute__((annotate(#a)))
#else 
#define __MC(a)
#endif // __MC__PARSER_RUNNING



struct FunctionExecutor {
    template < typename _Fct >
    FunctionExecutor(_Fct &fct) {
        fct();
    }
};

// FIXME FIXME
#define REFLECTABLE_CLASS(cls) friend struct metal::InvokableDispatcher<cls>; \
public: \
virtual const metal::RecordDecl *metaClass() \
    {return nullptr;} 


namespace metal {
    template < typename ClassTy >
    struct InvokableDispatcher {

    };

    typedef void(*methodCallUnpacker_t)(void *, int, void **, void *);

    class RecordDecl;

    class Reflectable {
    public:

        virtual const metal::RecordDecl *metaClass() = 0;
        virtual ~Reflectable() = default;
    };

    struct OutgoingConnection {
        void *caleeObject;
        methodCallUnpacker_t caleeFun;
    };

    struct EventBase;
    struct IncomingConnection {
        EventBase *caller;
    };

    struct Connection {
        OutgoingConnection *out;
        IncomingConnection *in;
    };

    struct EventBase {
        // then this event type contains the list of listeners. 
        std::vector<OutgoingConnection*> connections;
    };

    template < typename returnType, typename ...Args >
    struct Event : public EventBase {

        returnType operator() (Args ...args) {
            // pack arguments 
            // dispatch on every connection
            // q: how to handle return values? Can this be handled as a template policy? 
        }
    };

    // for incoming events
    // If we don't define an instance of this struct for a certain method, 
    // then we cannot have lifetime controlled bindings to it
    // this acts as a checked connection
    // we can then store all handles inside this struct, and be able to suspend connections
    // and destroy them, and so on
    // this is a mere container. the argument unpacking is handled by the unpacker
    struct EventBindingContainer {
        std::vector<IncomingConnection*> connections;
    };
}



#endif // MetalConfig_h_Included
