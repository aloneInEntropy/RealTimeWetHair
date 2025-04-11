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

    HairConfigs hs;
    /* Generate hairs */
    int rndCnt = 0;
    for (int i = 0; i < renderHead->vertexData.size(); ++i) {
        if (rndCnt >= 2048) break;
        if (renderHead->vertexData[i].pos.y >= 0) {
            // only normals facing out/up
            hs.push_back({20, vec4(renderHead->vertexData[i].pos, 1), renderHead->vertexData[i].norm});
            rndCnt++;
        }
    }

    headStartPos = vec3(150, 6, 150);
    FluidConfig fconfig = {5000, DAM_BREAK, vec3(0)};
    sim = new Sim::Simulation(hs, fconfig);
    sim->hair->headTrans = translate(mat4(1), headStartPos);

    SM::camera->lookAt(headStartPos + vec3(0, 30, 0));
    glPointSize(8.0);
    glLineWidth(3);

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
    // sim->update();
    SM::updateTick();
}

void display() {
    sim->fluid->envFBO->bind();
    glClearColor(0, 0, 0, 1);
    // glClearColor(SM::cfg.bgColour.r, SM::cfg.bgColour.g, SM::cfg.bgColour.b, 1);
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
    sim->fluid->envFBO->unbind();
    
    sim->fluid->render();
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
    if (ImGui::CollapsingHeader("Application\t\t\t", ImGuiTreeNodeFlags_DefaultOpen)) {
        // if (ImGui::CollapsingHeader("Hair\t\t\t", ImGuiTreeNodeFlags_DefaultOpen)) {
        //     if (ImGui::TreeNodeEx("Physics", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth)) {
        //         ImGui::DragFloat("Gravity", &Rods::gravity, 0.1f, -200, 200);
        //         ImGui::DragFloat3("Torque", &Rods::torque.x, 0.1f, -100, 100);
        //         ImGui::DragFloat("Drag", &Rods::drag, 0.001f, 0, 1);
        //         ImGui::DragFloat("Angular Drag", &Rods::a_drag, 0.001f, 0, 1);
        //         ImGui::NewLine();
        //         ImGui::TreePop();
        //     }
        //     if (ImGui::TreeNodeEx("Hair", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth)) {
        //         ImGui::DragFloat3("Head Position", &headStartPos.x, .1, -100, 300);
        //         ImGui::DragFloat3("Head Rotation", &headStartRot.x, .1, -1e9, 1e9);
        //         headStartRot = Util::wrapV(headStartRot, vec3(0), vec3(360));
        //         ImGui::ColorEdit4("Hair Colour", &Rods::hairColour.x);
        //         ImGui::ColorEdit4("Guide Colour", &Rods::guideColour.x);
        //         ImGui::Checkbox("Show Head", &showHead);
        //         ImGui::SameLine();
        //         ImGui::Checkbox("Show Lines", &Rods::showLines);
        //         ImGui::SameLine();
        //         ImGui::Checkbox("Show Points", &Rods::showPoints);
        //         ImGui::NewLine();
        //         ImGui::TreePop();
        //     }
        //     if (ImGui::TreeNodeEx("Settings", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth)) {
        //         if (ImGui::TreeNodeEx("Constraints", ImGuiTreeNodeFlags_SpanAvailWidth)) {
        //             ImGui::DragFloat("SS Stiffness", &Rods::ss_k, .0001, 0.01, 1);
        //             ImGui::DragFloat("SS SOR", &Rods::ss_SOR, .0001, 0.01, 10);
        //             UI::Help(
        //                 "[UPP14]\n"
        //                 "Successive over-relaxation value for the Stretch and Shear constraint.\n");
        //             ImGui::NewLine();
        //             ImGui::DragFloat("BT Stiffness", &Rods::bt_k, .0001, 0.01, 1);
        //             ImGui::DragFloat("BT SOR", &Rods::bt_SOR, .0001, 0.01, 10);
        //             UI::Help(
        //                 "[UPP14]\n"
        //                 "Successive over-relaxation value for the Bend and Twist constraint.\n");
        //             ImGui::DragInt("Substeps", &Rods::simulationSubsteps, .1, 1, 20);
        //             ImGui::DragInt("Iterations", &Rods::ii, .1, 1, 20);
        //             if (ImGui::Button("Print Physics Settings")) {
        //                 sim->hair->printPhysicsSettings();
        //             }
        //             ImGui::SameLine();
        //             if (ImGui::Button("Print Hair Settings")) {
        //                 sim->hair->printHairSettings();
        //             }
        //             ImGui::SameLine();
        //             if (ImGui::Button("Print Simulation Settings")) {
        //                 sim->hair->printSimulationSettings();
        //             }
        //             if (ImGui::Button("Print All Settings")) {
        //                 sim->hair->printPhysicsSettings();
        //                 // sim->hair->printHairSettings();
        //                 sim->hair->printSimulationSettings();
        //             }
        //             ImGui::TreePop();
        //         }
        //         ImGui::Checkbox("Play", &Rods::play);
        //         if (ImGui::Button("Step")) {
        //             sim->tick();
        //         }
        //         ImGui::SameLine();
        //         ImGui::Text("Tick: %d", Rods::simulationTick);

        //         ImGui::TreePop();
        //     }
        // }
        // if (ImGui::CollapsingHeader("Fluid\t\t\t", ImGuiTreeNodeFlags_DefaultOpen)) {
        //     if (ImGui::TreeNodeEx("Physics", ImGuiTreeNodeFlags_SpanAvailWidth)) {
        //         ImGui::SliderFloat("Epsilon", &PBF::relaxationEpsilon, 0, 1);
        //         ImGui::SliderFloat("Rest Density", &PBF::restDensity, 0.01, 1000);
        //         ImGui::SliderFloat("Smoothing Radius", &PBF::smoothingRadius, 0.01, 1000);
        //         ImGui::DragFloat3("Gravity", &PBF::gravityDir.x, 0.1f, -1000, 1000);
        //         ImGui::DragFloat("Cohesion", &PBF::f_cohesion, 0.1f, 0, 3000);
        //         ImGui::DragFloat("Curvature", &PBF::f_curvature, 0.0001f, 0, 1);
        //         ImGui::SliderFloat("Adhesion", &PBF::f_adhesion, 0, 10000);
        //         ImGui::SliderFloat("Viscosity", &PBF::f_viscosity, 0, 1);
        //         ImGui::TreePop();
        //     }

        //     if (ImGui::TreeNodeEx("Simulation", ImGuiTreeNodeFlags_SpanAvailWidth)) {
        //         ImGui::DragFloat("SOR", &PBF::SOR, 0.01f, 0.01, 8);
        //         ImGui::SliderInt("Substeps", &PBF::simulationSubsteps, 1, 20);
        //         ImGui::TreePop();
        //     }

        //     if (ImGui::TreeNodeEx("Rendering", ImGuiTreeNodeFlags_SpanAvailWidth)) {
        //         ImGui::ColorEdit3("Fluid Colour", &PBF::waterColour.x);
        //         ImGui::ColorEdit3("Attenuation Colour", &PBF::attenuationColour.x);
        //         ImGui::DragFloat("Depth Strength", &PBF::depthStrength, 0.1f, 0, 1000);
        //         ImGui::DragFloat("Thickness Strength", &PBF::thicknessStrength, 0.001, 0, 1);
        //         ImGui::DragFloat("Threshold Ratio", &PBF::thresholdRatio, 0.001, 0.00001, 10);
        //         ImGui::DragFloat("Clamp Ratio", &PBF::clampRatio, 0.00001, 0.000001, 1);
        //         ImGui::SliderInt("Max Kernel Half Width", &PBF::maxKernelHalfWidth, 1, 25);
        //         UI::Help(
        //             "The maximum kernel size to use for the simlation. The actual kernel size is often too large, so this value limits it. "
        //             "This will create a kernel of width 2 * n + 1.");
        //         if (ImGui::TreeNodeEx("Rendering Stage", ImGuiTreeNodeFlags_SpanAvailWidth)) {
        //             ImGui::RadioButton("Sprite", &PBF::renderStage, (int)PBF::FluidRenderStage::SPRITE);
        //             ImGui::RadioButton("Velocity", &PBF::renderStage, (int)PBF::FluidRenderStage::VELOCITY);
        //             ImGui::RadioButton("Depth", &PBF::renderStage, (int)PBF::FluidRenderStage::DEPTH);
        //             ImGui::RadioButton("Thickness", &PBF::renderStage, (int)PBF::FluidRenderStage::THICKNESS);
        //             ImGui::RadioButton("Smooth Depth", &PBF::renderStage, (int)PBF::FluidRenderStage::SMOOTH_DEPTH);
        //             ImGui::RadioButton("Smooth Thickness", &PBF::renderStage, (int)PBF::FluidRenderStage::SMOOTH_THICKNESS);
        //             ImGui::RadioButton("Normal", &PBF::renderStage, (int)PBF::FluidRenderStage::NORMAL);
        //             ImGui::RadioButton("Composition", &PBF::renderStage, (int)PBF::FluidRenderStage::COMPOSITION);
        //             if (PBF::renderStage == PBF::FluidRenderStage::COMPOSITION)
        //                 ImGui::Checkbox("Show Diffuse Only", &PBF::showDiffuseOnly);
        //             ImGui::TreePop();
        //         }
        //         ImGui::Checkbox("Show Depth Test", &showMonkeys);
        //         ImGui::TreePop();
        //     }

        //     if (ImGui::TreeNodeEx("Ticking", ImGuiTreeNodeFlags_SpanAvailWidth)) {
        //         ImGui::Checkbox("Ticking", &PBF::ticking);
        //         UI::Help(
        //             "If checked, the simulation will tick forward until the value in `targetTick`. "
        //             "The target tick will only update when \"Tick Until\" is pressed. "
        //             "The simulation will not run if the target tick is less than the current simulation tick. ");
        //         ImGui::Checkbox("Render While Ticking", &PBF::renderWhileTicking);
        //         ImGui::InputInt("Target Tick", &PBF::nextTick);
        //         if (ImGui::Button("Tick until")) {
        //             sim->tickTo(PBF::nextTick);
        //         }
        //         ImGui::SameLine();
        //         if (ImGui::Button("Set to Current Tick")) {
        //             PBF::nextTick = PBF::simulationTick;
        //         }
        //         ImGui::TreePop();
        //     }

        //     ImGui::Checkbox("Play", &PBF::play);
        //     ImGui::Text("Tick: %d%s", PBF::simulationTick, PBF::simulationTick >= PBF::nextTick ? " (target reached)" : "");
        // }
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
