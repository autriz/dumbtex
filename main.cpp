#include "src/Latex.hpp"

//Allows compilation from c++11, haven't checked runtime part
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
	catch(const Latex::ConversionException& err)
	{
		std::cerr << "Conversion Exception Error: " << err.what() << "\n";
	}
	catch(const Latex::FileException& err)
	{
		std::cerr << "File Exception Error: " << err.what() << "\n";
	}
	catch(const Latex::ParseException& err)
	{
		std::cerr << "Parse Exception Error: " << err.what() << "\n";
	}

	#if defined(DEBUG) || defined(PROFILER)
	}
	Instrumentor::Get().EndSession();
	#endif
	
};