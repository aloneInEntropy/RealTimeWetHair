#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "main.h"

void init() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(MessageCallback, 0);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true);
    srand(time(nullptr));

    SM::initScene();
    Input::initialiseInputMap();

    startShader = new Shader("starter", vert_smesh, frag_smesh);
    startLight = new Lighting("start light", startShader, MATERIAL_SHINY);
    renderHead = new StaticMesh("Render Head", MESH_HEAD);
    guideHead = new StaticMesh("Guide Head", MESH_GUIDE_HEAD);
    largeHead = new StaticMesh("Root Head", MESH_LARGE_HEAD);

    HairConfigs hs;
    /* Generate hairs */
    int rndCnt = 0;
    for (int i = 0; i < largeHead->vertexData.size(); ++i) {
        // if (rndCnt >= 512) break;
        if (largeHead->vertexData[i].pos.y >= 0) {
            // only normals facing out/up
            hs.push_back({10, vec4(largeHead->vertexData[i].pos, 1), largeHead->vertexData[i].norm});
            rndCnt++;
        }
    }

    headStartPos = vec3(150, 6, 150);
    FluidConfig fconfig = {20000, DAM_BREAK, vec3(0, 20, 0)};
    sim = new Sim::Simulation(hs, fconfig);
    sim->hair->headTrans = translate(mat4(1), headStartPos);

    SM::camera->lookAt(headStartPos + vec3(0, 30, 0));
    glPointSize(8.0);
    glLineWidth(1);

    startLight->addDirLightAtt(Util::DOWN, vec3(0.2f), vec3(0.2f), vec3(1));
    // startLight->addDirLightAtt(Util::UP, vec3(0.2f), vec3(0.2f), vec3(1));
    // startLight->addDirLightAtt(Util::LEFT, vec3(0.2f), vec3(0.2f), vec3(1));
    // startLight->addDirLightAtt(Util::RIGHT, vec3(0.2f), vec3(0.2f), vec3(1));
    // startLight->addDirLightAtt(Util::FORWARD, vec3(0.2f), vec3(0.2f), vec3(1));
    // startLight->addDirLightAtt(Util::BACKWARD, vec3(0.2f), vec3(0.2f), vec3(1));
    SM::startTime = timeGetTime();
}

void update() {
    SM::updateDelta();
    if (SM::cfg.isCamMode) {
        SM::camera->processMovement();
    }
    sim->hair->headTrans = translate(mat4(1), headStartPos) *
                           eulerAngleYXZ(Util::rad(headStartRot.y), Util::rad(headStartRot.x), Util::rad(headStartRot.z));
    sim->update();
    SM::updateTick();
}

void display() {
    if (showFluid) {
        sim->fluid->envFBO->bind();
    }
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);   // enable backface culling
    glEnable(GL_DEPTH_TEST);  // enable depth-testing
    glDepthFunc(GL_LESS);     // depth-testing interprets a smaller value as "closer"
    glEnable(GL_BLEND);       // enable colour blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mat4 view = SM::camera->getViewMatrix();
    mat4 projection = SM::camera->getPerspectiveMatrix();
    startLight->use();
    startLight->setLightAtt(view, projection, SM::camera->pos);
    startLight->setPointLightAtt(0, lightPos);
    if (showHead) guideHead->render(sim->hair->headTrans);
    sim->hair->render();
    SM::drawSceneExtras();
    if (showFluid) {
        sim->fluid->envFBO->unbind();
        sim->fluid->render();
    }
    
}

void displayUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // ImGui::ShowDemoWindow();
    ImGui::Begin("Debug Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    auto io = ImGui::GetIO();
    if (ImGui::CollapsingHeader("Debug")) {
        if (ImGui::TreeNodeEx("Info", ImGuiTreeNodeFlags_SpanAvailWidth)) {
            ImGui::Text("Average FPS: %.1f (%.3f ms/frame)", io.Framerate, 1000.0f / io.Framerate);
            ImGui::Text("Mouse Position: %.0f, %.0f", Input::mouse.x, Input::mouse.y);
            ImGui::Checkbox("Camera Mode", &SM::cfg.isCamMode);
            UI::Help(
                "Key: [Left Alt]\n"
                "If checked, the mouse controls the camera and can look around the scene. "
                "Otherwise, the mouse simply acts as the cursor.");
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_SpanAvailWidth)) {
            if (ImGui::TreeNodeEx("Background", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                ImGui::Checkbox("Show Skybox", &SM::cfg.showSkybox);
                ImGui::Checkbox("Show Grid", &SM::cfg.showGrid);
                ImGui::ColorEdit3("Background Colour", &SM::cfg.bgColour.x);
                ImGui::ColorEdit3("Grid Thick Colour", &SM::cfg.gridThickColour.x);
                ImGui::ColorEdit3("Grid Thin Colour", &SM::cfg.gridThinColour.x);
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                ImGui::Text("Camera Mode: %s", SM::isFreeCam ? "Free" : (SM::isFirstPerson ? "First" : "Third"));
                if (ImGui::DragFloat3("Camera Position", &SM::camera->pos.x, 0.1f, -100, 100)) {
                    SM::camera->updateViewMatrix();
                }
                if (ImGui::DragFloat3("Camera Front", &SM::camera->front.x, 0.01f, -1, 1)) {
                    SM::camera->updateViewMatrix();
                }
                ImGui::DragFloat("Camera Walk Speed", &SM::camera->baseSpeed, 0.1f, 0.1f, 100);
                ImGui::DragFloat("Camera Sprint Speed", &SM::camera->sprintSpeed, 0.1f, 0.1f, 1000);
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
    }
    // todo: organise
    if (ImGui::CollapsingHeader("Application\t\t\t", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::TreeNodeEx("Settings", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth)) {
            if (ImGui::TreeNodeEx("Constraints", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                ImGui::DragFloat("SS Stiffness", &sim->hair->ss_k, .0001, 0.01, 1);
                ImGui::DragFloat("SS SOR", &sim->hair->ss_SOR, .0001, 0.01, 10);
                UI::Help(
                    "[UPP14]\n"
                    "Successive over-relaxation value for the POCR Stretch and Shear constraint.\n");
                ImGui::NewLine();
                ImGui::DragFloat("BT Stiffness", &sim->hair->bt_k, .0001, 0.01, 1);
                ImGui::DragFloat("BT SOR", &sim->hair->bt_SOR, .0001, 0.01, 10);
                UI::Help(
                    "[UPP14]\n"
                    "Successive over-relaxation value for the POCR Bend and Twist constraint.\n");
                ImGui::DragFloat("DN SOR", &sim->fluid->SOR, 0.01f, 0.01, 10);
                UI::Help(
                    "[UPP14]\n"
                    "Successive over-relaxation value for the PBF Density constraint.\n");
                ImGui::DragInt("Substeps", &CommonSim::simulationSubsteps, .1, 1, 20);
                ImGui::DragInt("Iterations", &CommonSim::simulationIterations, .1, 1, 20);
                // if (ImGui::Button("Print Physics Settings")) {
                //     sim->printPhysicsSettings();
                // }
                // ImGui::SameLine();
                // if (ImGui::Button("Print Hair Settings")) {
                //     sim->printHairSettings();
                // }
                // ImGui::SameLine();
                // if (ImGui::Button("Print Simulation Settings")) {
                //     sim->printSimulationSettings();
                // }
                // if (ImGui::Button("Print All Settings")) {
                //     sim->printPhysicsSettings();
                //     // sim->hair->printHairSettings();
                //     sim->printSimulationSettings();
                // }
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Ticking", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                ImGui::Checkbox("Ticking", &CommonSim::ticking);
                UI::Help(
                    "If checked, the simulation will tick forward until the value in `targetTick`. "
                    "The target tick will only update when \"Tick Until\" is pressed. "
                    "The simulation will not run if the target tick is less than the current simulation tick. ");
                ImGui::Checkbox("Render While Ticking", &CommonSim::renderWhileTicking);
                ImGui::InputInt("Target Tick", &CommonSim::nextTick);
                if (ImGui::Button("Tick until")) {
                    sim->tickTo(CommonSim::nextTick);
                }
                ImGui::SameLine();
                if (ImGui::Button("Set to Current Tick")) {
                    CommonSim::nextTick = CommonSim::simulationTick;
                }
                ImGui::TreePop();
            }

            ImGui::Checkbox("Play", &CommonSim::play);
            ImGui::Text("Tick: %d%s", CommonSim::simulationTick, 
                (CommonSim::ticking &&  CommonSim::simulationTick >= CommonSim::nextTick) ? " (target reached)" : "");
            ImGui::TreePop();
        }
        ImGui::DragFloat3("Gravity", &CommonSim::fv_gravity.x, 0.1f, -200, 200);
        if (ImGui::CollapsingHeader("Hair\t\t\t", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::TreeNodeEx("Physics##Hair", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth)) {
                ImGui::DragFloat3("Torque", &sim->hair->torque.x, 0.1f, -100, 100);
                ImGui::DragFloat("Drag", &sim->hair->f_l_drag, 0.001f, 0, 1);
                ImGui::DragFloat("Angular Drag", &sim->hair->f_a_drag, 0.001f, 0, 1);
                ImGui::DragFloat("Clumping", &sim->hair->f_clumping, 0.001f, -100, 100);
                ImGui::DragFloat("Porosity", &sim->hair->f_porosity, 0.001f, -100, 100);
                ImGui::InputInt("Clumping Range", &sim->hair->clumpingRange);
                ImGui::TreePop();
            }
            if (ImGui::TreeNodeEx("Rendering##Hair", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth)) {
                ImGui::Checkbox("Show Head", &showHead);
                ImGui::DragFloat3("Head Position", &headStartPos.x, .1, -100, 300);
                ImGui::DragFloat3("Head Rotation", &headStartRot.x, .1, -1e9, 1e9);
                headStartRot = Util::wrapV(headStartRot, vec3(0), vec3(360));
                ImGui::ColorEdit4("Hair Colour", &sim->hair->hairColour.x);
                ImGui::DragFloat("Metalness", &sim->hair->metalness, 0.01, 0, 1);
                ImGui::DragFloat("Roughness", &sim->hair->roughness, 0.01, 0, 1);
                ImGui::DragFloat("Fresnel Exponent", &sim->hair->fresnelExponent, 0.01, 0, 10);
                ImGui::TreePop();
            }
        }
        if (ImGui::CollapsingHeader("Fluid\t\t\t", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::TreeNodeEx("Physics##Fluid", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                ImGui::SliderFloat("Epsilon", &sim->fluid->relaxationEpsilon, 0, 1);
                ImGui::SliderFloat("Rest Density", &sim->fluid->restDensity, 0.01, 1000);
                ImGui::SliderFloat("Smoothing Radius", &sim->fluid->smoothingRadius, 0.01, 1000);
                ImGui::DragFloat("Cohesion", &sim->fluid->f_cohesion, 0.001f, 0, 3000);
                ImGui::DragFloat("Curvature", &sim->fluid->f_curvature, 0.0001f, 0, 1);
                ImGui::DragFloat("Adhesion", &sim->fluid->f_adhesion, 0.1, 0, 10000);
                ImGui::DragFloat("Viscosity", &sim->fluid->f_viscosity, 0.001, 0, 3);
                ImGui::DragFloat("Diffusion", &sim->fluid->fluidMassDiffusionFactor, 0.01, 0, 10);
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Rendering##Fluid", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                ImGui::Checkbox("Show Fluid", &showFluid);
                ImGui::ColorEdit3("Fluid Colour", &sim->fluid->waterColour.x);
                ImGui::ColorEdit3("Attenuation Colour", &sim->fluid->attenuationColour.x);
                ImGui::DragFloat("Depth Strength", &sim->fluid->depthStrength, 0.1f, 0, 1000);
                ImGui::DragFloat("Thickness Strength", &sim->fluid->thicknessStrength, 0.001, 0, 1);
                ImGui::DragFloat("Threshold Ratio", &sim->fluid->thresholdRatio, 0.001, 0.00001, 10);
                ImGui::DragFloat("Clamp Ratio", &sim->fluid->clampRatio, 0.00001, 0.000001, 1);
                ImGui::SliderInt("Max Kernel Half Width", &sim->fluid->maxKernelHalfWidth, 1, 25);
                UI::Help(
                    "The maximum kernel size to use for the simlation. The actual kernel size is often too large, so this value limits it. "
                    "This will create a kernel of width 2 * n + 1.");
                if (ImGui::TreeNodeEx("Rendering Stage", ImGuiTreeNodeFlags_SpanAvailWidth)) {
                    ImGui::RadioButton("Sprite", &sim->fluid->renderStage, (int)Sim::PBF::SPRITE); ImGui::SameLine();
                    ImGui::RadioButton("Velocity", &sim->fluid->renderStage, (int)Sim::PBF::VELOCITY); ImGui::SameLine();
                    ImGui::RadioButton("Depth", &sim->fluid->renderStage, (int)Sim::PBF::DEPTH); ImGui::SameLine();
                    ImGui::RadioButton("Thickness", &sim->fluid->renderStage, (int)Sim::PBF::THICKNESS);
                    ImGui::RadioButton("Smooth Depth", &sim->fluid->renderStage, (int)Sim::PBF::SMOOTH_DEPTH); ImGui::SameLine();
                    ImGui::RadioButton("Smooth Thickness", &sim->fluid->renderStage, (int)Sim::PBF::SMOOTH_THICKNESS);
                    ImGui::RadioButton("Normal", &sim->fluid->renderStage, (int)Sim::PBF::NORMAL); ImGui::SameLine();
                    ImGui::RadioButton("Composition", &sim->fluid->renderStage, (int)Sim::PBF::COMPOSITION);
                    if (sim->fluid->renderStage == Sim::PBF::COMPOSITION)
                        ImGui::Checkbox("Show Diffuse Only", &sim->fluid->showDiffuseOnly);
                    ImGui::TreePop();
                }
                ImGui::Checkbox("Show Depth Test", &showMonkeys);
                ImGui::TreePop();
            }
        }
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glBindVertexArray(0);
}

// key pressed
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureKeyboard) {
        Input::updateKey(key, action);
        if (Input::isKeyPressed(Key::ESCAPE)) {
            glfwSetWindowShouldClose(window, true);
        }
        if (Input::isKeyJustReleased(Key::LEFT_ALT) && Input::windowFocused) {
            // prevent toggling camera mode when using alt+tab
            SM::cfg.isCamMode = !SM::cfg.isCamMode;
        }
    }
}

// mouse clicked
void click_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        Input::updateMouseClickState(button, action);
    }
}

// mouse moved
void mouse_pos_callback(GLFWwindow* window, double nx, double ny) {
    ImGuiIO& io = ImGui::GetIO();
    if (SM::cfg.isCamMode) {
        SM::camera->processView(nx - Input::mouse.x, ny - Input::mouse.y);
    }
    Input::updateMousePosition(nx, ny);
}

// Window focus
void window_focus_callback(GLFWwindow* window, int focused) {
    Input::windowFocused = focused;
}

int main(int argc, char const* argv[]) {
    // Set up OpenGL version (4.6)
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(SM::o_width, SM::o_height, "Real-Time Wet Hair", NULL, NULL);
    SM::updateWindow(SM::o_width, SM::o_height);

    if (!window) {
        printf("Failed to initialise GLFW window\n");
        glfwTerminate();
        return -1;
    }

    // Set up GLFW context
    glfwMakeContextCurrent(window);
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }

    // Set up GLFW callbacks
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  // resize
    glfwSetKeyCallback(window, key_callback);                           // key press
    glfwSetMouseButtonCallback(window, click_callback);                 // mouse click
    glfwSetCursorPosCallback(window, mouse_pos_callback);               // mouse pos
    glfwSetWindowFocusCallback(window, window_focus_callback);          // window focus

    glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_FASTEST);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);  // manually call callback, because dimensions are fucked otherwise
    glfwSetCursorPos(window, width / 2, height / 2);   // manually set mouse position before disabling cursor for initial camera position
    glfwSwapInterval(1);

    // Set up ImGui
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = 1.5;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    init();

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        Input::updateOldKeys();
        Input::updateOldButtons();
        glfwPollEvents();
        if (SM::cfg.isCamMode) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
        }
        update();
        display();
        displayUI();
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
