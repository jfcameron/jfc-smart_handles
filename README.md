## jfc-smart_handles

scoped wrappers for handle types found in OpenGL, OpenAL, LibClang. Associates a handle with a deletor, to be called when the wrapper falls out of scope.
The smart_handles define handle move semantics and end of life behavior, simplifying writing objects around e.g vertex buffer handles, audio buffer handles.
