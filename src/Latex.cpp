#include "Latex.hpp"

Latex::Latex(WarningBehavior behavior): p_warning_behavior(behavior) { };

Latex::~Latex() { };

void Latex::toImage(std::string& expression,  const std::string& filepath, ImageType format)
{
    PROFILE_SCOPE("Latex::toImage");

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

        prepExpression() should split expression string to parts with function handlers (if needed one)
        then all those images should be rasterized and concatenated

        and it should be done for a project

   */

    // this->prepExpression(expression); //prepare expression to find unsupported subfunctions and remove unnecessary braces, if found

    //prepExpression() method returns void, but for it's lifespan splits and rasterizes expression, 
    //in the end saving somewhere array of images, that should be concatenated in toImage() method

    //maybe it could return array itself
    
    // std::cout << expression << "\n";

    Image image;

    Font font(p_normal_font.c_str(), 50);

    int i, j;
    size_t subfunc_index;

    while (expression.length() > 0)
    {
        for (i = 0; i < expression.length(); i++)
        {
            if (expression[i] == '\\')
            {
                if ((subfunc_index = getSubfunction(expression, i)) != -1) {
                    if (subfunctions[subfunc_index].handler) subfunctions[subfunc_index].handler(*this, expression); //find arguments for handler and pass it
                    //should return image, store it in some array for later concatenation
                    //remove rasterized part of expression  
                    // expression.erase(i,)
                }
                else 
                {
                    for (j = i; j < expression.length(); j++)
                    {
                        if (expression[j] == '{' || isdigit(expression[j]))
                            expression.erase(i, j - i); //or enclose it in [] and add '?' at the end of function
                    }
                    j = 0;
                }
                break;
            }
            else
            {
                Image temp;

                temp.rasterizeCharacter(expression[i], font, p_color);
                image.concat(temp);
                expression.erase(i, 1);
                // std::cout << expression << "\n";
                break;
            }
        }
        i = 0;
    }

    image.write(filepath.c_str(), format);

    return;

    //TODO: end

    image.rasterizeText(expression, font, p_color);

    image.write(filepath.c_str(), format);

};

void Latex::prepExpression(std::string expression)
{
    PROFILE_FUNCTION();

    //!Change void to std::vector<Image>, when all will be ready for that
    //!Or, prepExpression() can be merged with toImage()
    size_t subfunc_index;

    if (expression.length() == 0)
        return;

    #if 0
    for (size_t i = 0; i < expression.length(); i++)
    {
        if (expression[i] == '\\')
        {
            #if DEBUG
                std::cout << "found subexpression/function" << "\n";
            #endif
            // for (size_t u = 0; subfunctions[u].expression != NULL; u++)
            //     if ((pos = expression.find(subfunctions[u].expression, i)) != std::string::npos)
            //     {
            //         std::cout << "u: " << u << ", i: " << i << "\n";
            //         std::cout << "found \"" << subfunctions[u].expression << "\" at " << pos << "\n";
            //         break;
            //     };
            if ((subfunc_index = getSubfunction(expression, i)) != -1) //get matched subfunction index
            {
                #if DEBUG
                    std::cout << subfunc_index << "\n";
                    std::cout << "found match \"" << subfunctions[subfunc_index].expression << "\" at " << i << "\n";
                #endif
                //?do something with this

                //!FIND BRACES FOR ENOUGH ARGUMENTS AND DROP THAT AS ARGUMENT TO HANDLER
                //!like: {...}{...}.... for 2-count argument handler, function should get only {...}{...} and leave .... as-is

                //?implement argument finder
                //?also, think of the way how to throw them in handler function
                //?maybe, it's fitting to use std::optional
                //?if std::optional is used, try to find minimum maximum amount of args handlers can require

                if (subfunctions[subfunc_index].handler != NULL) 
                    subfunctions[subfunc_index].handler(*this, expression); //use existing handler for subfunction
                else 
                    std::cout << "handler not found" << "\n";

                //*if string (function) does eventually get erased,
                //*check if string had open and closing brackets and erase them too
                //*expression.erase(i, strlen(subfunctions[subfunc_index].expression)); // remove subfunction from expression
            }
            else
            {
                std::cout << "not found matching subfunction at " << i << "\n";
                for (size_t j = i; j < expression.length(); j++) //if not matched, iterate through string
                {
                    if (expression[j] == '{' || isdigit(expression[j])) //stop if bracket or number is found
                    {
                        expression.erase(i, j - i); //remove what before bracket/number
                        break;
                    };
                };
                //remove unmatched subfunction, loop through string, stop if bracket or number is found, remove what before that
            };
            //if ()
        };
        // std::cout << expression[i] << "\n";
    };

    std::cout << "prepared expression: " << expression << "\n";

    return;

    #endif

    int i, j;

    while (expression.length() > 0)
    {
        for (i = 0; i < expression.length(); i++)
        {
            if (expression[i] == '\\')
            {
                if ((subfunc_index = getSubfunction(expression, i)) != -1) {
                    if (subfunctions[subfunc_index].handler) subfunctions[subfunc_index].handler(*this, expression); //find arguments for handler and pass it
                    //should return image, store it in some array for later concatenation
                    //remove rasterized part of expression  
                    // expression.erase(i,)
                }
                else 
                {
                    for (j = i; j < expression.length(); j++)
                    {
                        if (expression[j] == '{' || isdigit(expression[j]))
                            expression.erase(i, j - i); //or enclose it in [] and add '?' at the end of function
                    }
                    j = 0;
                }
            }
            else
            {
                // Image image;

                // image.rasterizeText(expression.c_str()[i], font, p_color)
                expression.erase(i, 1);
                std::cout << expression << "\n";
            }
        }
        i = 0;
    }
};

Image Latex::texScripts(std::string expression, int at)
{

};

size_t Latex::getSubfunction(const std::string& expression, int at)
{
    for (size_t i = 0; subfunctions[i].expression != NULL; i++) { //iterate through subfunction list
        std::cout << subfunctions[i].expression << "\n";
        size_t match = expression.find(subfunctions[i].expression, at); //get match
        if (match != std::string::npos && match == at) //if matches
        {
            return i;
        };
    };
    return -1; //if not
}; 

void Latex::toPNG(std::string& expression,  const std::string& filepath) 
{ 
    this->toImage(expression, filepath, ImageType::PNG); 
};

void Latex::toJPG(std::string& expression,  const std::string& filepath) 
{ 
    this->toImage(expression, filepath, ImageType::JPG); 
};

void Latex::toBMP(std::string& expression,  const std::string& filepath)
{
    this->toImage(expression, filepath, ImageType::BMP);
};

void Latex::toTGA(std::string& expression,  const std::string& filepath)
{
    this->toImage(expression, filepath, ImageType::TGA);
};

void Latex::setNormalFont(const std::string& path_to_font) 
{ 
    this->p_normal_font = path_to_font; 
};

void Latex::setItalicFont(const std::string& path_to_font) 
{ 
    this->p_italic_font = path_to_font; 
};

void Latex::setBoldFont(const std::string& path_to_font)
{
    this->p_bold_font = path_to_font;
}

void Latex::setFonts(const std::string& normal, const std::string& italic, const std::string& bold) 
{ 
    this->setNormalFont(normal); 
    this->setItalicFont(italic); 
    this->setBoldFont(bold);
};

void Latex::setFontColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    this->p_color = {r, g, b, a};
};

void Latex::setFontColor(const Color& color)
{
    this->p_color = color;
};

void Latex::warningBehavior(WarningBehavior behaviour) 
{ 
    this->p_warning_behavior = behaviour; 
};

const Latex::WarningBehavior& Latex::warningBehavior()
{ 
    return this->p_warning_behavior; 
};

Color hexToRGBA(const std::string& hex, uint8_t alpha)
{
    return hexToRGBA(std::stoi(hex, 0, 16), alpha);
};

Color hexToRGBA(const int& hex, uint8_t alpha)
{
    Color RGBA {0, 0, 0, alpha};

    RGBA.r = (uint8_t)((hex >> 16) & 0xFF);
    RGBA.g = (uint8_t)((hex >> 8) & 0xFF);
    RGBA.b = (uint8_t)((hex) & 0xFF);

    return RGBA;
};

Image Handlers::test_newline(Latex& latex, std::string& expression)
{

};

Image Handlers::test_color(Latex& latex, std::string& expression)
{

    //Font font;
    //Image image;
    //Color rgba = hexToRGBA(args[0]); //? find hex argument in expression string
    //image.rasterizeText(args[1], font, rgba); //? find text argument in expression string, and find font to use,

    //? return image;

    std::cout << "test_color" << "\n";
};