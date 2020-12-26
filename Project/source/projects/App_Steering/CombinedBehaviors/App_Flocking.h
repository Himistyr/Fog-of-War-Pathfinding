#ifndef STEERINGBEHAVIORS_APPLICATION_H
#define STEERINGBEHAVIORS_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "../SteeringBehaviors.h"

class SteeringAgent;
class Flock;

//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_Flocking final : public IApp
{
public:
	//Constructor & Destructor
	App_Flocking() = default;
	virtual ~App_Flocking();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	//Datamembers
	TargetData m_MouseTarget = {};
	bool m_UseMouseTarget = false;
	bool m_VisualizeMouseTarget = true;
	
	//standard 100.f
	float m_TrimWorldSize = 200.f;

	//C++ make the class non-copyable
	App_Flocking(const App_Flocking&) = delete;
	App_Flocking& operator=(const App_Flocking&) = delete;

	Wander* m_pWander = nullptr;

	SteeringAgent* m_pAgentToEvade = nullptr;
	Flock* m_pFlock;
};
#endif