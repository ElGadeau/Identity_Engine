# Identity Engine

(WIP) A DirectX game engine with an editor developped for academic purposes.

The Identity Engine has been named as such because one of the primary features the development team wanted to see is a very user-friendly and simple, uncluttered and straightforward experience for the user, reminiscent of an identity matrix.

One of the main goals the team has set for itself is to make a very optimised engine. Therefore, the engine is being developped with an ECS architecture and will be event-driven as much as possible to avoid unnecessary checks at every frame update.

The external libraries used are:

- GPM (homemade library) for math.
- Bullet for physics.
- IrrKlang for audio.
- Assimp for model loading.
- ImGUI for the editor.
- Near-future plans include, but are not limited to:

Frustrum culling
Serialization
User scripting
Currently, the engine project automatically creates a very simple scene with a car falling on a character, showcasing our renderer, our first version of the editor and our rudimentary (for now) physics. Debug mode is enabled, so the colliders are visible. To run the engine with Visual Studio, set the StartUp Project to Test_Identity and compile. Can run in Debug and Release configurations, but 32bit architecture is not currently supported.

## Authors

* **Lilian Gadeau** - *Programming*
* **Filipe Antunes Da Silva** - *Programming* - [GitHub](https://github.com/fantunes-ds)
* **Gabriel Meloche** - *Programming* - [GitHub](https://github.com/GabMeloche)
