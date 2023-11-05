#include "Game.h"
#include <iostream>
#include <fstream>
#include <cmath>

Game::Game(const std::string& config)
{
	init(config);
}

void Game::init(const std::string& config)
{
	std::string setting;
	unsigned int width, height, limit, style;
	std::string fontPath;
	int fontSize, fR, fG, fB;

	std::ifstream fin(config);
	fin >> setting >> width >> height >> limit >> style;
	std::cout << setting << " " << width << " " << height << " " << limit << " " << style << std::endl;
	fin >> setting >> fontPath >> fontSize >> fR >> fG >> fB;
	std::cout << setting << " " << fontPath << " " << fontSize << " " << fR << " " << fG << " " << fB << std::endl;
	fin >> setting >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
	std::cout << setting << " " << m_playerConfig.SR << " " << m_playerConfig.CR << " " << m_playerConfig.S << " " << m_playerConfig.FR << " " << m_playerConfig.FG << " " << m_playerConfig.FB << " " << m_playerConfig.OR << " " << m_playerConfig.OG << " " << m_playerConfig.OB << " " << m_playerConfig.OT << " " << m_playerConfig.V << std::endl;
	fin >> setting >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
	std::cout << setting << " " << m_enemyConfig.SR << " " << m_enemyConfig.CR << " " << m_enemyConfig.SMIN << " " << m_enemyConfig.SMAX << " " << m_enemyConfig.OR << " " << m_enemyConfig.OG << " " << m_enemyConfig.OB << " " << m_enemyConfig.OT << " " << m_enemyConfig.VMIN << " " << m_enemyConfig.VMAX << " " << m_enemyConfig.L << " " << m_enemyConfig.SI << std::endl;
	fin >> setting >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V  >> m_bulletConfig.L;
	std::cout << setting << " " << m_bulletConfig.SR << " " << m_bulletConfig.CR << " " << m_bulletConfig.S << " " << m_bulletConfig.FR << " " << m_bulletConfig.FG << " " << m_bulletConfig.FB << " " << m_bulletConfig.OR << " " << m_bulletConfig.OG << " " << m_bulletConfig.OB << " " << m_bulletConfig.OT << " " << m_bulletConfig.V << " " << m_bulletConfig.L << std::endl;


	if (!m_font.loadFromFile(fontPath))
	{
		std::cerr << "Unable to load font";
		std::getchar();
		exit(1);
	}

	m_text.setFont(m_font);
	m_text.setCharacterSize(fontSize);
	m_text.setFillColor(sf::Color(fR, fG, fB));

	m_window.create(sf::VideoMode(width, height), "Assignment 2", (style) ? sf::Style::Fullscreen : sf::Style::Default);
	m_window.setFramerateLimit(limit);

	spawnPlayer();
}

void Game::run()
{
	while (m_running)
	{
		m_entityManager.update();

		sUserInput();

		if (!m_paused)
		{
			sEnemySpawner();
			sMovement();
			sCollision();

			m_currentFrame++;
		}

		sRender();
	}
}

void Game::setPaused(bool paused)
{
	m_paused = paused;
}


void Game::spawnPlayer()
{
	auto player = m_entityManager.addEntity("player");

	float halfWidth = m_window.getSize().x / 2.0f;
	float halfHeight = m_window.getSize().y / 2.0f;

	std::cout << halfWidth << " " << halfHeight << std::endl;

	player->cTransform = std::make_shared<CTransform>(Vec2(halfWidth, halfHeight), Vec2(0, 0), 0);

	player->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);

	player->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

	player->cInput = std::make_shared<CInput>();

	m_player = player;
}

void Game::spawnEnemy()
{
	auto enemy = m_entityManager.addEntity("enemy");

	int width = m_window.getSize().x;
	int height = m_window.getSize().y;

	float positionX = m_enemyConfig.CR + (float)rand() / RAND_MAX * (width - 2 * m_enemyConfig.CR);
	float positionY = m_enemyConfig.CR + (float)rand() / RAND_MAX * (height - 2 * m_enemyConfig.CR);

	float angle = (float)rand() / RAND_MAX * 360;

	float speed = m_enemyConfig.SMIN + (float)rand() / RAND_MAX * (m_enemyConfig.SMAX - m_enemyConfig.SMIN);
	Vec2 velocity(cos(angle), sin(angle));
	velocity *= speed;

	int points = m_enemyConfig.VMIN + (float)rand() / RAND_MAX * (m_enemyConfig.VMAX - m_enemyConfig.VMIN + 1);

	int fillR = (float)rand() / RAND_MAX * 256;
	int fillG = (float)rand() / RAND_MAX * 256;
	int fillB = (float)rand() / RAND_MAX * 256;

	enemy->cTransform = std::make_shared<CTransform>(Vec2(positionX, positionY), velocity, angle);

	enemy->cShape = std::make_shared<CShape>(m_enemyConfig.SR, points, sf::Color(fillR, fillG, fillB), sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);

	enemy->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
}

void Game::spawnSmallEnemy(std::shared_ptr<Entity> enemy)
{

}

void Game::spawnBullet(std::shared_ptr<Entity> player, const Vec2& mousePos)
{
	auto bullet = m_entityManager.addEntity("bullet");

	Vec2 direction = mousePos - player->cTransform->position;
	direction.normalize();

	Vec2 velocity = direction * m_bulletConfig.S;

	bullet->cTransform = std::make_shared<CTransform>(player->cTransform->position, velocity, 0);

	bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);

	bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);

	bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> player)
{

}



void Game::sMovement()
{
	CInput playerInput = *(m_player->cInput);

	Vec2 playerVelocity;

	if (playerInput.up)    playerVelocity.y--;
	if (playerInput.down)  playerVelocity.y++;
	if (playerInput.left)  playerVelocity.x--;
	if (playerInput.right) playerVelocity.x++;

	playerVelocity.normalize();
	playerVelocity *= m_playerConfig.S;

	m_player->cTransform->velocity = playerVelocity;

	for (auto& entity : m_entityManager.getEntities())
	{
		if (entity->cTransform)
		{
			entity->cTransform->position += entity->cTransform->velocity;
			if (entity->tag() == "player")
			{
				std::cout << entity->cTransform->velocity.x << " " << entity->cTransform->velocity.y << std::endl;
				std::cout << entity->cTransform->position.x << " " << entity->cTransform->position.y << std::endl;
			}
		}
	}
}

void Game::sUserInput()
{
	sf::Event event;
	while (m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			m_running = false;
		}

		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::P :
				setPaused(!m_paused);
				break;
			case sf::Keyboard::Escape :
				m_running = false;
				break;
			case sf::Keyboard::W :
				m_player->cInput->up = true;
				std::cout << "W pressed\n";
				break;
			case sf::Keyboard::A :
				m_player->cInput->left = true;
				std::cout << "A pressed\n";
				break;
			case sf::Keyboard::S :
				m_player->cInput->down = true;
				std::cout << "S pressed\n";
				break;
			case sf::Keyboard::D :
				m_player->cInput->right = true;
				std::cout << "D pressed\n";
				break;
			default:
				break;
			}
		}

		if (event.type == sf::Event::KeyReleased)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::W :
				m_player->cInput->up = false;
				std::cout << "W released\n";
				break;
			case sf::Keyboard::A :
				m_player->cInput->left = false;
				std::cout << "A released\n";
				break;
			case sf::Keyboard::S :
				m_player->cInput->down = false;
				std::cout << "S released\n";
				break;
			case sf::Keyboard::D :
				m_player->cInput->right = false;
				std::cout << "D released\n";
				break;
			default:
				break;
			}
		}

		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				std::cout << "Left mouse pressed at " << event.mouseButton.x << " " << event.mouseButton.y << std::endl;
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}
			if (event.mouseButton.button == sf::Mouse::Right)
			{
				std::cout << "Right mouse pressed at " << event.mouseButton.x << " " << event.mouseButton.y << std::endl;
				spawnSpecialWeapon(m_player);
			}
		}
	}
}

void Game::sLifespan()
{
	for (auto& entity : m_entityManager.getEntities())
	{
		if (entity->cLifespan)
		{
			entity->cLifespan->remaining--;

			if (entity->cLifespan->remaining <= 0)
			{
				entity->destroy();
			}
		}
	}
}

void Game::sRender()
{
	m_window.clear();

	for (auto& entity : m_entityManager.getEntities())
	{
		if (entity->cShape)
		{
			auto entityCircle = entity->cShape->circle;
			entityCircle.setPosition(entity->cTransform->position.x, entity->cTransform->position.y);
			entityCircle.setRotation(entity->cTransform->angle);

			entity->cTransform->angle++;

			sf::Color FC = entityCircle.getFillColor();
			sf::Color OC = entityCircle.getOutlineColor();

			float opacity = (entity->cLifespan) ? entity->cLifespan->remaining / entity->cLifespan->total * 255.0f : 255.0f;

			entityCircle.setFillColor(sf::Color(FC.r, FC.g, FC.b, opacity));
			entityCircle.setOutlineColor(sf::Color(OC.r, OC.g, OC.b, opacity));

			m_window.draw(entityCircle);
		}
	}

	m_window.display();
}

void Game::sEnemySpawner()
{
	if (m_currentFrame - m_lastEnemySpawnFrame >= m_enemyConfig.SI)
	{
		spawnEnemy();
		
		m_lastEnemySpawnFrame = m_currentFrame;
	}
}

void Game::sCollision()
{
	Vec2 windowSize(m_window.getSize().x, m_window.getSize().y);

	/*for (auto& entity : m_entityManager.getEntities())
	{
		if (entity->tag() == "enemy")
		{
			float cRadius = entity->cCollision->radius;
			Vec2 pos = entity->cTransform->position;
			Vec2 vel = entity->cTransform->velocity;
			if (pos.x < cRadius)
			{
				pos.x = cRadius;
				vel.x *= -1;
			}
			else if (pos.x > windowSize.x - cRadius)
			{
				pos.x = windowSize.x - cRadius;
				vel.x *= -1;
			}
			if (pos.y < cRadius)
			{
				pos.y = cRadius;
				vel.y *= -1;
			}
			else if (pos.y > windowSize.y - cRadius)
			{
				pos.y = windowSize.y - cRadius;
				vel.y *= -1;
			}
		}
	}*/
}
