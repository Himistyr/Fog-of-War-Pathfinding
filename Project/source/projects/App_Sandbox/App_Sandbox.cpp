//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Sandbox.h"
#include "SandboxAgent.h"

//Destructor
App_Sandbox::~App_Sandbox()
{
	SAFE_DELETE(m_pSandboxAgent);
}

//Functions
void App_Sandbox::Start()
{
	m_pSandboxAgent = new SandboxAgent();
}

void App_Sandbox::Update(float deltaTime)
{
	if (INPUTMANAGER->IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		Elite::MouseData mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, (Elite::InputMouseButton::eLeft));
		m_pSandboxAgent->SetTarget(DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(Elite::Vector2(mouseData.X, mouseData.Y)));
	}
	m_pSandboxAgent->Update(deltaTime);
}

void App_Sandbox::Render(float deltaTime) const
{
	m_pSandboxAgent->Render(deltaTime);
}