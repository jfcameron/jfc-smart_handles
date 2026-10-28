# jfc-smart_handles

scoped wrappers for handle types found in libraries like OpenGL, OpenAL, LibClang, FreeType. Associates a handle with a deleter, to be called when the wrapper falls out of scope.

The smart_handles define move semantics, copy semantics and end of life behavior, simplifying writing objects around e.g vertex buffer handles, audio buffer handles.

The types, their interface and interactions are modeled after the stl smart pointers.

API documentation: https://jfcameron.github.io/jfc-smart_handles/

## Getting Started
Install cmake version 3.21 or greater, and a c++20 compatible compiler 

See `CMakePresets.json` for the full list of supported platforms and compilers. See `.github/workflows/ci.yml` to see exactly how CI runners build it on different platforms.

## Handle types
### unique_handle
Unique Handle models the single owner case with move support to another unique handle or a shared handle. `release` gives up the handle without deleting it, for handing it to an API that takes ownership.

### shared_handle
Shared handle models the many owners case, the deletor is invoked when the final owner falls out of scope. A shared handle can also be copied to a Weak Handle. 

### weak_handle
Weak handle is similar to Shared, except it does not contribute to the use_count and the handle cannot be accessed directly. Instead, the lock method must be called, which returns a shared handle. If the shared handle that the weak handle is observing has not fallen out of scope at the time of the lock then it is a new share of the resource. If not, then it is empty.

