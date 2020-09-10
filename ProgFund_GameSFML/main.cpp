#include "pch.h"

#include "source/Engine/ResourceManager.hpp"
#include "source/Engine/Input.hpp"
#include "source/Engine/Window.hpp"
#include "source/Engine/Scene.hpp"

int main() {
    try {
        printf("Initializing application...\n");

        sf::Text fpsCounter;
        fpsCounter.setCharacterSize(16);
        fpsCounter.setPosition(0.0f, 0.0f);
        fpsCounter.setFillColor(sf::Color::White);
        fpsCounter.setString("");

        ResourceManager* resourceManager = new ResourceManager();
        resourceManager->Initialize();
        {
            std::string path = "D:/Visual Studio Projects/ProgFund_GameSFML/Debug/resource/font/GN-Kin-iro_SansSerif.ttf";
            auto fntKinIro = std::make_shared<FontResource>();
            fntKinIro->LoadFromFile(path);
            resourceManager->AddResource(fntKinIro, "GN-Kin-iro_SansSerif.ttf");

            fpsCounter.setFont(*fntKinIro->GetData());
        }

        WindowMain* winMain = new WindowMain();
        winMain->Initialize();

        InputManager* inputManager = new InputManager();
        inputManager->Initialize();

        SceneManager* sceneManager = new SceneManager();
        sceneManager->Initialize();

        printf("Initialized application.\n");

        {
            sf::RenderWindow* window = winMain->GetWindow();

            //Refresh rate -> 60fps
            auto t_target_ms = stdch::duration<double, std::milli>(1000.0f / 60);

            auto current_time = stdch::high_resolution_clock::now();
            auto previous_time = current_time;
            auto max_frame_delay = stdch::milliseconds(33);

            auto accum_fps = stdch::milliseconds(0);
            auto accum_update = stdch::milliseconds(0);

            std::list<uint64_t> listDelta;

            while (window->isOpen()) {
                {
                    sf::Event event;
                    while (window->pollEvent(event)) {
                        if (event.type == sf::Event::Closed)
                            window->close();
                    }
                }

                current_time = stdch::high_resolution_clock::now();
                auto delta_time = stdch::duration_cast<stdch::milliseconds>(current_time - previous_time);
                if (delta_time > max_frame_delay)
                    delta_time = max_frame_delay;

                accum_fps += delta_time;
                accum_update += delta_time;

                if (accum_update > t_target_ms) {
                    listDelta.push_back(delta_time.count());

                    //Engine update
                    inputManager->Update();
                    sceneManager->Update();
                    
                    //Engine render
                    winMain->BeginScene();
                    sceneManager->Render();
                    {
                        //Render the FPS counter
                        window->pushGLStates();
                        winMain->SetBlendMode(BlendMode::Alpha);
                        window->draw(fpsCounter);
                        window->popGLStates();
                    }
                    winMain->EndScene();

                    accum_update = stdch::milliseconds(0);
                }

                //2 fps updates per second
                if (accum_fps > stdch::milliseconds(500)) {
                    uint64_t sumMs = 0ui64;
                    for (uint64_t& iMs : listDelta)
                        sumMs += iMs;
                    listDelta.clear();

                    double fps = t_target_ms.count() / (double)sumMs;
                    winMain->SetFPS(fps);

                    fpsCounter.setString(StringFormat("%.2f fps", fps));
                    //printf("%.2f\n", fps);

                    accum_fps = stdch::milliseconds(0);
                }
            }
        }

        printf("Finalizing application...\n");

        ptr_delete(resourceManager);
        ptr_delete(sceneManager);
        ptr_delete(inputManager);
        ptr_release(winMain);

        printf("Finalized application.\n");
        return 0;
    }
    catch (std::exception& e) {
        printf("Unexpected error: %s", e.what());
        return -2;
    }
    catch (EngineError& e) {
        printf("Engine error: %s", e.what());
        return -1;
    }

    return 0;
}
