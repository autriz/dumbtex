#pragma once

#include "Image.hpp"

#include <stdexcept>
#include <cstring>
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

        enum class FontType {Normal, Italic, Bold, BoldItalic};

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

        /*
            @brief Set font file
            @param type Font type
            @param pathToFont
            @param size Font size
        */
        void setFont(FontType type, const char* pathToFont, uint16_t size = 50);

        Font& getSelectedFont();

        void setSelectedFont(FontType type);

        /*
            @brief Sets font files (normal, italic, bold and bold italic)
            @param normal Path to regular font file
            @param italic Path to italic font file
            @param bold Path to bold font file
            @param boldItalic Path to bold italic font file
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
        static std::string getSubExpression(std::string& expression, int from, const char left, const char right, bool returnDelims);
        
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

        void rastLimits(std::string& expression, Image& image);

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

        Font* p_SelectedFont;

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
    COLOR_GRADIENT,
    COLOR_RED,
    COLOR_YELLOW,
    /* Text types */
    TEXT_CYR,
    TEXT_GREEK,
    /* Font types */
    FONT_REGULAR,
    FONT_ITALIC,
    FONT_BOLD,
    FONT_BOLDITALIC

};

/* Name is subject to change */
struct SubFunction
{
    const char* expression;
    std::function<void(Latex&, std::string&, Image&, SubFunctionType)> handler;
    SubFunctionType type; //WIP
};

struct Letter
{
	const char* character;
	uint16_t charCode;
};

/* Subfunction handlers */
struct Handlers
{
        /*
            @brief Rasterizes expression (or subexpression) with defined color in HEX format
            @example "\color{FF00FF}{hello}" rasterizes only text between brackets
            @example "\color{00FFFF}~hello" rasterizes everything after tilda
            @example "\gradient{FF0000}{00FF00}{hello}" rasterizes text between brackets and colors it with linear gradient
        */
        static void rastColor(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief Rasterizes left-hand expression on top of right-hand expression
            @example "abc\\def"
        */
        static void rastNewline(Latex&, std::string&, Image&, SubFunctionType);

		/*
            @brief Rasterizes expression and concatenates with some y offset (positive or negative)
            @example "\raisebox{50}{hel}lo" lifts "hel" part up by 50 pixels
            @example "\raisebox{-10}{wo}rld" drops "wo" part down by 10 pixels
        */
		static void rastRaise(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief
        */
        static void rastSetFont(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief
        */
        static void rastText(Latex&, std::string&, Image&, SubFunctionType);
};

static Letter cyr_table[] = 
{
	{"A", 1040}, {"B", 1041}, {"V", 1042}, {"D", 1043},
	{"IE", 1044}, {"YO", 1025}, {"ZH", 1046}, {"Z", 1047},
	{"I", 1048}, {"J", 1049}, {"K", 1050}, {"L", 1051},
	{"M", 1052}, {"N", 1053}, {"O", 1054}, {"P", 1055},
	{"R", 1056}, {"S", 1057}, {"T", 1058}, {"U", 1059},
	{"F", 1060}, {"KH", 1061}, {"TS", 1062}, {"CH", 1063},
	{"SH", 1064}, {"SHCH", 1065}, {"\\Cdprime", 1066}, {"\\Yeta", 1067},
	{"\\Cprime", 1068}, {"E", 1069}, {"YU", 1070}, {"YA", 1071},
	{"a", 1072}, {"b", 1073}, {"v", 1074}, {"g", 1075},
	{"d", 1076}, {"ie", 1077}, {"yo", 1105}, {"zh", 1078},
	{"z", 1079}, {"i", 1080}, {"j", 1081}, {"k", 1082},
	{"l", 1083}, {"m", 1084}, {"n", 1085}, {"o", 1086},
	{"p", 1087}, {"r", 1088}, {"s", 1089}, {"t", 1090},
	{"u", 1091}, {"f", 1092}, {"kh", 1093}, {"ts", 1094},
	{"ch", 1095}, {"sh", 1096}, {"shch", 1097}, {"\\cdprime", 1098},
	{"\\yeta", 1099}, {"\\cprime", 1100}, {"e", 1101}, {"yu", 1102},
	{"ya", 1103}
};

static Letter greek_table[] =
{
	{"Alpha", 913}, {"Beta", 914}, {"Gamma", 915}, {"Delta", 916},
	{"Epsilon", 917}, {"Zeta", 918}, {"Eta", 919}, {"Theta", 920},
	{"Iota", 921}, {"Kappa", 922}, {"Lambda", 923}, {"Mu", 924},
	{"Nu", 925}, {"Xi", 926}, {"Omicron", 927}, {"Pi", 928},
	{"Rho", 929}, {"Sigma", 930}, {"Tau", 931}, {"Upsilon", 932},
	{"Phi", 933}, {"Chi", 934}, {"Psi", 935}, {"Omega", 936},
	{"alpha", 945}, {"beta", 946}, {"gamma", 947}, {"delta", 948},
	{"epsilon", 949}, {"zeta", 950}, {"eta", 951}, {"theta", 952},
	{"iota", 953}, {"kappa", 954}, {"lambda", 955}, {"mu", 956},
	{"nu", 957}, {"xi", 958}, {"omicron", 959}, {"pi", 960},
	{"rho", 961}, {"sigma", 963}, {"tau", 964}, {"upsilon", 965},
	{"phi", 966}, {"chi", 967}, {"psi", 968}, {"omega", 969},
};

static const SubFunction subfunctions[] = {
    /* Color */
    {"\\color",    Handlers::rastColor,   COLOR_CUSTOM},
    {"\\red",      Handlers::rastColor,   COLOR_RED},
    {"\\yellow",   Handlers::rastColor,   COLOR_YELLOW},
    {"\\gradient", Handlers::rastColor,   COLOR_GRADIENT},
    /* Text */
    {"\\cyr",      Handlers::rastText,    TEXT_CYR},
    {"\\greek",    Handlers::rastText,    TEXT_GREEK},
    /* Font */
    {"\\it",       Handlers::rastSetFont, FONT_ITALIC},
    {"\\bold",     Handlers::rastSetFont, FONT_BOLD},
    {"\\boldit",   Handlers::rastSetFont, FONT_BOLDITALIC},
    /* Math */
    {"\\frac",     nullptr,               NONE},
    {"\\sum",      nullptr,               NONE},
    {"\\prod",     nullptr,               NONE},
    /* New line */
    {"\\\\",       Handlers::rastNewline, NONE},
	{"\\raisebox", Handlers::rastRaise,   NONE},
    {"\\}",        nullptr,               NONE},
    {NULL,         nullptr,               NONE}
};