# CustomOBJLoader
OBJ and MTL loader for OpenGL. Made from scratch.

## Features
This lightweight library can load OBJ Wavefront 3D-models alongside with MTL material. Supports only colors for now.

## How to use
Import "obj_loader.hpp" to start using library.

### Short example:
```
std::vector<Vertex> vertices;
std::vector<GLuint> indices;

// Replace "model" with your OBJ
if (ParseOBJ("model.obj", &vertices, &indices)) {
    std::cout << "Loaded model with " << vertices.size() << " vertices and " << indices.size() << " indices.\n";
    // You can now use vertices and indices for rendering
} else {
    std::cout << "Failed to load OBJ file.\n";
}
```

Long example available in "example" folder.