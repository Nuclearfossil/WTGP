# WGTP

This is a repo that contains all the code-backed examples for tutorials located here: [D3D 11 Course Outline](https://docs.google.com/document/d/1x0jPJXkq5OQAx-CtBElPALUMeJXIycc8aj5Dq8j3qNk/edit?usp=sharing)

Most libraries are installed as VCPKG recipes.

Please use VCPKG to install:

```
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ./vcpkg integrate install
    ./vcpkg install imgui[core,win32-binding,dx11-binding]:x64-windows --recurse
    ./vcpkg install plog
    ./vcpkg install assimp
````

Additionally, note that to debug the projects, you may need to set the **Working Directory**, in the _Debugging Configuration_ properties to `$(OutputPath)`. I have been seeing this not actually persist into the project. It may be part of the User Config files for VCXPROJ files, which I believe I have resolved at this point
