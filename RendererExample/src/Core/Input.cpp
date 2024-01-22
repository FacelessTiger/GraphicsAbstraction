#include <Core/Input.h>
#include <Core/Application.h>

namespace GraphicsAbstraction {

	bool Input::IsKeyPressed(int keycode)
	{
		return Application::Get().GetWindow()->IsKeyPressed(keycode);
	}

	bool Input::IsMouseButtonPressed(int button)
	{
		return Application::Get().GetWindow()->IsMouseButtonPressed(button);
	}

	glm::vec2 Input::GetMousePosition()
	{
		return Application::Get().GetWindow()->GetMousePosition();
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().y;
	}

}