#pragma once

#include "Image.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <filesystem>

class Latex {

    public:

        /*
            @brief The available image formats.
        */
        enum class ImageFormat { PNG, JPG };

        /*
            @brief The types of warning behavior.

            @details Use WarningBehavior::Strict to escalate any warnings
            generated during image conversion as exceptions, i.e.
            for them to be thrown as a Latex::ConversionException.
        */
        enum class WarningBehavior { Strict, Ignore, Log };

        class Logger;

        /*
            @brief An exception thrown by the LaTeX parsing mechanism. 
        */
        struct ParseException : public std::runtime_error
        {
            ParseException(const std::string& what)
            : std::runtime_error(what)
            { }
        };
        

        /*	
            @brief An exception thrown during the LaTeX-to-image conversion. 
        */
        struct ConversionException : public std::runtime_error
        {
            ConversionException(const std::string& what)
            : std::runtime_error(what)
            { }
        };
        
        /* 
            @brief An exception thrown during file-manipulation. 
        */
        struct FileException : public std::runtime_error
        {
            FileException(const std::string& what)
            : std::runtime_error(what)
            { }
        };

        /*
        	@brief Constructs a Latex instance.
            @param behaviour The warning behavior to use.
        	@see WarningBehavior
        */
        Latex(WarningBehavior behavior = WarningBehavior::Log);

        ~Latex();

        /*
            @brief Renders PNG image and saves it
            @param expression Math expression
            @param filepath Path for image
        */
        void toPng( const std::string& expression,  const std::string& filepath);

        /*
            @brief Renders JPG image and saves it
            @param expression Math expression
            @param filepath Path for image
        */
        void toJpg( const std::string& expression,  const std::string& filepath);

        void setNormalFont(const char* path_to_font);

        void setItalicFont(const char* path_to_font);

        void setBoldFont(const char* path_to_font);

        void setFonts(const char* normal, const char* italic, const char* bold);
        
        /*
            @brief Returns the currently in-place warning behavior.
            @returns The warning behavior, as a member of the WarningBehavior enum.
        */
        const WarningBehavior& warningBehavior() const;

        /*
            @brief Sets the warning behavior.
    	    @param behavior The new warning behavior, a member of
    		   			    the WarningBehavior enum.
        */
        void warningBehavior(WarningBehavior behavior);

    protected:

        /* The current WarningBehavior configuration. */
	    Latex::WarningBehavior p_warning_behavior;

        /* Standard font color */
        Color p_colors {255, 255, 255, 255};

        std::string p_normal_font;
        std::string p_italic_font;
        std::string p_bold_font;

    private:

        Latex(const Latex& other);

        /*
            @brief Renders image and saves it
            @param expression Math expression
            @param filepath Path for image 
            @param format File format
        */
        void toImage( const std::string& expression,  const std::string& filepath, ImageFormat format);

        /*
            @brief WIP. Preprocesses math expression
            @returns Vector of strings
        */
        void prepExpression(const std::string& expression);

        void operator=(const Latex& other);

};