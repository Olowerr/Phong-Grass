#include "Application/Application.h"

int main()
{
	startApplication(L"Phong Grass");
	
#ifdef DIST
	runPerformanceTests();
#else
	runEditorApplication();
#endif // DIST
	destroyApplication();


	return 0;
}