#include "src/latex.hpp"

#include <locale>
#include <codecvt>
#include <chrono>

class Timer
{
    
    public:
        Timer() : m_StartTimepoint(std::chrono::high_resolution_clock::now()) { };
        
        Timer(const char* timer_name) : Timer()
        {
            m_name = timer_name;
        }

        ~Timer()
        {
            stop();
        }

        void stop()
        {
            auto endTimepoint = std::chrono::high_resolution_clock::now();

            auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

            auto duration = end - start;
            double ms = duration * .001;

            if (m_name.length() > 0)
                printf_s("[%s] execution time: %ld us (%lf ms)\n", m_name, duration, ms);
            else
                printf_s("Execution time: %ld us (%lf ms)\n", duration, ms);
        }

    private:
        Timer(const Timer& other);

        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
        std::string m_name;
};

int main(int argc, char* argv[]) {
    // #ifdef _WIN32
    //     system("chcp 65001");
    // #endif

    Timer timer;

    Latex latex;
    try
    {
        latex.setFonts("./fonts/OpenSans-Regular.ttf", "", "");

        std::string equation = argv[1];
        std::string filepath = argv[2];

        if (filepath.length() > 0)
            if (filepath.find(".png") != std::string::npos)
                latex.toPng(equation, filepath);
            else if (filepath.find(".jpg") != std::string::npos)
                latex.toJpg(equation, filepath);
            else if (filepath.find(".jpeg") != std::string::npos)
                latex.toJpg(equation, filepath);
    }
    catch(const Latex::ConversionException& err)
    {
        std::cerr << "Conversion Exception Error: \n" << err.what() << "\n";
    }
    catch(const Latex::FileException& err)
    {
        std::cerr << "File Exception Error: \n" << err.what() << "\n";
    }
    catch(const Latex::ParseException& err)
    {
        std::cerr << "Parse Exception Error: \n" << err.what() << "\n";
    }

};