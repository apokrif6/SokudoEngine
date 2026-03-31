# SokudoEngine

Game Engine written in C++ and Vulkan. I am writing this in my free time to learn and experiment with new technologies and concepts.

---
## ⚙️ Overview

![Overview 1](https://github.com/apokrif6/apokrif6.github.io/blob/main/assets/images/portfolio/sokudoengine/SokudoEngine_EngineOverview1.png)

![Overview 2](https://github.com/apokrif6/apokrif6.github.io/blob/main/assets/images/portfolio/sokudoengine/SokudoEngine_EngineOverview2.png)

## 🌟 Features

### 💎 Physically Based Rendering
- HDR environment maps
- Image Based Lighting
- Directional Lighting support

![PBR 1](https://github.com/apokrif6/apokrif6.github.io/blob/main/assets/images/portfolio/sokudoengine/SokudoEngine_PBR1.gif)
![PBR 2](https://github.com/apokrif6/apokrif6.github.io/blob/main/assets/images/portfolio/sokudoengine/SokudoEngine_PBR2.gif)
![PBR 3](https://github.com/apokrif6/apokrif6.github.io/blob/main/assets/images/portfolio/sokudoengine/SokudoEngine_PBR3.gif)


### 🧱 Component System
- Unreal style component system
- Easy to extend and create new components
- Supports serialization and deserialization of components

![Component System](https://github.com/apokrif6/apokrif6.github.io/blob/main/assets/images/portfolio/sokudoengine/SokudoEngine_ComponentSystem.gif)

### 🕺 Animation system
- Skeletal animation with skinning
- Supports multiple animation clips per model
- Debug visualization of skeleton

![Animations Example](https://github.com/apokrif6/apokrif6.github.io/blob/main/assets/images/portfolio/sokudoengine/SokudoEngine_Animations.gif)

### 🌍 Scene system
- Scene graph traversal for rendering and updates
- Hierarchical transformations
- Editor picking and manipulation

![Scene Example](https://github.com/apokrif6/apokrif6.github.io/blob/main/assets/images/portfolio/sokudoengine/SokudoEngine_SceneSystem.gif)

### 💾 Serialization system for saving and loading levels
- yaml based serialization
- Can serialize and deserialize complex object hierarchies with ease
- Supports custom data types and references between objects
- Uses meta-data to store asset paths instead of raw data

![Serialization Example 1](https://github.com/apokrif6/apokrif6.github.io/blob/main/assets/images/portfolio/sokudoengine/SokudoEngine_SceneSerialization1.gif)
![Serialization Example 2](https://github.com/apokrif6/apokrif6.github.io/blob/main/assets/images/portfolio/sokudoengine/SokudoEngine_SceneSerialization2.gif)

## 🔗 Dependencies

To build and run `SokudoEngine`, you will need:
- [Vulkan SDK](https://vulkan.lunarg.com)
- [vcpkg](https://vcpkg.io)

## 💜 Special Thanks

Special thanks to ![@Niyoofficial](https://github.com/Niyoofficial) for the help with debugging, architecture, and just generally being an awesome guy!
