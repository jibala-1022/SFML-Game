#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>


class MyShape
{
public:
    std::shared_ptr<sf::Shape> ptr;
    sf::Text name;
    sf::Vector2f velocity;
    MyShape(std::shared_ptr<sf::Shape> _ptr, sf::Text _name, sf::Vector2f _velocity)
        : ptr(_ptr), name(_name), velocity(_velocity) {}
    ~MyShape() {}
};


int main()
{
    std::string property;
    unsigned int windowX = 0, windowY = 0;
    std::string fontPath;
    int fontSize;
    std::string propName;
    float posX, posY, velX, velY;
    int colorR, colorG, colorB;
    float rectX, rectY, circleR;
    sf::Font font;
    sf::Text text;

    std::vector<MyShape> shapes;


    std::ifstream fin("config.txt");
    while (fin >> property)
    {
        if (property == "Window")
        {
            fin >> windowX >> windowY;
        }
        else if (property == "Fonts")
        {
            fin >> fontPath >> fontSize >> colorR >> colorG >> colorB;
            if (!font.loadFromFile("JetBrainsMono-Medium.ttf"))
            {
                std::cerr << "Unable to load font" << std::endl;
                std::getchar();
                return -1;
            }
            text.setFont(font);
            text.setCharacterSize(fontSize);
            text.setFillColor(sf::Color(colorR, colorG, colorB));
        }
        else if (property == "Circle")
        {
            fin >> propName >> posX >> posY >> velX >> velY >> colorR >> colorG >> colorB >> circleR;
            std::shared_ptr<sf::Shape> circle = std::make_shared<sf::CircleShape>(circleR);
            circle->setPosition(posX, posY);
            circle->setFillColor(sf::Color(colorR, colorG, colorB));
            text.setString(propName);
            shapes.push_back(MyShape(circle, text, sf::Vector2f(velX, velY)));
        }
        else if (property == "Rectangle")
        {
            fin >> propName >> posX >> posY >> velX >> velY >> colorR >> colorG >> colorB >> rectX >> rectY;
            std::shared_ptr<sf::Shape> rectangle = std::make_shared<sf::RectangleShape>(sf::Vector2f(rectX, rectY));
            rectangle->setPosition(posX, posY);
            rectangle->setFillColor(sf::Color(colorR, colorG, colorG));
            text.setString(propName);
            shapes.push_back(MyShape(rectangle, text, sf::Vector2f(velX, velY)));
        }
    }

    sf::RenderWindow window(sf::VideoMode(windowX, windowY), "Assignment 1");
    window.setFramerateLimit(60);

    
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.clear();

        for (size_t i = 0; i < shapes.size(); i++)
        {
            std::shared_ptr<sf::Shape> shape = shapes[i].ptr;
            sf::Text name = shapes[i].name;
            sf::Vector2f velocity = shapes[i].velocity;
            
            sf::FloatRect shapeBounds = shape->getLocalBounds();
            sf::FloatRect nameBounds = name.getLocalBounds();

            sf::Vector2f shapePosition = shape->getPosition();
            sf::Vector2f namePosition;

            shapePosition.x += velocity.x;
            shapePosition.y += velocity.y;

            if (shapePosition.x < 0)
            {
                shapePosition.x = 0;
                velocity.x *= -1.0f;
            }
            else if (shapePosition.x + shapeBounds.width > windowX)
            {
                shapePosition.x = windowX - shapeBounds.width;
                velocity.x *= -1.0f;
            }
            if (shapePosition.y < 0)
            {
                shapePosition.y = 0;
                velocity.y *= -1.0f;
            }
            else if (shapePosition.y + shapeBounds.height > windowY)
            {
                shapePosition.y = windowY - shapeBounds.height;
                velocity.y *= -1.0f;
            }
            shape->setPosition(shapePosition.x, shapePosition.y);
            shapes[i].velocity = velocity;
            window.draw(*shape);

            namePosition.x = shapePosition.x + shapeBounds.left - nameBounds.left + (shapeBounds.width - nameBounds.width) / 2.0f;
            namePosition.y = shapePosition.y + shapeBounds.top - nameBounds.top + (shapeBounds.height - nameBounds.height) / 2.0f;
            name.setPosition(namePosition.x, namePosition.y);
            window.draw(name);
        }

        window.display();
    }

    return 0;
}