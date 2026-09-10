# jfc-smart_handles

scoped wrappers for handle types found in libraries like OpenGL, OpenAL, LibClang, FreeType. Associates a handle with a deleter, to be called when the wrapper falls out of scope.

The smart_handles define move semantics, copy semantics and end of life behavior, simplifying writing objects around e.g vertex buffer handles, audio buffer handles.

The types, their interface and interactions are modeled after the stl smart pointers.

## unique_handle
Unique Handle models the single owner case with move support to another unique handle or a shared handle. 

## shared_handle
Shared handle models the many owners case, the deletor is invoked when the final owner falls out of scope. A shared handle can also be copied to a Weak Handle. 

## weak_handle
Weak handle is similar to Shared, except it does not contribute to the use_count and the handle cannot be accessed directly. Instead, the lock method must be called, which returns an optional to a shared handle. If the shared handle that the weak handle is observing has not fallen out of scope at the time of the lock then the optional will contain a new shared handle to the resource. If not, then the optional will be null.

