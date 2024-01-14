#version 430

// Input
layout(location = 0) in vec3 world_position;
layout(location = 1) in vec3 world_normal;

// Uniform properties
uniform samplerCube texture_cubemap;
uniform int type;

uniform vec3 camera_position;

// Output
layout(location = 0) out vec4 out_color;


vec3 myReflect()
{
    // TODO(student): Compute the reflection color value
    vec3 V = normalize(world_position - camera_position);

    return reflect(V, world_normal);

}


vec3 myRefract(float refractive_index)
{
    // TODO(student): Compute the refraction color value
    vec3 V = normalize(world_position - camera_position);

    return refract(V, world_normal, 1.0 / refractive_index);
}


void main()
{

    if (type == 0)
    {
        out_color = texture(texture_cubemap, myReflect());
    }
    else
    {
        out_color = texture(texture_cubemap, myRefract(1.33));
    }
}
