#include "LeakChecker.h"
#include"MyGame.h"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	LeakChecker leakChecker;

	std::unique_ptr<MyGame> myGame = std::make_unique<MyGame>();

	myGame->Run();

	return 0;
}