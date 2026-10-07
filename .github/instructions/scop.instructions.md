I am building a C++ OpenGL project called SCOP. I need a manual OBJ/MTL loading system.

Important constraints:

* Use standard C++ only for parsing and mesh processing.
* Do NOT use GLM or any external OBJ/MTL/model-loading library.
* Parser and mesh-processing classes must NOT depend on OpenGL.
* Do not create VAOs, VBOs, EBOs, textures, shaders, or any OpenGL objects.
* Keep parsing separate from rendering.
* Prefer clear, maintainable C++ over unnecessary abstractions.
* Do not add features I did not ask for.
* Do not redesign the architecture.
* Use std::vector, std::string, std::unordered_map, std::ifstream, etc. where appropriate.
* Handle malformed input with clear exceptions and useful error messages.
* Keep declarations in .hpp files and implementations in .cpp files.
* Make the code compile cleanly with a modern C++ standard.
* Explain the design briefly before the code.
* Give me complete .hpp and .cpp files.
* Preserve the interfaces and data structures I specify.
