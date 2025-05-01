#pragma once
// Might need to change

#include "Model.h"

#include <memory>

struct Transform2DComponent {
	glm::vec2 translation{};
	glm::vec2 scale{ 1.0f ,1.0f };
	float rotation;						// Will use RADIANS
	
	glm::mat2 mat2() {
		const float sin = glm::sin(rotation);
		const float cos = glm::cos(rotation);
		glm::mat2 rotMat = { {cos, sin}, {-sin, cos} };

		glm::mat2 scaleMat{ {scale.x, 0.0f}, {0.0f, scale.y} };
		return rotMat * scaleMat;
	};
};

class Object
{
public:
	std::shared_ptr<Model> model{};
	glm::vec3 colour{};
	Transform2DComponent transfrom2D;

	static Object createObject() {
		static unsigned int currentObjectId = 0;
		return Object{ currentObjectId++ };
	};

	Object(const Object&) = delete;
	Object& operator=(const Object&) = delete;
	Object(Object&&) = default;
	Object& operator=(Object&&) = default;

	const unsigned int getObjectId() { return m_Id; };

	

private:
	unsigned int m_Id;

	Object(unsigned int objectId) : m_Id(objectId) {};
};

