#include <Core/Application.h>
#include <Core/Log.h>

int main()
{
	GraphicsAbstraction::Log::Init();

	GraphicsAbstraction::Application application;
	application.Run();

	return 0;
}