#include "pch.h"

#include "source/Engine/Input.hpp"
#include "source/Engine/Window.hpp"

int main() {
    try {
        printf("Initializing application...\n");

        sf::Font font;
        if (!font.loadFromFile("resources/font/GN-Kin-iro_SansSerif.ttf"))
            throw EngineError("Failed to load font resource.\n");

        sf::Text fpsCounter;
        fpsCounter.setCharacterSize(16);
        fpsCounter.setPosition(8.0f, 8.0f);
        fpsCounter.setFillColor(sf::Color::White);
        fpsCounter.setString("");

        WindowMain* winMain = new WindowMain();
        winMain->Initialize();

        InputManager* inputManager = new InputManager();
        inputManager->Initialize();

        printf("Initialized application.\n");

        {
            sf::RenderWindow* window = winMain->GetWindow();

            //Refresh rate -> 60fps
            double target_ms = 1000.0 / 60;
            auto t_target_ms = std::chrono::duration<double, std::milli>(target_ms);

            auto current_time = std::chrono::high_resolution_clock::now();
            auto previous_time = current_time;
            auto max_frame_delay = std::chrono::milliseconds(33);

            auto accum_fps = std::chrono::nanoseconds(0);
            auto accum_update = std::chrono::nanoseconds(0);

            std::list<DWORD> listDelta;

            while (window->isOpen()) {
                {
                    sf::Event event;
                    while (window->pollEvent(event)) {
                        if (event.type == sf::Event::Closed)
                            window->close();
                    }
                }
                inputManager->Update();

                current_time = std::chrono::high_resolution_clock::now();
                auto delta_time = current_time - previous_time;
                if (delta_time > max_frame_delay)
                    delta_time = max_frame_delay;

                accum_fps += delta_time;
                accum_update += delta_time;

                if (accum_update > t_target_ms) {
                    listDelta.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count());

                    //Engine update

                    winMain->BeginScene();
                    //Engine render

                    {
                        //Render the FPS counter
                        window->pushGLStates();
                        winMain->SetBlendMode(BlendMode::Alpha);
                        window->draw(fpsCounter);
                        window->popGLStates();
                    }

                    winMain->EndScene();

                    accum_update = std::chrono::nanoseconds(0);
                }

                //5 fps updates per second
                if (accum_fps > std::chrono::milliseconds(200)) {
                    DWORD sumMs = 0U;
                    for (DWORD& iMs : listDelta)
                        sumMs += iMs;
                    listDelta.clear();

                    float fps = 1000.0f / (60.0f / (float)sumMs);
                    winMain->SetFPS(fps);

                    fpsCounter.setString(StringFormat("%.2f fps", fps));

                    accum_fps = std::chrono::nanoseconds(0);
                }
            }
        }

        printf("Finalizing application...\n");

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
