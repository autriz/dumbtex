#pragma once

#include "Image.hpp"

#include <stdexcept>
#include <stack>
#include <iostream>

class Latex {

    public:

        /*
            @brief The types of warning behavior.
            @details Use WarningBehavior::Strict to escalate any warnings
            generated during image conversion as exceptions, i.e.
            for them to be thrown as a Latex::ConversionException.
        */
        enum class WarningBehavior {Strict, Ignore, Log};

        //?Add warnings, logs, errors to code
        //?Or remove it

        /*
            @brief An exception thrown by the LaTeX parsing mechanism. 
        */
        struct ParseException : public std::runtime_error
        {
            
            ParseException(const std::string& message)
            : std::runtime_error(message)
            { }

            ParseException(const std::string& message, const char* file, unsigned int line)
            : std::runtime_error("\"" + message + "\" at " + file + ":" + std::to_string(line))
            { }

        };
        

        /*	
            @brief An exception thrown during the LaTeX-to-image conversion. 
        */
        struct ConversionException : public std::runtime_error
        {
            
            ConversionException(const std::string& message)
            : std::runtime_error(message)
            { }

            ConversionException(const std::string& message, const char* file, unsigned int line)
            : std::runtime_error("\"" + message + "\" at " + file + ":" + std::to_string(line))
            { }
        };
        
        /* 
            @brief An exception thrown during file-manipulation. 
        */
        struct FileException : public std::runtime_error
        {
            
            FileException(const std::string& message)
            : std::runtime_error(message)
            { }

            FileException(const std::string& message, const char* file, unsigned int line)
            : std::runtime_error("\"" + message + "\" at " + file + ":" + std::to_string(line))
            { }

        };

        /*
        	@brief Constructs a Latex instance.
            @param behavior The warning behavior to use.
        	@see WarningBehavior
        */
        Latex(WarningBehavior behavior = WarningBehavior::Log);

        Latex(const Latex& other) = delete;

        Latex(Latex& other) = delete;

        Latex(Latex&& other) = delete;

        ~Latex();

        /*
            @brief Renders PNG image and saves it
            @param expression Math expression
            @param filepath Path for image
        */
        void toPNG(std::string& expression, const char* filepath);

        /*
            @brief Renders JPG image and saves it
            @param expression Math expression
            @param filepath Path for image
        */
        void toJPG(std::string& expression, const char* filepath);

        /*
            @brief Renders BMP image and saves it
            @param expression Math expression
            @param filepath Path for image
        */
        void toBMP(std::string& expression, const char* filepath);

        /*
            @brief Renders TGA image and saves it
            @param expression Math expression
            @param filepath Path for image
        */
        void toTGA(std::string& expression, const char* filepath);

        /*
            @brief Renders image and returns it. Do not use this method outside class or handlers, use toPNG() or other alternatives instead
            @param expression Math expression
            @returns Rasterized image or NULL if size of image is 0
        */
        Image toImage(std::string& expression);

        void setNormalFont(const char* path_to_font, uint16_t size = 50);

        void setItalicFont(const char* path_to_font, uint16_t size = 50);

        void setBoldFont(const char* path_to_font, uint16_t size = 50);

        void setBoldItalicFont(const char* path_to_font, uint16_t size = 50);

        /*
            @brief Sets font files (normal, italic and bold)
            @
        */
        void setFonts(const char* normal, const char* italic, const char* bold, const char* boldItalic);

        /*
            @brief Set font color
            @param r,g,b,a RGBA values (0-255)
        */
        void setFontColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

        /*
            @brief Set font color
            @param color Color struct, containing RGBA values (0-255)
        */
        void setFontColor(const Color& color);

        const Color& getFontColor();

        /*
            @brief Scans expression for subexpression between delimeters
            @param expression Math expression
            @param from Index of left delimeter
            @param left Delimeter that begin subexpression
            @param right Delimeter that end subexpression
            @param returnDelims If true, subexpression returns with left and right delimeters
            @returns String, containing everything between left and right braces, or NULL if none found
        */
        static std::string getSubExpression(std::string& expression, int from, const char* left, const char* right, bool returnDelims);
        
        /*
            @brief Searches and gets index of subfunction from the list
            @param expression Expression string
            @param at An index of starting point (always starts from backslash)
            @returns A positive integer, representing index, or negative (-1), if subfunction was not found
        */
        static struct SubFunction getSubFunction(const std::string& expression, int at);

        /*
            @brief Returns the currently in-place warning behavior.
            @returns The warning behavior, a member of the WarningBehavior enum.
        */
        const WarningBehavior& warningBehavior();

        /*
            @brief Sets the warning behavior.
    	    @param behavior The new warning behavior, a member of
    		   	the WarningBehavior enum.
        */
        void warningBehavior(WarningBehavior behavior);

        void operator=(const Latex& other) = delete;

        /*
            @brief WIP. Preprocesses math expression. 
            @brief Removes comments, converts \\left( to \\( and \\right) to \\)
            @brief *Change later from copy of expression to expression itself*
            @param expression Math expression
            @returns void
        */
        void prepExpression(std::string& expression);

        /*
            @brief WIP
            @returns Image
        */
        Image texScripts(std::string expression, int at);

    public:

        Font m_NormalFont;
        Font m_ItalicFont;
        Font m_BoldFont;
        Font m_BoldItalicFont;

    protected:

        /* The current WarningBehavior configuration. */
	    Latex::WarningBehavior p_WarningBehavior;

        /* Standard font color */
        Color p_Color {255, 255, 255, 255};

};

/* 
    @brief Converts hexadecimal (i.e 0xFFFFFF) to RGBA
    @brief **Added to handle hexadecimal color changer**
    @param hex Hexadecimal number
    @param alpha Alpha channel
    @returns Color struct containing RGBA values
*/
Color hexToRGBA(const int& hex, uint8_t alpha = 255);

/* 
    @brief Converts hexadecimal (i.e "FFFFFF") to RGBA
    @brief **Added to handle hexadecimal color changer**
    @param hex Hexadecimal string
    @param alpha Alpha channel
    @returns Color struct containing RGBA values
*/
Color hexToRGBA(const std::string& hex, uint8_t alpha = 255);

/* Subject to change */
enum SubFunctionType
{
    NONE = -1,
    /* Coloring types */
    COLOR_CUSTOM,
    COLOR_RED,

};

/* Name is subject to change */
struct SubFunction
{
    const char* expression;
    std::function<void(Latex&, std::string&, Image&, SubFunctionType)> handler;
    SubFunctionType type; //WIP
};

/* Subfunction handlers */
namespace Handlers
{
        /**/
        void test_color(Latex&, std::string&, Image&, SubFunctionType);

        /*Rasterizes left-hand expression on top of right-hand expression*/
        void test_newline(Latex&, std::string&, Image&, SubFunctionType);
};

static const SubFunction subfunctions[] = {
    {"\\color", Handlers::test_color, COLOR_CUSTOM},
    {"\\red", Handlers::test_color, COLOR_RED},
    {"\\frac", nullptr, NONE},
    {"\\sum", nullptr, NONE},
    {"\\prod", nullptr, NONE},
    {"\\\\", Handlers::test_newline, NONE},
    {"\\}", nullptr, NONE},
    {NULL, nullptr, NONE}
};