#include "Test.h"

void Test::Init()
{

}

void Test::OnClose()
{

}

Application* CreateApplication(ApplicationInfo* appInfo)
{
	appInfo->renderAPI = RENDER_API_VULKAN;
	appInfo->versionString = "1.0.0";
	appInfo->displayInfo.title = "Test";
	appInfo->displayInfo.width = 1920;
	appInfo->displayInfo.height = 1080;

	return new Test();
}