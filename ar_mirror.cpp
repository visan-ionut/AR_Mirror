#include "lab_m2/lab6/lab6.h"

#include <vector>
#include <iostream>

#include "stb/stb_image.h"

using namespace std;
using namespace m2;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */
struct Particle
{
    glm::vec4 position;
    glm::vec4 speed;
    glm::vec4 initialPos;
    glm::vec4 initialSpeed;
    float startTime; // Add this line

    Particle() : startTime(0) {} // Initialize startTime in the constructor

    Particle(const glm::vec4& pos, const glm::vec4& spd, float start)
        : position(pos), speed(spd), initialPos(pos), initialSpeed(spd), startTime(start) {}

    void SetInitial(const glm::vec4& pos, const glm::vec4& spd, float start) {
        position = pos;
        initialPos = pos;
        speed = spd;
        initialSpeed = spd;
        startTime = start;
    }
};

ParticleEffect<Particle>* particleEffect;

Lab6::Lab6()
{
    framebuffer_object = 0;
    color_texture = 0;
    depth_texture = 0;

    angle = 0;

    type = 0;
    useContourShader = false;
    unghiperspectiva = 90.0f;
    totalTime = 0.0f;
}


Lab6::~Lab6()
{
}


void Lab6::Init()
{
    auto camera = GetSceneCamera();
    camera->SetPositionAndRotation(glm::vec3(0, 0, 15), glm::quat(glm::vec3(0, 0, 0)));
    camera->Update();

    std::string texturePath = PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube");
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "lab6", "shaders");

    // Load textures
    {
        TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "particle2.png");
    }

    {
        Mesh* mesh = new Mesh("bunny");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "animals"), "bunny.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("cube");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("archer");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "characters", "archer"), "Archer.fbx");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("car");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "my_models/car"), "car.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("tree");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "my_models/tree"), "tree.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    // Create a shader program for rendering cubemap texture
    {
        Shader* shader = new Shader("CubeMap");
        shader->AddShader(PATH_JOIN(shaderPath, "CubeMap.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "CubeMap.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Create a shader program for standard rendering
    {
        Shader* shader = new Shader("ShaderNormal");
        shader->AddShader(PATH_JOIN(shaderPath, "Normal.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Normal.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Create a shader program for creating a CUBEMAP
    {
        Shader* shader = new Shader("Framebuffer");
        shader->AddShader(PATH_JOIN(shaderPath, "Framebuffer.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Framebuffer.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Framebuffer.GS.glsl"), GL_GEOMETRY_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        Shader* shader = new Shader("Contour");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "lab1", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "lab1", "shaders", "GeometryShader.glsl"), GL_GEOMETRY_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "lab1", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        LoadShader("Simple", false);
        LoadShader("Particle");
    }
    cubeMapTextureID = UploadCubeMapTexture(
        PATH_JOIN(texturePath, "pos_x.png"),
        PATH_JOIN(texturePath, "pos_y.png"),
        PATH_JOIN(texturePath, "pos_z.png"),
        PATH_JOIN(texturePath, "neg_x.png"),
        PATH_JOIN(texturePath, "neg_y.png"),
        PATH_JOIN(texturePath, "neg_z.png"));

    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS), "characters", "archer", "Akai_E_Espiritu.fbm", "akai_diffuse.png");

    // Create the framebuffer on which the scene is rendered from the perspective of the mesh
    // Texture size must be cubic
    CreateFramebuffer(1024, 1024);

    // Define Bezier curve control points
    glm::vec3 bezierP0 = glm::vec3(2, 5, -3); // Replace with your control point values
    glm::vec3 bezierP1 = glm::vec3(1, 3, -2);
    glm::vec3 bezierP2 = glm::vec3(2, 1, -1);
    glm::vec3 bezierP3 = glm::vec3(1, 0, 0);
    // Get the shader program ID
    GLuint shaderProgramID = shaders["Particle"]->GetProgramID();
    // Set the Bezier curve control points in the shader
    glUseProgram(shaderProgramID);
    glUniform3fv(glGetUniformLocation(shaderProgramID, "bezierP0"), 1, glm::value_ptr(bezierP0));
    glUniform3fv(glGetUniformLocation(shaderProgramID, "bezierP1"), 1, glm::value_ptr(bezierP1));
    glUniform3fv(glGetUniformLocation(shaderProgramID, "bezierP2"), 1, glm::value_ptr(bezierP2));
    glUniform3fv(glGetUniformLocation(shaderProgramID, "bezierP3"), 1, glm::value_ptr(bezierP3));
    unsigned int nrParticles = 100;

    particleEffect = new ParticleEffect<Particle>();
    particleEffect->Generate(nrParticles, true);

    auto particleSSBO = particleEffect->GetParticleBuffer();
    Particle* data = const_cast<Particle*>(particleSSBO->GetBuffer());

    float minSpeed = 1.5f; // Define minimum speed limit
    float maxSpeed = 1.6f; // Define maximum speed limit

    float startTimeOffset = 0.1f; // Adjust this value for the staggering effect

    for (unsigned int i = 0; i < nrParticles; i++) {
        glm::vec4 pos(0.0, 0.0, 0.0, 1.0); // Starting position at the origin

        // Generate a random speed within the defined limits
        float randomSpeedFactor = minSpeed + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxSpeed - minSpeed)));
        glm::vec4 speed(randomSpeedFactor, 0.0, 0.0, 0.0); // Speed factor in the x component

        float startTime = static_cast<float>(i) * startTimeOffset;

        data[i].SetInitial(pos, speed, startTime);
    }

    particleSSBO->SetBufferData(data);
}


void Lab6::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Lab6::Update(float deltaTimeSeconds)
{
    angle += 0.5f * deltaTimeSeconds;

    totalTime += deltaTimeSeconds;


    auto camera = GetSceneCamera();

    // Draw the scene in Framebuffer
    if (framebuffer_object)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
        // Set the clear color for the color buffer
        glClearColor(0, 0, 0, 1);
        // Clears the color buffer (using the previously set color) and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, 1024, 1024);

        Shader* shader;
        if (useContourShader) {
            shader = shaders["Contour"];

        }
        else {
            shader = shaders["Framebuffer"];
        }
        shader->Use();

        glm::mat4 projection = glm::perspective(glm::radians(unghiperspectiva), 1.0f, 0.1f, 100.0f);

        {
            glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(30));

            glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
            glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
            glUniform1i(glGetUniformLocation(shader->program, "texture_cubemap"), 1);

            glUniform1i(glGetUniformLocation(shader->program, "cube_draw"), 1);

            meshes["cube"]->Render();
        }

        for (int i = 0; i < 5; i++)
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix *= glm::rotate(glm::mat4(1), angle + i * glm::radians(360.0f) / 5, glm::vec3(0, 1, 0));
            modelMatrix *= glm::translate(glm::mat4(1), glm::vec3(3, -1, 0));
            modelMatrix *= glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0, 1, 0));
            modelMatrix *= glm::scale(glm::mat4(1), glm::vec3(0.01f));

            glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

            glm::mat4 cubeView[6] =
            {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f)), // +X
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f)), // -X
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), // +Y
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f), glm::vec3(0.0f, 0.0f,-1.0f)), // -Y
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f,-1.0f, 0.0f)), // +Z
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f,-1.0f), glm::vec3(0.0f,-1.0f, 0.0f)), // -Z
            };

            glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "viewMatrices"), 6, GL_FALSE, glm::value_ptr(cubeView[0]));
            glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("Akai_E_Espiritu.fbm\\akai_diffuse.png")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);

            glUniform1i(glGetUniformLocation(shader->program, "cube_draw"), 0);
            // Set camera position for silhouette detection in geometry shader

            // Get the view matrix from the camera
            glm::mat4 viewMatrix = camera->GetViewMatrix();

            // Invert the view matrix to get the camera's world transformation matrix
            glm::mat4 cameraWorldTransform = glm::inverse(viewMatrix);

            // Extract the camera's world position from the transformation matrix
            glm::vec3 cameraPosition = glm::vec3(cameraWorldTransform[3]);

            // If the cameraWorldTransform matrix is a pure rotation-translation matrix (no scaling or skewing),
            // the camera position can be extracted directly like this
            cameraPosition = glm::vec3(cameraWorldTransform[3][0], cameraWorldTransform[3][1], cameraWorldTransform[3][2]);
            glUniform3fv(shader->GetUniformLocation("cameraPos"), 1, glm::value_ptr(cameraPosition));

            // Set contour color in fragment shader (if applicable)
            glUniform4f(shader->GetUniformLocation("contourColor"), 1.0f, 0.0f, 0.0f, 1.0f); // Example: Red color

            meshes["archer"]->Render();
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        //reset drawing to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, window->GetResolution().x, window->GetResolution().y);

    // Draw the cubemap
    {
        Shader* shader = shaders["ShaderNormal"];
        shader->Use();

        glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(30));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
        int loc_texture = shader->GetUniformLocation("texture_cubemap");
        glUniform1i(loc_texture, 0);

        meshes["cube"]->Render();
    }

    // Draw five archers around the mesh
    for (int i = 0; i < 5; i++)
    {
        Shader* shader = shaders["Simple"];
        shader->Use();

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= glm::rotate(glm::mat4(1), angle + i * glm::radians(360.0f) / 5, glm::vec3(0, 1, 0));
        modelMatrix *= glm::translate(glm::mat4(1), glm::vec3(3, -1, 0));
        modelMatrix *= glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0, 1, 0));
        modelMatrix *= glm::scale(glm::mat4(1), glm::vec3(0.01f));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("Akai_E_Espiritu.fbm\\akai_diffuse.png")->GetTextureID());
        glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);

        meshes["archer"]->Render();
    }

    {
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        auto shader = shaders["Particle"];
        if (shader->GetProgramID())
        {
            shader->Use();
            glUniform1f(glGetUniformLocation(shader->GetProgramID(), "time"), totalTime);
            TextureManager::GetTexture("particle2.png")->BindToTextureUnit(GL_TEXTURE0);
            particleEffect->Render(GetSceneCamera(), shader);
        }
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    {
        Shader* shader = shaders["CubeMap"];
        shader->Use();

        // Adjust these transformation values as needed
        float scale = 1; // Scale factor
        glm::vec3 position = glm::vec3(5, 0, 0); // Position coordinates
        float angle = glm::radians(25.0f); // Rotation angle in degrees, converted to radians
        glm::vec3 axis = glm::vec3(0, 1, 1); // Rotation axis (e.g., around the y-axis)

        glm::mat4 modelMatrix = glm::translate(glm::mat4(1), position) *
            glm::rotate(glm::mat4(1), angle, axis) *
            glm::scale(glm::mat4(1), glm::vec3(scale));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        auto cameraPosition = camera->m_transform->GetWorldPosition();

        if (!color_texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
            int loc_texture = shader->GetUniformLocation("texture_cubemap");
            glUniform1i(loc_texture, 0);
        }

        if (color_texture) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);
            int loc_texture2 = shader->GetUniformLocation("texture_cubemap");
            glUniform1i(loc_texture2, 1);
        }


        int loc_camera = shader->GetUniformLocation("camera_position");
        glUniform3f(loc_camera, cameraPosition.x, cameraPosition.y, cameraPosition.z);

        glUniform1i(shader->GetUniformLocation("type"), ~type);

        meshes["tree"]->Render();
    }


    // Update car rotation
    carRotationAngle += carRotationSpeed * deltaTimeSeconds;

    // Ensure the angle stays within a valid range, such as 0 to 360 degrees
    carRotationAngle += carRotationSpeed * deltaTimeSeconds;
    if (carRotationAngle >= 360.0f) {
        carRotationAngle -= 360.0f;
    }
    else if (carRotationAngle < 0.0f) {
        carRotationAngle += 360.0f;
    }
    // Draw the reflection on the mesh
    {
        Shader* shader = shaders["CubeMap"];
        shader->Use();
        glm::mat4 modelMatrix = glm::mat4(1);
        // Adjust these transformation values as needed
        float scale = 1.25; // Scale factor
        glm::vec3 position = glm::vec3(0, 0, -5); // Position coordinates

        modelMatrix = glm::translate(glm::mat4(1), position) *
            glm::rotate(modelMatrix, glm::radians(carRotationAngle), glm::vec3(0, 1, 0)) *
            glm::scale(glm::mat4(1), glm::vec3(scale));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        auto cameraPosition = camera->m_transform->GetWorldPosition();

        if (!color_texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
            int loc_texture = shader->GetUniformLocation("texture_cubemap");
            glUniform1i(loc_texture, 0);
        }

        if (color_texture) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);
            int loc_texture2 = shader->GetUniformLocation("texture_cubemap");
            glUniform1i(loc_texture2, 1);
        }


        int loc_camera = shader->GetUniformLocation("camera_position");
        glUniform3f(loc_camera, cameraPosition.x, cameraPosition.y, cameraPosition.z);

        glUniform1i(shader->GetUniformLocation("type"), type);

        meshes["car"]->Render();
    }




}


void Lab6::FrameEnd()
{
    // DrawCoordinateSystem();
}


unsigned int Lab6::UploadCubeMapTexture(const std::string& pos_x, const std::string& pos_y, const std::string& pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z)
{
    int width, height, chn;

    unsigned char* data_pos_x = stbi_load(pos_x.c_str(), &width, &height, &chn, 0);
    unsigned char* data_pos_y = stbi_load(pos_y.c_str(), &width, &height, &chn, 0);
    unsigned char* data_pos_z = stbi_load(pos_z.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_x = stbi_load(neg_x.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_y = stbi_load(neg_y.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_z = stbi_load(neg_z.c_str(), &width, &height, &chn, 0);

    unsigned int textureID = 0;
    // TODO(student): Create the texture
    glGenTextures(1, &textureID);

    // TODO(student): Bind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (GLEW_EXT_texture_filter_anisotropic) {
        float maxAnisotropy;

        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // TODO(student): Load texture information for each face
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_z);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_z);


    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    if (GetOpenGLError() == GL_INVALID_OPERATION)
    {
        cout << "\t[NOTE] : For students : DON'T PANIC! This error should go away when completing the tasks." << std::endl;
    }

    // Free memory
    SAFE_FREE(data_pos_x);
    SAFE_FREE(data_pos_y);
    SAFE_FREE(data_pos_z);
    SAFE_FREE(data_neg_x);
    SAFE_FREE(data_neg_y);
    SAFE_FREE(data_neg_z);

    return textureID;
}

void Lab6::CreateFramebuffer(int width, int height)
{
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &framebuffer_object);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);

    // Generate and bind the color texture as a cubemap
    glGenTextures(1, &color_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);

    // Allocate the necessary memory for each face of the color cubemap texture
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

    // Set up the texture parameters for the color texture
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Anisotropic filtering, if available
    if (GLEW_EXT_texture_filter_anisotropic) {
        float maxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    }

    // Attach the color texture to the framebuffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);

    // Generate and bind the depth texture as a cubemap
    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_texture);

    // Allocate the necessary memory for each face of the depth cubemap texture
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
    }

    // Attach the depth texture to the framebuffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
    }

    // Unbind the framebuffer to avoid accidental rendering to it
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}






/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Lab6::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
}


void Lab6::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_1)
    {
        type = 1;
    }

    if (key == GLFW_KEY_2)
    {
        type = 0;
    }

    if (key == GLFW_KEY_LEFT) {
        carRotationSpeed = -25.0f; // Rotate left
    }
    if (key == GLFW_KEY_RIGHT) {
        carRotationSpeed = 25.0f; // Rotate right
    }
    if (key == GLFW_KEY_K) {
        useContourShader = !useContourShader; // Toggle between true and false
        unghiperspectiva = -unghiperspectiva;
    }
}


void Lab6::OnKeyRelease(int key, int mods)
{
    if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) {
        carRotationSpeed = 0.0f; // Stop rotation when key is released
    }
}

void Lab6::LoadShader(const std::string& name, bool hasGeomtery)
{
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Labp", "shaders");

    // Create a shader program for particle system
    {
        Shader* shader = new Shader(name);
        shader->AddShader(PATH_JOIN(shaderPath, name + ".VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, name + ".FS.glsl"), GL_FRAGMENT_SHADER);
        if (hasGeomtery)
        {
            shader->AddShader(PATH_JOIN(shaderPath, name + ".GS.glsl"), GL_GEOMETRY_SHADER);
        }

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}

void Lab6::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Lab6::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Lab6::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Lab6::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Lab6::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
