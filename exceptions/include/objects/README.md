# Objects

## Overview

This directory provides the tools used for stack unwinding on exception throwing.

Specifically, it exports the `AutoObject` class template to be used explicitly
to register an object for potential stack unwinding.

## Dependencies

Both the declarations and the definitions of the symbols from this directory
are dependent on the symbols provided by the [utils directory](../utils).

## Symbols

### Exported

```cpp
::
   AutoObject  // auto.hpp
```

### Internal

```cpp
except::detail::
        ObjectManager   // manager.hpp
        AutoObject      // auto.hpp
```

## Design description

### ObjectManager

The `ObjectManager` static class implements the main part of the stack unwinding
business logics.

Internally it stores a stack, the items of which may either be a checkpoint
(represented by the `std::monostate` type) or a pair of a pointer to a function
that destroys an object and a pointer to that object converted to `void*`.
The latter pair will be referred to as *destruction instructions*.

#### RecordCheckpoint

The `RecordCheckpoint` static member function is used to push checkpoints
to the stack. It does not accept any arguments since it needs none to construct
an `std::monostate` instance.

The checkpoint is placed in the stack at the beginning of each `TRY` block.
It is used as a stop point for the stack unwinding: the objects are destroyed
from the top of the stack down until the checkpoint is encountered.

#### RecordObject

The `RecordObject` static member function is used to push objects destruction
instructions to the stack. It accepts a reference to the instance
to be registered for destruction during a potential stack unwinding.
It instantiates a function template `Destroy` with the type template parameter
equal to the type of the provided instance and pushes it to the stack
in pair with the pointer to the provided instance converted to `void*`.

Since the `Destroy` template function instantiation is different for each type
to be destroyed, and each instantiation is paired with a pointer that
was initially of a respective type, it can safely convert the `void*` provided
to it back to a pointer to the initial type and then call the destructor of that
type on it.

#### Pop

The `Pop` static member function is used to pop an element from the stack.
It does not check whether the popped item is a checkpoint or destruction
instructions, it is the user's responsibility to keep track of what is popped
from the stack.

#### UnwindToCheckpoint

The `UnwindToCheckpoint` static member function implements the stack unwinding
business logics itself.

First, it pops the elements from the stack until they are destruction
instructions and calls the destructors of the objects by providing
the second element of the pair as an argument to the function indicated by
the pointer stored by the first element of the pair. See [above](#recordobject)
for details on the specific details of the destructors calls.

After that it pops one more element, which is a checkpoint due to the stop
condition of the loop above.

It is the user's responsibility to ensure that there is at least one checkpoint
in the stack when calling this function, otherwise at some point it will call
the `std::stack::top` method on an empty stack which will invoke
`std::deque::back` on the underlying empty deque, which in turn will lead
to undefined behaviour (as [specified](https://clck.ru/34N64H) in the standard).

### AutoObject

The `AutoObject` class template is used to automatically register objects
for a potential stack unwinding on construction and unregister them
on destruction.

It has two specifications: for `final` and non-`final` wrapped classes.
The former explicitly stores the instance as a field while the latter
derives from the wrapped class and stored the instance implicitly as the instance
of the base class.

#### Constructors

Both specializations define all the constructors available in the wrapped
class, which delegate the construction itself to the respective constructor
of the wrapped class, after which they call `ObjectManager::RecordObject`
static method to register the object for destruction during a potential
stack unwinding.

#### Member access

Both specializations also overload the `operator->`, which has a different
logics depending on the wrapped class.

If the wrapped class in turn defines `operator->`, the overloaded operator
returns a reference to the stored instance. Otherwise, it returns a pointer
to it. Such behaviour is chosen due to the rules by which the `operator->` gets
applied: it is applied over and over again until the result is a reference
or a value of a type which in turn has an overloaded `operator->` (if at some
point it returns a reference or a value which does not have an overloaded
`operator->`, an error occurs). Once the result is a raw pointer, the builtin
`operator->` is used. For more details on `operator->` refer
to the [standard](https://clck.ru/34N6Xt).

Therefore, we have 4 situations:

| Final | Defines `operator->` | Description                                                                                                                                                                                                                                                                                                                                    |
|:-----:|:--------------------:|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|  Yes  |         Yes          | In this case the `operator->` of the `AutoObject` gets chained with the class's own `operator->` and can be used in exactly the same way. To access the members of the wrapped class, an explicit `Get()` method should be used (see [below](#get-and-move) for details)                                                                       |
|  Yes  |          No          | In this case the members of the wrapped class can be accessed via the `operator->` of the `AutoObject` instance (since in this case it returns a pointer to the stored instance, which would cause it to get chained with the builtin `operator->`)                                                                                            |
|  No   |         Yes          | In this case the `operator->` of the `AutoObject` gets chained with the class's own `operator->` and can be used in exactly the same way. To access the members of the wrapped class, the `operator.` can be used (since the `AutoObject` in this case is derived from the wrapped class and thus inherits its members)                        |
|  No   |          No          | In this case the members of the wrapped class can be accessed via the `operator->` of the `AutoObject` instance (since in this case it returns a pointer to the stored instance, which would cause it to get chained with the builtin `operator->`). They can also be accessed via the `operator.` for the same reasons as in the previous row |                                                               

#### Operators

The first specialization (for the `final` classes) additionally redefines all
the member operators defined in the wrapped class, which allows to use
the `AutoObject` instance in the same way as the stored instance would be used.

#### Conversions

Additionally, the specialization for the `final` classes explicitly defines
implicit conversion operators from `AutoObject` to the stored class. These
conversions are overloaded for lvalue references (both const and non-const) and
rvalue references and return a reference of the same category to the stored
instance.

> **Note**
>
> The specialization for the non-`final` classes also has these implicit
> conversions. They are available due to its being derived from the wrapped
> class, and do not require explicitly defining them.

#### Get and Move

The specialization for the `final` classes also defines methods `Get` and `Move`.

The former can only be used on lvalues and returns an lvalue reference
to the stored instance. It can be used for explicit access to the stored
instance.

The latter can only be used on rvalues to disambiguate overloads in
some situations. An example of such a situation, along with its description,
can be found in the last section of the [usage test](tests/suits/usage.cpp).

#### Deduction guide

A deduction guide is introduced for creating an `AutoObject` instance directly
from another object (via copy or move). This allows to use `AutoObject` in case
when the type cannot be spelled (e.g. with lambdas). This feature is illustrated
in the `Deduction` test in the [templates suit](tests/suits/templates.cpp).

### Caveats

Calling the `delete` operator on instances that were created using the `new`
operator during stack unwinding is currently not implemented. If an instance
of `AutoObject` was created via the `new` operator, the stored object will get
destroyed during stack unwinding, but the `delete` operator will not be called,
which can lead to memory leaks.

A workaround for this issue is to use smart pointers and wrap them inside
an `AutoObject`:

```cpp
AutoObject<std::unique_ptr<MyClass>> on_heap = std::make_unique<MyClass>(...); 
```

or simply (using the deduction guide):

```cpp
AutoObject on_heap = std::make_unique<MyClass>(...); 
```

> **Note**
>
> This is not the same as `std::unique_ptr<AutoObject<MyClass>>`, which would
> not register the `std::unique_ptr` instance for destruction during a potential
> stack unwinding.

Then during stack unwinding the destructor of `std::unique_ptr` will be called,
which will in turn call the `delete` operator.
