#ifndef INFLUENCE_MAP_APPLICATION_H
#define INFLUENCE_MAP_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"

#include "framework\EliteAI\EliteGraphs\EGraphNodeTypes.h"
#include "framework\EliteAI\EliteGraphs\EGraphConnectionTypes.h"
#include "framework\EliteAI\EliteGraphs\EGraph2D.h"

#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"

//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_GraphTheory final : public IApp
{
public:
	//Constructor & Destructor
	App_GraphTheory() = default;
	virtual ~App_GraphTheory() final;

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	Elite::Graph2D<Elite::GraphNode2D, Elite::GraphConnection2D>* m_pGraph2D;
	vector<Elite::GraphNode2D*> m_Path;

	//C++ make the class non-copyable
	App_GraphTheory(const App_GraphTheory&) = delete;
	App_GraphTheory& operator=(const App_GraphTheory&) = delete;

	Elite::EGraphRenderer m_GraphRenderer{};
};
#endif