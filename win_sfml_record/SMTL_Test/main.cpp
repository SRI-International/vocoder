#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>

int main()
{
    // Create a 640 x 480 window with the specified string title
    // "Simple Sound Recorder"
    sf::RenderWindow window(sf::VideoMode(640, 240, 32), "Simple Sound Recorder");

    // Ensure that there is a microphone/recording device available
    // If not, error out of the program.
    if (!sf::SoundBufferRecorder::isAvailable())
    {
        std::cerr << "Error: No available audio device detected." << std::endl;
        return 1;
    }

    // Relevant sound components
    sf::SoundBufferRecorder recorder;
    sf::SoundBuffer buffer;
    sf::Sound sound;

    // Text rendering components
    // First load the font needed (ensure no errors)
    sf::Font font;
    if (!font.loadFromFile("arial.ttf"))
    {
        std::cerr << "Error: Font file could not be loaded." << std::endl;
        return 1;
    }
    // Setting some simple GUI text to explain what you can do.
    sf::Text text;
    text.setFont(font);
    text.setString
    ("Press 'R' to record.\nPress 'S' to stop and save recording.\nPress 'P' to playback recent recording.\nPress 'Q' to quit (WARNING: Save before quitting!)");
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);

    while (window.isOpen())
    {
        sf::Event Event;
        while (window.pollEvent(Event))
        {
            switch (Event.type)
            {
                // Pressing 'X' in the window corner.
            case sf::Event::Closed:
                window.close();
                break;
                // Define behavior for certain key presses.
            case sf::Event::KeyPressed:
                // Recording audio
                if (Event.key.code == sf::Keyboard::R)
                {
                    std::cout << "Recording start. Begin speaking now..." << std::endl;
                    recorder.start();
                }
                // Stopping the recording 
                else if (Event.key.code == sf::Keyboard::S)
                {
                    std::cout << "Recording stopped." << std::endl;
                    recorder.stop();

                    std::string filename;
                    std::cout << "Provide a name for this file: ";
                    getline(std::cin, filename);

                    if (filename.empty())
                    {
                        std::cerr << "Warning: No file name provided. Using default of 'output.ogg'." << std::endl;
                        filename = "output.ogg";
                    }
                    else
                    {
                        filename += ".ogg";
                    }

                    buffer = recorder.getBuffer();
                    buffer.saveToFile(filename); // Supports .wav, .ogg (not .mp3)
                    std::cout << "Recording saved as '" << filename << "'." << std::endl;
                }
                else if (Event.key.code == sf::Keyboard::P)
                {
                    std::cout << "Playing back your recording..." << std::endl;
                    if (buffer.getDuration() == sf::seconds(0.0f))
                    {
                        std::cerr << "Warning: No sound has been recorded yet" << std::endl;
                        continue;
                    }
                    sound.setBuffer(buffer);
                    sound.play();
                    std::cout << "Playback done." << std::endl;
                }
                if (Event.key.code == sf::Keyboard::Q)
                {
                    std::cout << "Bye!" << std::endl;
                    window.close();
                    break;
                }
            }
        }

        window.draw(text);
        // window.clear(sf::Color(0,0,0));
        window.display();
    }

    return 0;
}