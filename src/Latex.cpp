#include "Latex.hpp"

Latex::Latex(WarningBehavior behavior): p_WarningBehavior(behavior) { };

Latex::~Latex() { };

/*Name is subject to change. Finds not closed bracket pair and closes it*/
void testt(std::string& expr)
{
    std::stack<char> brackets;

    for (int i = 0; i < expr.length(); i++)
    {
        if (expr[i] == '{')
            brackets.push(expr[i]);
        if (expr[i] == '}')
            if (brackets.size() > 0)
                brackets.pop();
            else
                expr.erase(i, 1);
        
    }

    if (!brackets.empty())
        expr.append("}");
};

Image Latex::toImage(std::string& expression)
{
    PROFILE_SCOPE("Latex::toImage");

    /*

        Preprocess and rasterization

        Iteration processing symbol or function (like "\sum")

        If iteration is processing function, that iteration is split up by atoms
         
    */

   /*
        toImage()
        |__rastlimits()
           |__rastscripts()
              |__texscripts()
   
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

    Image finalImage;

    testt(expression);

    // Font font(p_NormalFont, 50); //Should be initialized when rasterization is needed

    int i, j;
    SubFunction subFunction;

    prepExpression(expression); //prepare expression to find unsupported subfunctions and remove unnecessary braces, if found

    #ifdef DEBUG
        printf("[Latex::toImage] start expression: %s\n", expression.c_str());
    #endif

    while (expression.length() > 0)
    {
        for (i = 0; i < expression.length(); i++)
        {
            //if expression[i] == '{' and expression[i-1] != '\\' then
            //find '}', check if '}' not escaped
            //if '}' not found, add it to the end of expression
            //cut expression between '{' and '}'
            //remove '{' and '}'
            //parse it to toImage() again
            //concat with finalImage
            if (expression[i] == '{' && expression[i-1] != '\\')
            {
                size_t rightBracketIndex;
                std::string subexpression;

                //?Find all open and close brackets (not escaped)
                //?If open bracket does not have a close pair, add it

                if ((rightBracketIndex = expression.find_last_of('}', i)) == std::string::npos || (expression[rightBracketIndex - 1] == '\\'))
                {
                    expression.push_back('}');
                    rightBracketIndex = expression.length() - 1;
                }

                subexpression = expression.substr(i + 1, rightBracketIndex - i - 2);
                expression.erase(i, subexpression.length() + 3);

                if (expression.length() <= 0)
                {
                    expression = subexpression;

                    break;
                }
                else
                {
                    Image tempImage = toImage(subexpression);
                    finalImage.concat(tempImage);
                }
            }
            else if (expression[i] == '\\')
            {
                if ((subFunction = getSubFunction(expression, i)).expression != NULL)
                {

                    //?Esaped delimeters like "\}" or "\{" are processed after everything between them is rasterized
                    //?Because escaped delims depend on size of all expression

                    //!Take inspiration from MimeTeX's texsubexpr() function, quite a gold mine

                    // std::cout << getSubExpression(expression, i + strlen(subfunctions[subfunc_index].expression), "{", "}", true) << "\n";

                    expression.erase(i, strlen(subFunction.expression)); 
                    if (subFunction.handler) 
                        subFunction.handler(*this, expression, finalImage, subFunction.type);

                    // if (subfunctions[subfunc_index].handler) subfunctions[subfunc_index].handler(*this, expression); //find arguments for handler and pass it
                    //should return image for further concatenation with final image
                    //remove rasterized part of expression  
                    // expression.erase(i,)
                }
                else 
                {
                    for (j = i; j < expression.length(); j++)
                    {
                        if (expression[j] == '{' || isdigit(expression[j]) || expression[j] == '~')
                            expression.erase(i, j - i); //or enclose it in [] and add '?' at the end of function
                    }

                    j = 0;
                }
                break;
            }
            else
            {
                // Image tempImage;

                // tempImage.rasterizeCharacter(expression[i], m_NormalFont, p_Color);
                // finalImage.concat(tempImage);
                // expression.erase(i, 1);

                finalImage.rasterizeCharacter(expression[i], m_NormalFont, p_Color);
                expression.erase(i, 1);
                break;
            }
        }

        i = 0;

        #ifdef DEBUG
            printf("[Latex::toImage] expression: %s\n", expression.c_str());
        #endif
    }

    return finalImage.isEmpty() ? NULL : finalImage;

};

void Latex::prepExpression(std::string& expression)
{
    //!Erase all spaces

    if (expression.length() == 0)
        return;

    const char* comment = "%%";

    size_t startIndex, endIndex;

    //Find and erase comments
    while ((startIndex = expression.find_first_of(comment)) != std::string::npos) //found start of the comment
    {
        if ((endIndex = expression.find_first_of(comment, startIndex + 1)) != std::string::npos) //if end is found
        {
            expression.erase(startIndex, endIndex - startIndex + 1); //erase comment
            break;
        }
        else //if end not found
        {
            expression.erase(startIndex); //erase all expression
            break;
        }
    }

    //Fix unbalanced brackets
    // for (int i = 0; i < expression.length(); i++)
    // {
        //find {
        //place it in stack or smth
        //if } is found, remove one { from stack
        //if something is left in stack, brackets are unbalanced
        //then get index where we stopped and add there closing bracket

        //OR

    // }

    //Convert \\left( to \\( and \\right) to \\)
    // while ( )
};

Image Latex::texScripts(std::string expression, int at)
{

};

std::string Latex::getSubExpression(std::string& expression, int from, const char* left, const char* right, bool returnDelims)
{
    PROFILE_SCOPE("Latex::getSubExpression");

    unsigned first = expression.find(left, from); //find index of left delimeter
    if (first > from) return NULL;

    unsigned last = expression.find(right, from); //find index of right delimeter

    while (expression[last - 1] == '\\') //if delimeter is escaped
        last = expression.find(right, last+1); //find right delimeter again

    return returnDelims ? expression.substr(first, last - first + 1) : expression.substr(first + 1, last - first - 1);
};

SubFunction Latex::getSubFunction(const std::string& expression, int at)
{
    PROFILE_SCOPE("Latex::getSubFunction");

    size_t i, match;

    for (i = 0; subfunctions[i].expression != NULL; i++) //iterate through subfunction list
    {
        match = expression.find(subfunctions[i].expression, at); //get match

        if (match != std::string::npos && match == at) //if matches
            return subfunctions[i];
    };
    return subfunctions[i]; //if not, should return NULL defined subfunction (dummy)
}; 

void Latex::toPNG(std::string& expression,  const char* filepath) 
{ 
    Image image = toImage(expression);

    if (!image.isEmpty())
        image.write(filepath, ImageType::PNG);
    else
        throw Latex::ParseException("There's nothing to rasterize.", __FILE__, __LINE__);
};

void Latex::toJPG(std::string& expression,  const char* filepath) 
{ 
    Image image = toImage(expression);

    if (!image.isEmpty())
        image.write(filepath, ImageType::JPG);
    else
        throw Latex::ParseException("There's nothing to rasterize.", __FILE__, __LINE__); 
};

void Latex::toBMP(std::string& expression,  const char* filepath)
{
    Image image = toImage(expression);

    if (!image.isEmpty())
        image.write(filepath, ImageType::BMP);
    else
        throw Latex::ParseException("There's nothing to rasterize.", __FILE__, __LINE__); 
};

void Latex::toTGA(std::string& expression,  const char* filepath)
{
    Image image = toImage(expression);

    if (!image.isEmpty())
        image.write(filepath, ImageType::TGA);
    else
        throw Latex::ParseException("There's nothing to rasterize.", __FILE__, __LINE__); 
};

void Latex::setNormalFont(const char* path_to_font, uint16_t size) 
{ 
    m_NormalFont.setFont(path_to_font);
    m_NormalFont.setSize(size);
};

void Latex::setItalicFont(const char* path_to_font, uint16_t size) 
{ 
    m_ItalicFont.setFont(path_to_font);
    m_ItalicFont.setSize(size); 
};

void Latex::setBoldFont(const char* path_to_font, uint16_t size)
{
    m_BoldFont.setFont(path_to_font);
    m_BoldFont.setSize(size);
};

void Latex::setBoldItalicFont(const char* path_to_font, uint16_t size)
{
    m_BoldItalicFont.setFont(path_to_font);
    m_BoldItalicFont.setSize(size);
};

void Latex::setFonts(const char* normal, const char* italic, const char* bold, const char* boldItalic) 
{ 
    PROFILE_SCOPE("Latex::setFonts");

    if (strlen(normal) > 0) this->setNormalFont(normal); 
    if (strlen(italic) > 0) this->setItalicFont(italic); 
    if (strlen(bold) > 0) this->setBoldFont(bold);
    if (strlen(boldItalic) > 0) this->setBoldItalicFont(boldItalic);
};

void Latex::setFontColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    this->p_Color = {r, g, b, a};
};

void Latex::setFontColor(const Color& color)
{
    this->p_Color = color;
};

const Color& Latex::getFontColor()
{
    return this->p_Color;
}

void Latex::warningBehavior(WarningBehavior behaviour) 
{ 
    this->p_WarningBehavior = behaviour; 
};

const Latex::WarningBehavior& Latex::warningBehavior()
{ 
    return this->p_WarningBehavior; 
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

void Handlers::test_newline(Latex& latex, std::string& expression, Image& image, SubFunctionType arg1)
{
    image.concat(latex.toImage(expression), ImagePosition::BOTTOM);
};

void Handlers::test_color(Latex& latex, std::string& expression, Image& image, SubFunctionType colorType)
{

    Image tempImage;
    std::string hex, arg;
    //?if called, should set color temporarily or permanently, if 1st char is "~"
    //!create temporary color handler

    switch(colorType)
    {
        case COLOR_CUSTOM:
            hex = Latex::getSubExpression(expression, 0, "{", "}", false);
            if (hex.length() == 0) return;

            expression.erase(0, hex.length() + 2);
            break;
        case COLOR_RED:
            hex = "ff0000";
            break;
    }
    
    if (expression[0] == '~') //color all text after subfunction 
    {
        arg = expression.substr(1);
        expression.erase(0);
        if (arg.length() == 0) return;

        latex.setFontColor(hexToRGBA(hex));

        tempImage = latex.toImage(arg);
        if (!tempImage.isEmpty()) image.concat(tempImage);
    }
    else //color only text in brackets
    {
        arg = Latex::getSubExpression(expression, 0, "{", "}", false);
        if (arg.length() == 0) return;

        Color tempColor = latex.getFontColor();
        expression.erase(0, arg.length() + 2);

        latex.setFontColor(hexToRGBA(hex));

        tempImage = latex.toImage(arg);
        if (!tempImage.isEmpty()) image.concat(tempImage);

        latex.setFontColor(tempColor);
    }
};