#include <GraphicsAbstraction/GraphicsAbstraction.h>
#include "Application.h"

int main()
{
	GraphicsAbstraction::Log::Init();

	GraphicsAbstraction::Application application;
	application.Run();
	
	return 0;
}