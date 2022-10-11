#include "src/Latex.hpp"

//Compiles from c++11
int main(int argc, char* argv[]) {
	
	#if defined(DEBUG) || defined(PROFILER)
	Instrumentor::Get().BeginSession("Profile");
	{
	#endif
	
	PROFILE_FUNCTION();

	std::string equation = argv[1];
	const char* filepath = argv[2];
	Latex latex;

	try
	{
		latex.setFonts("./fonts/OpenSans-Regular.ttf", "./fonts/OpenSans-Italic.ttf", "./fonts/OpenSans-Bold.ttf", "./fonts/OpenSans-BoldItalic.ttf");

		if (argc >= 3)
			latex.toImage(equation).write(filepath);
	
	}
	catch(const std::runtime_error& err)
	{
		std::cerr << "Runtime Error: " << err.what() << "\n";
	}

	#if defined(DEBUG) || defined(PROFILER)
	}
	Instrumentor::Get().EndSession();
	#endif
	
};