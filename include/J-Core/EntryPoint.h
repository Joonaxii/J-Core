#pragma once 
#include <J-Core/Application.h>
#include <windows.h>
#pragma execution_character_set( "utf-8" )

extern JCore::Application* JCore::createApplication(AppArgs args);

int main(int argc, char** argv)
{
	SetConsoleOutputCP(65001);
	setlocale(LC_ALL, "C");
	setlocale(LC_CTYPE, ".UTF8");

	JCore::Log::init();
	JCORE_TRACE("Initializing J-Core");

	auto app = JCore::createApplication({ argc, argv });
	if (app->initialize()) {
		app->run();
	}

	JCORE_TRACE("Destroying J-Core");
	delete app;
}
