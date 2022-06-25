#include "src/latex.hpp"

#include <locale>
#include <codecvt>
#include <iostream>

#include <chrono>
#include <string>

class Timer
{
    
    public:
        Timer()
        {
            m_StartTimepoint = std::chrono::high_resolution_clock::now();
        }
        Timer(const char* timer_name) : Timer()
        {
            name = timer_name;
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

            if (name.length() > 0)
                printf_s("[%s] execution time: %ld us (%lf ms)\n", duration, ms);
            else
                printf_s("Execution time: %ld us (%lf ms)\n", duration, ms);
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
        std::string name;
};

int main(int argc, char* argv[]) {
    // #ifdef _WIN32
    //     system("chcp 65001");
    // #endif

    Timer timer;

    Latex latex;

    latex.setFonts("./fonts/OpenSans-Regular.ttf", "", "");

    std::string equation = argv[1];
    std::string filepath = argv[2];

    for (int i = 0; i < equation.length(); i++) {
        std::cout << argv[1][i] << " - " << (int)equation[i] << "\n";
    }

    if (filepath.length() > 0)
        if (filepath.find(".png") != std::string::npos)
            latex.toPng(equation, filepath);
        else if (filepath.find(".jpg") != std::string::npos)
            latex.toJpg(equation, filepath);
        else if (filepath.find(".jpeg") != std::string::npos)
            latex.toJpg(equation, filepath);

};