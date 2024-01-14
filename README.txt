=========
AR Mirror
=========

This project provides only the implementation code. To run the scene, ask for the
framework and additional details.

The AR Mirror project is a computer graphics assignment for an augmented reality
mirror. The mirror reflects the environment in three modes: normal reflection,
object contours only, and an environment with a particle system. 
The implementation involves OpenGL and C++, following a provided framework and
adhering to specific guidelines.

Task Description:
Objective: Implement a technical demo showcasing the application of
accumulated knowledge in computer graphics.
Concept: Create an augmented reality mirror that reflects the surrounding
environment with added augmented elements based on user interaction.

Mirror Interaction:
By default, the mirror reflects the unaltered surrounding environment.
Upon pressing a key, the mirror displays only the contours of objects in
the environment. Another key press reveals the environment with the addition
of a particle system resembling fireflies following Bezier curve trajectories.

Rendering 3D Object Contours:
Implement a technique to render the contours of 3D objects based on the
orientation of their faces towards the observer. Utilize a method involving
vertex normals to distinguish between front-facing and back-facing vertices.

Particle System:
Create a particle system emitting "firefly" particles that follow Bezier curve
trajectories. Each particle has properties such as position, speed, delay, and
lifetime, contributing to the dynamic and visually appealing nature of the scene.

Dynamic Reflection of Surroundings:
Design a dynamic environment containing a static skybox and various dynamically
moving objects. Implement a mirror (quad) that can be translated and rotated.
Use a cubemap rendering approach to reflect the environment on the mirror's surface.

Geometry Shader for Cubemap Rendering:
Emit geometry six times in the geometry shader, once for each face of the cubemap.
Implement transformations based on the view matrix of each camera to create six
different perspectives for the cubemap.

Rendering Modes:
Switch between rendering modes with key presses, including default reflection,
object contours only, and the environment with the particle system.
