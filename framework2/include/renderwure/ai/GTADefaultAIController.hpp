#pragma once
#ifndef _GTADEFAULTAICONTROLLER_HPP_
#define _GTADEFAULTAICONTROLLER_HPP_
#include "renderwure/ai/GTAAIController.hpp"
#include <random>

struct GTAAINode;
class GTADefaultAIController : public GTAAIController
{
	enum Action
	{
		Wander
	};
	
	Action action;
	
	GTAAINode* targetNode;
	
	float nodeMargin; /// Minimum distance away to "reach" node.
	
public:
	
    GTADefaultAIController(GTACharacter* character)
	 : GTAAIController(character), action(Wander), targetNode(nullptr), nodeMargin(0.f) {}
	
    virtual void update(float dt);
	
    virtual glm::vec3 getTargetPosition();
	
    virtual glm::quat getTargetRotation();
};

#endif