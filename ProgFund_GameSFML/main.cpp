#include "pch.h"

#include "source/Engine/Window.hpp"

int main() {
    try {
        printf("Initializing application...");

        WindowMain* winMain = new WindowMain();
        winMain->Initialize();

        printf("Initialized application.");

        {
            sf::Window* window = winMain->GetWindow();

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

                        //Process input here
                    }
                }

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
                    winMain->EndScene();

                    accum_update = std::chrono::nanoseconds(0);
                }

                //5 fps updates per second
                if (accum_fps > std::chrono::milliseconds(200)) {
                    DWORD sumMs = 0U;
                    for (DWORD& iMs : listDelta)
                        sumMs += iMs;
                    listDelta.clear();

                    winMain->SetFPS(1000.0f / (60.0f / (float)sumMs));
                    accum_fps = std::chrono::nanoseconds(0);
                }
            }
        }

        printf("Finalizing application...");

        ptr_release(winMain);

        printf("Finalized application.");
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
