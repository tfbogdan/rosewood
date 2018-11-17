# Export Rules

For any arbitrary source file every declaration that is declared within the file itself and not in an included header shall be exported by default if it is not captured by any of the rules below.

## Namespaces

Namespaces are traversed and their contents exported recursively by default. Exceptions for namespaces are anonymous namespaces. They will be skipped entirely as they are tipically used for private declarations at namespace level.

## Enumerations

All enumerations and their enumerators shall be exported.

## Classes

All classes and their nested declarations shall be exported by default if they are not private.

## Templates

No template declaration shall be exoported. Reasoning for this is that they are pure compile time constructs thus not of much use at runtime. Typically when one thinks of reflection one thinks about applying it in a runtime context for purposes such as script bindings, serialization, dynamic binding or others. When it comes to templates, their instantiations are much more interesting during runtime. This is open for debate though as the reflection data generated with this toolkit can be used during compile time.

## Template instantiations

No template instantiation or specialization, be it explicit or implicit shall be exported by default. Reason for this is that it's easy to generate template instantiations and with generic code their numbers can quickly become huge. It should however be possible to generate reflection data but it should be explicit. The mechanism by which this shall be achieved shall be `using` declarations. A mockup follows:

```C++

template <typename T>
struct aStruct {};    // not covered by reflection

template <>
struct aStruct<double> {int m1, m2;};  // explicit specializations also not reflected

auto aVariable = aStruct<char>; // this generates a template instantiation but it shall not be reflected upon.

using intPair = aStruct<double>;      // this will generate a reflection descriptor for a class named intPair.
```

This way, the actual class name (`aStruct<double>`) is hidden from it's user. Whether this should be accessible or not is up to debate but that is certainly possible. To the user however a more descriptive and readable name is more desirable.

## Using declarations

Besides being used for triggering the export of template instantiations they shall also be used for generating reflection data for declarations coming from external headers. A good example of this:

```C++
#include <glm/vec3.hpp>

using Vec3 = glm::vec3; // even if glm::vec3 is defined in an external header, this is the mechanism
                        // by which we can get reflection data for it. A meta class called Vec3 will be created as a result of this declaration.
```

## Functions

Functions, template functions or their instantiations are skipped right now. In the close future, they should be looked at too but right now types are of more interest.

