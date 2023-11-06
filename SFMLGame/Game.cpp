#include "Game.h"
#include <iostream>
#include <fstream>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

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
	fin >> setting >> fontPath >> fontSize >> fR >> fG >> fB;
	fin >> setting >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
	fin >> setting >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
	fin >> setting >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V  >> m_bulletConfig.L;


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
			sLifespan();
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
	float halfWidth = m_window.getSize().x / 2.0f;
	float halfHeight = m_window.getSize().y / 2.0f;

	if (m_player)
	{
		m_player->cTransform = std::make_shared<CTransform>(Vec2(halfWidth, halfHeight), Vec2(0, 0), 0);
		return;
	}

	auto player = m_entityManager.addEntity("player");

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

	Vec2 playerPos = m_player->cTransform->position;
	float playerCR = m_player->cCollision->radius;

	float positionX, positionY;
	do {
		positionX = m_enemyConfig.CR + (float)rand() / RAND_MAX * (width - 2 * m_enemyConfig.CR);
		positionY = m_enemyConfig.CR + (float)rand() / RAND_MAX * (height - 2 * m_enemyConfig.CR);
	} while ((positionX - playerPos.x) * (positionX - playerPos.x) + (positionY - playerPos.y) * (positionY - playerPos.y) <= 4.0f * playerCR * playerCR);

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

	enemy->cScore = std::make_shared<CScore>(points * 100);
}

void Game::spawnSmallEnemy(std::shared_ptr<Entity> enemy)
{
	int points = enemy->cShape->circle.getPointCount();

	sf::Color fill = enemy->cShape->circle.getFillColor();

	Vec2 position = enemy->cTransform->position;

	float speed = enemy->cTransform->velocity.length();

	float angle = enemy->cTransform->angle;

	float dAngleRad = 2.0f * M_PI / points;
	float angleRad = 2.0f * M_PI * angle / 360.0f;

	for (int i = 0; i < points; i++)
	{
		auto enemy = m_entityManager.addEntity("smallenemy");

		float dirAngleRad = angleRad + i * dAngleRad;

		Vec2 direction(cos(dirAngleRad), sin(dirAngleRad));
		Vec2 velocity = direction * speed;

		enemy->cTransform = std::make_shared<CTransform>(position, velocity, angle);

		enemy->cShape = std::make_shared<CShape>(m_enemyConfig.SR / 2.0f, points, fill, sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);

		enemy->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR / 2.0f);

		enemy->cScore = std::make_shared<CScore>(points * 200);

		enemy->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
	}
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
		}
	}
}


void Game::sUserInput()
{
	sf::Event event;

	m_player->cInput->shoot = false;

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
				break;
			case sf::Keyboard::A :
				m_player->cInput->left = true;
				break;
			case sf::Keyboard::S :
				m_player->cInput->down = true;
				break;
			case sf::Keyboard::D :
				m_player->cInput->right = true;
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
				break;
			case sf::Keyboard::A :
				m_player->cInput->left = false;
				break;
			case sf::Keyboard::S :
				m_player->cInput->down = false;
				break;
			case sf::Keyboard::D :
				m_player->cInput->right = false;
				break;
			default:
				break;
			}
		}

		if (event.type == sf::Event::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Left)
			{
				m_player->cInput->shoot = true;
				spawnBullet(m_player, Vec2(event.mouseButton.x, event.mouseButton.y));
			}
			if (event.mouseButton.button == sf::Mouse::Right)
			{
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
			if (entity->cTransform->angle >= 360.0f)
			{
				entity->cTransform->angle -= 360.0f;
			}

			sf::Color FC = entityCircle.getFillColor();
			sf::Color OC = entityCircle.getOutlineColor();

			float opacity = (entity->cLifespan) ? 255.0f * entity->cLifespan->remaining / entity->cLifespan->total : 255.0f;

			entityCircle.setFillColor(sf::Color(FC.r, FC.g, FC.b, opacity));
			entityCircle.setOutlineColor(sf::Color(OC.r, OC.g, OC.b, opacity));

			m_window.draw(entityCircle);
		}
	}

	sf::Vector2u windowSize = m_window.getSize();
	sf::FloatRect bounds = m_text.getLocalBounds();

	m_text.setString("Score: " + std::to_string(m_score));
	m_text.setPosition(windowSize.x - (2.0f * bounds.left + bounds.width), windowSize.y - (2.0f * bounds.top + bounds.height));
	m_window.draw(m_text);
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

	auto checkCollisionBoundary = [windowSize](std::shared_ptr<Entity>& entity)
		{
			Vec2& entityPos = entity->cTransform->position;
			Vec2& entityVel = entity->cTransform->velocity;
			float entityCR = entity->cCollision->radius;

			if (entityPos.x < entityCR)
			{
				entityPos.x = entityCR;
				entityVel.x *= -1.0f;
			}
			else if (entityPos.x > windowSize.x - entityCR)
			{
				entityPos.x = windowSize.x - entityCR;
				entityVel.x *= -1.0f;
			}
			if (entityPos.y < entityCR)
			{
				entityPos.y = entityCR;
				entityVel.y *= -1.0f;
			}
			else if (entityPos.y > windowSize.y - entityCR)
			{
				entityPos.y = windowSize.y - entityCR;
				entityVel.y *= -1.0f;
			}
		};

	auto checkCollisionDestroy = [](std::shared_ptr<Entity>& entityA, std::shared_ptr<Entity>& entityB) -> bool
		{
			float crA = entityA->cCollision->radius;
			float crB = entityB->cCollision->radius;

			Vec2 displacement = entityB->cTransform->position - entityA->cTransform->position;

			return (displacement.x * displacement.x + displacement.y * displacement.y < (crA + crB) * (crA + crB));
		};



	for (auto& enemy : m_entityManager.getEntities("enemy"))
	{
		if (checkCollisionDestroy(m_player, enemy))
		{
			m_score /= 2;
			spawnPlayer();

			enemy->destroy();
		}
	}

	for (auto& smallenemy : m_entityManager.getEntities("smallenemy"))
	{
		if (checkCollisionDestroy(m_player, smallenemy))
		{
			m_score /= 2;
			spawnPlayer();

			smallenemy->destroy();
		}
	}

	for (auto& bullet : m_entityManager.getEntities("bullet"))
	{
		for (auto& enemy : m_entityManager.getEntities("enemy"))
		{
			if (checkCollisionDestroy(bullet, enemy))
			{
				m_score += enemy->cScore->score;
				spawnSmallEnemy(enemy);

				bullet->destroy();
				enemy->destroy();
			}
		}

		for (auto& smallenemy : m_entityManager.getEntities("smallenemy"))
		{
			if (checkCollisionDestroy(bullet, smallenemy))
			{
				m_score += smallenemy->cScore->score;

				bullet->destroy();
				smallenemy->destroy();
			}
		}
	}

	checkCollisionBoundary(m_player);

	for (auto& enemy : m_entityManager.getEntities("enemy"))
	{
		checkCollisionBoundary(enemy);
	}

}
