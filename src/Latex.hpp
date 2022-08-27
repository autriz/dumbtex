#pragma once

#include "Image.hpp"

#include <stdexcept>
#include <iostream>

#if __cplusplus >= 202002L
#include <source_location>
#endif

class Latex {

    public:

        /*
            @brief The types of warning behavior.

            @details Use WarningBehavior::Strict to escalate any warnings
            generated during image conversion as exceptions, i.e.
            for them to be thrown as a Latex::ConversionException.
        */
        enum class WarningBehavior {Strict, Ignore, Log};

        /*
            @brief An exception thrown by the LaTeX parsing mechanism. 
        */
        struct ParseException : public std::runtime_error
        {
            ParseException(const std::string& message)
            : std::runtime_error(message)
            { }

            ParseException(const std::string& message, const char* file, unsigned int line)
            : std::runtime_error(message + " at " + file + ":" + std::to_string(line))
            { }

            #if __cplusplus >= 202002L
            
            ParseException(const std::string& message, const std::source_location location = std::source_location::current())
            : std::runtime_error(message + " at " + location.file_name() + ":" + std::to_string(location.line()))
            { }

            #endif
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
            : std::runtime_error(message + " at " + file + ":" + std::to_string(line))
            { }

            #if __cplusplus >= 202002L
            
            ConversionException(const std::string& message, const std::source_location location = std::source_location::current())
            : std::runtime_error(message + " at " + location.file_name() + ":" + std::to_string(location.line()))
            { }

            #endif
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
            : std::runtime_error(message + " at " + file + ":" + std::to_string(line))
            { }

            #if __cplusplus >= 202002L
            
            FileException(const std::string& message, const std::source_location location = std::source_location::current())
            : std::runtime_error(message + " at " + location.file_name() + ":" + std::to_string(location.line()))
            { }

            #endif
        };

        /*
        	@brief Constructs a Latex instance.
            @param behaviour The warning behavior to use.
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
        void toPNG(std::string& expression,  const std::string& filepath);

        /*
            @brief Renders JPG image and saves it
            @param expression Math expression
            @param filepath Path for image
        */
        void toJPG(std::string& expression,  const std::string& filepath);

        /*
            @brief Renders BMP image and saves it
            @param expression Math expression
            @param filepath Path for image
        */
        void toBMP(std::string& expression,  const std::string& filepath);

        /*
            @brief Renders TGA image and saves it
            @param expression Math expression
            @param filepath Path for image
        */
        void toTGA(std::string& expression,  const std::string& filepath);

        void setNormalFont(const std::string& path_to_font);

        void setItalicFont(const std::string& path_to_font);

        void setBoldFont(const std::string& path_to_font);

        void setFonts(const std::string& normal, const std::string& italic, const std::string& bold);

        /*
            @brief Set font color
            @param r,g,b,a RGBA parameters (0-255)
        */
        void setFontColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

        /*
            @brief Set font color
            @param color RGBA parameters
        */
        void setFontColor(const Color& color);
        
        /*
            @brief Returns the currently in-place warning behavior.
            @returns The warning behavior, as a member of the WarningBehavior enum.
        */
        const WarningBehavior& warningBehavior();

        /*
            @brief Sets the warning behavior.
    	    @param behavior The new warning behavior, a member of
    		   	the WarningBehavior enum.
        */
        void warningBehavior(WarningBehavior behavior);

        void operator=(const Latex& other) = delete;

    protected:

        /* The current WarningBehavior configuration. */
	    Latex::WarningBehavior p_warning_behavior;

        /* Standard font color */
        Color p_color {255, 255, 255, 255};

        std::string p_normal_font;
        std::string p_italic_font;
        std::string p_bold_font;

    private:

        /*
            @brief Renders image and saves it
            @param expression Math expression
            @param filepath Path for image 
            @param format File format
        */
        void toImage(std::string& expression,  const std::string& filepath, ImageType format);

        /*
            @brief WIP. Preprocesses math expression
            @brief *Change later from copy of expression to expression itself*
            @param expression Math expression
            @returns void
        */
        void prepExpression(std::string expression);

        Image texScripts(std::string expression, int at);

        /*
            @brief Searches and gets index of subfunction from the list
            @param expression Expression string
            @param at An index of starting point (always starts from backslash)
            @returns A positive integer, representing index, or negative (-1), if subfunction was not found
        */
        size_t getSubfunction(const std::string& expression, int at);

};

/* 
    @brief Converts hexadecimal (i.e 0xFFFFFF) to RGBA
    @brief **Added to handle hexadecimal color changer**
    @param hex hexadecimal number
    @param alpha alpha channel
    @returns Color struct containing RGBA values
*/
Color hexToRGBA(const int& hex, uint8_t alpha = 255);

/* 
    @brief Converts hexadecimal (i.e "FFFFFF") to RGBA
    @brief **Added to handle hexadecimal color changer**
    @param hex hexadecimal string
    @param alpha alpha channel
    @returns Color struct containing RGBA values
*/
Color hexToRGBA(const std::string& hex, uint8_t alpha = 255);

/* Name is subject to change */
struct Subfunction
{
    const char* expression;
    std::function<Image(Latex&, std::string&)> handler;
    uint8_t argc;
};

/* Subfunction handlers */
namespace Handlers
{
    /**/
    Image test_color(Latex&, std::string&);

    /*Rasterizes left-hand expression on top of right-hand expression*/
    Image test_newline(Latex&, std::string&);
}

static const Subfunction subfunctions[] = {
    {"\\color", Handlers::test_color, 2},
    {"\\frac", nullptr, 0},
    {"\\sum", nullptr, 0},
    {"\\prod", nullptr, 0},
    {"\\", Handlers::test_newline, 0},
    {NULL, nullptr, 0}
};