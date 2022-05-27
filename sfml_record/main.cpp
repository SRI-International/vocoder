#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode(640, 480, 32), "Recording Sounds");

    // Ensure that there is a microphone
    if (!sf::SoundBufferRecorder::isAvailable())
    {
        std::cout << "You need a mic" << std::endl;
        return 1;
    }

    sf::SoundBufferRecorder recorder;
    sf::SoundBuffer buffer;
    sf::Sound sound;

    while(window.isOpen())
    {
        sf::Event Event;
        while(window.pollEvent(Event))
        {
            switch(Event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                if (Event.key.code == sf::Keyboard::R)
                {
                    std::cout << "Recording start" << std::endl;
                    recorder.start();
                }
                else if (Event.key.code == sf::Keyboard::P)
                {
                    std::cout << "Recording stop" << std::endl;
                    recorder.stop();
                    buffer = recorder.getBuffer();
                    buffer.saveToFile("test.ogg"); // Supports .wav, .ogg (not .mp3)
                    sound.setBuffer(buffer);
                    sound.play();
                }
                if (Event.key.code == sf::Keyboard::Q)
                {
                    std::cout << "Bye!" << std::endl;
                    window.close();
                    break;
                }
            }
        }

        window.clear(sf::Color::Red);
        window.display();
    }
    return 0;
}