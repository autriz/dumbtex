#include "src/Latex.hpp"

//Allows compilation from c++11, haven't checked runtime part
int main(int argc, char* argv[]) {
    
    #if defined(DEBUG) || defined(PROFILER)
    Instrumentor::Get().BeginSession("Profile");
    {
    #endif
    
    PROFILE_FUNCTION();

    Latex latex;
    try
    {
        //create runtime font setter
        latex.setFonts("./fonts/OpenSans-Regular.ttf", "./fonts/OpenSans-Italic.ttf", "./fonts/OpenSans-Bold.ttf", "./fonts/OpenSans-BoldItalic.ttf");

        std::string equation = argv[1];
        const char* filepath = argv[2];

        if (argc >= 3)
            switch(Image::getFileType(filepath))
            {
                case ImageType::PNG:
                    latex.toPNG(equation, filepath);
                    break;
                case ImageType::JPG:
                    latex.toJPG(equation, filepath);
                    break;
                case ImageType::BMP:
                    latex.toBMP(equation, filepath);
                    break;
                case ImageType::TGA:
                    latex.toTGA(equation, filepath);
                    break;   
                case ImageType::NO_TYPE:
                    throw Latex::FileException("No file type.", __FILE__, __LINE__); 
                case ImageType::UNKNOWN:
                    throw Latex::FileException("Unknown file type.", __FILE__, __LINE__);
            }
    
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