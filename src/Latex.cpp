#include "Latex.hpp"

Latex::Latex(WarningBehavior behavior): p_warning_behavior(behavior) { };

Latex::~Latex() { };

void Latex::toImage( const std::string& expression,  const std::string& filepath, ImageFormat format) 
{
    /*

        Preprocess and rasterization

        Iteration processing symbol or function (like "\sum")

        If iteration is processing function, that iteration is split up by atoms
    
    */

   /*
   
        TODO:
        Render text onto image and save in file
        Allow to override all file if current exists

        if "\alpha" is present in string, render alpha symbol

   */

    prepExpression(expression);

    return;

    Image image;

    if (expression.find_first_of("\\") != std::string::npos) 
    {
        std::cout << "found subcommand" << "\n";

        //TODO: um...
    }

    Font font(p_normal_font.c_str(), 50);

    //TODO: end

    image.rasterizeText(expression, font, 255, 255, 0, 255);

    // std::cout << "Start\n";
    // image.rasterizeText(expression, font, 255, 255, 0, 255);
    // std::cout << "End\n";

    // Resolution res = image.getResolution();
    // std::cout << res.width << "\n";

    // printf("result:\nwidth: %ld, height: %ld, channels: %ld, size: %ld\n", result.width, result.height, result.channels, result.size);

    // result.write(filepath.c_str());

    image.write(filepath.c_str());
};

void Latex::prepExpression(const std::string& expression)
{
    if (expression.length() <= 0)
        goto end_of_job;

    while (1)
    {
        goto end_of_job;
    }

    std::cout << expression << "\n";

    end_of_job:
        return;
}

void Latex::toPng( const std::string& expression,  const std::string& filepath) 
{ 
    this->toImage(expression, filepath, ImageFormat::PNG); 
};

void Latex::toJpg( const std::string& expression,  const std::string& filepath) 
{ 
    this->toImage(expression, filepath, ImageFormat::JPG); 
};

void Latex::setNormalFont(const char* path_to_font) 
{ 
    this->p_normal_font = path_to_font; 
};

void Latex::setItalicFont(const char* path_to_font) 
{ 
    this->p_italic_font = path_to_font; 
};

void Latex::setBoldFont(const char* path_to_font)
{
    this->p_bold_font = path_to_font;
}

void Latex::setFonts(const char* normal, const char* italic, const char* bold) 
{ 
    setNormalFont(normal); 
    setItalicFont(italic); 
    setBoldFont(bold);
};

void Latex::warningBehavior(WarningBehavior behaviour) 
{ 
    p_warning_behavior = behaviour; 
};

const Latex::WarningBehavior& Latex::warningBehavior() const 
{ 
    return p_warning_behavior; 
};