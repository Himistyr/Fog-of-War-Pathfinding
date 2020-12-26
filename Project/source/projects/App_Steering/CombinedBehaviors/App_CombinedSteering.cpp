//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_CombinedSteering.h"
#include "../SteeringAgent.h"
#include "CombinedSteeringBehaviors.h"
#include "projects\App_Steering\Obstacle.h"

App_CombinedSteering::~App_CombinedSteering()
{
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pDrunkWander);
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pDrunkAgent);

	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pSoberWander);
	SAFE_DELETE(m_pEvade);
	SAFE_DELETE(m_pSoberAgent);
}

void App_CombinedSteering::Start()
{
	//Drunk Steering
	m_pSeek = new Seek();
	m_pDrunkWander = new Wander();
	//m_pDrunkWander->SetWanderOffset(0);
	m_pBlendedSteering = new BlendedSteering({ {m_pSeek, 0.5f}, {m_pDrunkWander, 0.5f} });

	m_pDrunkAgent = new SteeringAgent();
	m_pDrunkAgent->SetSteeringBehavior(m_pBlendedSteering);
	m_pDrunkAgent->SetMaxAngularSpeed(15.f);
	m_pDrunkAgent->SetAutoOrient(true);
	m_pDrunkAgent->SetMass(1.f);
	m_pDrunkAgent->SetBodyColor({1, 0, 0});

	//Sober steering
	m_pSoberWander = new Wander();
	m_pEvade = new Evade();
	m_pPrioritySteering = new PrioritySteering({m_pEvade, m_pSoberWander});

	m_pSoberAgent = new SteeringAgent();
	m_pSoberAgent->SetSteeringBehavior(m_pPrioritySteering);
	m_pSoberAgent->SetMaxAngularSpeed(15.f);
	m_pSoberAgent->SetAutoOrient(true);
	m_pSoberAgent->SetMass(1.f);
}

void App_CombinedSteering::Update(float deltaTime)
{
	//INPUT
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeMouseTarget)
	{
		auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
		m_MouseTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	}

#ifdef PLATFORM_WINDOWS
	#pragma region UI
	//UI
	{
		//Setup
		int const menuWidth = 235;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Checkbox("Debug Rendering", &m_CanDebugRender);
		ImGui::Checkbox("Trim World", &m_TrimWorld);
		if (m_TrimWorld)
		{
			ImGui::SliderFloat("Trim Size", &m_TrimWorldSize, 0.f, 500.f, "%1.");
		}
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Behaviour Weights");
		ImGui::Spacing();

		ImGui::SliderFloat("Seek", &m_pBlendedSteering->m_WeightedBehaviors[0].weight, 0.f, 1.f, "%.2");
		ImGui::SliderFloat("Wander", &m_pBlendedSteering->m_WeightedBehaviors[1].weight, 0.f, 1.f, "%.2");
		

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	#pragma endregion
#endif

	m_pSeek->SetTarget(m_MouseTarget);
	m_pDrunkAgent->Update(deltaTime);
	m_pDrunkAgent->TrimToWorld(m_TrimWorldSize);

	TargetData evadeTarget;
	evadeTarget.LinearVelocity = m_pDrunkAgent->GetLinearVelocity();
	evadeTarget.Position = m_pDrunkAgent->GetPosition();

	m_pEvade->SetTarget(evadeTarget);
	m_pSoberAgent->Update(deltaTime);
	m_pSoberAgent->TrimToWorld(m_TrimWorldSize);

}

void App_CombinedSteering::Render(float deltaTime) const
{
	m_pDrunkAgent->SetRenderBehavior(m_CanDebugRender);
	m_pDrunkAgent->Render(deltaTime);

	m_pSoberAgent->SetRenderBehavior(m_CanDebugRender);
	m_pSoberAgent->Render(deltaTime);

	if (m_TrimWorld)
	{
		std::vector<Elite::Vector2> points =
		{
			{ -m_TrimWorldSize,m_TrimWorldSize },
			{ m_TrimWorldSize,m_TrimWorldSize },
			{ m_TrimWorldSize,-m_TrimWorldSize },
			{-m_TrimWorldSize,-m_TrimWorldSize }
		};
		DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);
	}

	//Render Target
	if(m_VisualizeMouseTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_MouseTarget.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f },-0.8f);
}
