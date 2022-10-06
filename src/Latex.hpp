#pragma once

#include "Image.hpp"

#include <stdexcept>
#include <cstring>
#include <stack>
#include <iostream>

#if !defined(DEBUG) || !defined(PROFILER)
#include <chrono>
#endif

struct Scripts;

enum ScriptType { SUBSCRIPT = 1, SUPSCRIPT, BOTH };

class Latex {

    public:

        /*
            @brief The types of warning behavior.
            @details Use WarningBehavior::Strict to escalate any warnings
            generated during image conversion as exceptions, i.e.
            for them to be thrown as a Latex::ConversionException.
        */
        enum class WarningBehavior { Strict, Ignore, Log };

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
            @returns String, containing everything between left and right delimeters, or next char if none found
        */
        static std::string getSubExpression(std::string& expression, unsigned from, const char left, const char right, bool returnDelims);
        
        /*
            @brief Searches and gets index of subfunction from the list
            @param expression Expression string
            @param at An index of starting point (always starts from backslash)
            @returns A positive integer, representing index, or negative (-1), if subfunction was not found
        */
        static struct SubFunction getSubFunction(const std::string& expression, size_t at);

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
            @brief Searches for subscript and/or superscript in expression
            @returns Scripts struct, containing subscript and supscript strings
        */
        Scripts texScripts(std::string& expression, ScriptType which);

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

struct Scripts
{
    std::string subScript;
    std::string supScript;
};

enum SubFunctionType
{
    NONE = -1,
    /* Coloring types */
    COLOR_CUSTOM,
    COLOR_GRADIENT,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_BLACK,
    COLOR_WHITE,
    /* Text types */
    TEXT_CYR,
    TEXT_GREEK,
    /* Font types */
    FONT_REGULAR,
    FONT_ITALIC,
    FONT_BOLD,
    FONT_BOLDITALIC,
    /* rastFrac types */
    FRAC_NORMAL,
    FRAC_OVER,
    FRAC_ATOP,
    FRAC_CHOOSE,
    /* rastArray types */
    ARR_NORMAL,
    ARR_MATRIX,
    ARR_TABULAR,
    /* rastMathFunc types */
    MATH_ARCCOS,
    MATH_ARCSIN,
    MATH_ARCTAN,
    MATH_ARG,
    MATH_SIN,
    MATH_SINH,
    MATH_COS,
    MATH_COSH,
    MATH_TAN,
    MATH_TANH,
    MATH_COT,
    MATH_COTH,
    MATH_CSC,
    MATH_DEG,
    MATH_DET,
    MATH_DIM,
    MATH_EXP,
    MATH_GCD,
    MATH_HOM,
    MATH_INF,
    MATH_KER,
    MATH_LG,
    MATH_LIM,
    MATH_LIMINF,
    MATH_LIMSUP,
    MATH_LN,
    MATH_LOG,
    MATH_MAX,
    MATH_MIN,
    MATH_SUM,
    MATH_PROD,
    MATH_PR,
    MATH_SEC
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
            @brief \color \red \yellow \gradient handler
            @details Rasterizes expression (or subexpression) with defined color in HEX format
            @example "\color{FF00FF}{hello}" rasterizes only text between brackets
            @example "\color{00FFFF}~hello" rasterizes everything after tilda
            @example "\gradient{FF0000}{00FF00}{hello}" rasterizes text between brackets and colors it with linear gradient
            Warning: \gradient will override all colors that are present in the subexpression
        */
        static void rastColor(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief Rasterizes left-hand expression on top of right-hand expression
            @example "abc\\def"
        */
        static void rastNewline(Latex&, std::string&, Image&, SubFunctionType);

		/*
            @brief \raisebox handler
            @details Rasterizes expression and concatenates with some y offset (positive or negative)
            @example "\raisebox{50}{hel}lo" lifts "hel" part up by 50 pixels
            @example "\raisebox{-10}{wo}rld" drops "wo" part down by 10 pixels
        */
		static void rastRaise(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief \it \bold \boldit handler
            @details Sets font weight
            @example "\it~hello" - hello with italic weight
            @example "\bold{hello}" - hello with bold weight
        */
        static void rastSetWeight(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief
        */
        static void rastText(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief Subscript / superscript handler
        */
        static void rastScripts(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief \array \matrix \tabular handler
            @example "\array{a b c\\a b c\\a b c}" - matrix with curly braces
            @example "\matrix{a b c d e f}" - array with square brackets
            @example "\tabular{a b c\\a b c\\a b c}" - matrix without braces/brackets
        */
        static void rastArray(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief Rotation by an arbitrary angle
            @example "\rotate{90}{a}" - rotate "a" by 90 degrees clockwise
            @example "\rotate{-90}~abc" - rotate everything after subfunction by 90 degrees anticlockwise
        */
        static void rastRotate(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief \frac \over \atop \choose handler
            @example "\frac{a}{b}" - a on top of b and line between
            @example "\atop{a}{b}" - a on top of b without line
            @example "\over{a}{b}" - a on top of b and line between
            @example "\choose{a}{b}" - a on top of b without line and parenthesis around them
        */
        static void rastFrac(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief \not \Not \widenot \sout \strikeout \compose handler
        */
        static void rastOverlay(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief \sqrt handler
        */
        static void rastSqrt(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief \evaluate handler
            @example "\evaluate(5+5)"
            @example "\eval(10+6)"
        */
        static void rastEval(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief \today handler
            @details Rasterizes date in format "{day of the week}, {month} {day of the month}, {year}"
            @example "\today"
            @example "\today is now"Handlers::rastAccent  */
        static void rastToday(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief \picture handler
            @details
            @example "\picture(width, height){(x,y){abc}(x,y){def}}"
        */
        static void rastPicture(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief oh...
            @details
            @example
        */
        static void rastAccent(Latex&, std::string&, Image&, SubFunctionType);

        /*
            @brief Math function handler (i.e \cos, \sin, etc.)
            @details
            @example
        */
        static void rastMathFunc(Latex&, std::string&, Image&, SubFunctionType);

};

static const char* dayNames[] = 
{ 
    "Sunday", "Monday", "Tuesday", "Wednesday", 
    "Thursday", "Friday", "Saturday"
};

static const char* monthNames[] = 
{ 
    "January", "February", "March", 
    "April", "May", "June", "July", 
    "August", "September", "October", "November", 
    "December"
};

static const char* mathFuncNames[] =
{
    "error",
    "arccos", "arcsin", "arctan",
    "arg", "sin", "sinh", "cos",
    "cosh", "tan", "tanh", "cot",
    "coth", "csc", "deg", "det",
    "dim", "exp", "gcd", "hom", "inf", 
    "ker", "lg", "lim", "liminf", 
    "limsup", "ln", "log", "max", 
    "min"
};

static Letter cyrTable[] = 
{
	{"A",         1040}, {"B",         1041},
    {"V",         1042}, {"D",         1043},
	{"E",        1044}, {"YO",        1025},
    {"ZH",        1046}, {"Z",         1047},
	{"I",         1048}, {"J",         1049},
    {"K",         1050}, {"L",         1051},
	{"M",         1052}, {"N",         1053},
    {"O",         1054}, {"P",         1055},
	{"R",         1056}, {"S",         1057},
    {"T",         1058}, {"U",         1059},
	{"F",         1060}, {"KH",        1061},
    {"TS",        1062}, {"CH",        1063},
	{"SH",        1064}, {"SHCH",      1065},
    {"\\Cdprime", 1066}, {"\\Yeta",    1067},
	{"\\Cprime",  1068}, {"`E",         1069},
    {"YU",        1070}, {"YA",        1071},
	{"a",         1072}, {"b",         1073},
    {"v",         1074}, {"g",         1075},
	{"d",         1076}, {"e",        1077},
    {"yo",        1105}, {"zh",        1078},
	{"z",         1079}, {"i",         1080},
    {"j",         1081}, {"k",         1082},
	{"l",         1083}, {"m",         1084},
    {"n",         1085}, {"o",         1086},
	{"p",         1087}, {"r",         1088},
    {"s",         1089}, {"t",         1090},
	{"u",         1091}, {"f",         1092},
    {"kh",        1093}, {"ts",        1094},
	{"ch",        1095}, {"sh",        1096},
    {"shch",      1097}, {"\\cdprime", 1098},
	{"\\yeta",    1099}, {"\\cprime",  1100},
    {"`e",         1101}, {"yu",        1102},
	{"ya",        1103}, {NULL,        NULL}
};

static Letter greekTable[] =
{
	{"Alpha",   913}, {"Beta",    914},
    {"Gamma",   915}, {"Delta",   916},
	{"Epsilon", 917}, {"Zeta",    918},
    {"Eta",     919}, {"Theta",   920},
	{"Iota",    921}, {"Kappa",   922},
    {"Lambda",  923}, {"Mu",      924},
	{"Nu",      925}, {"Xi",      926},
    {"Omicron", 927}, {"Pi",      928},
	{"Rho",     929}, {"Sigma",   930},
    {"Tau",     931}, {"Upsilon", 932},
	{"Phi",     933}, {"Chi",     934},
    {"Psi",     935}, {"Omega",   936},
	{"alpha",   945}, {"beta",    946},
    {"gamma",   947}, {"delta",   948},
	{"epsilon", 949}, {"zeta",    950},
    {"eta",     951}, {"theta",   952},
	{"iota",    953}, {"kappa",   954},
    {"lambda",  955}, {"mu",      956},
	{"nu",      957}, {"xi",      958},
    {"omicron", 959}, {"pi",      960},
	{"rho",     961}, {"sigma",   963},
    {"tau",     964}, {"upsilon", 965},
	{"phi",     966}, {"chi",     967},
    {"psi",     968}, {"omega",   969},
    {NULL,      NULL}
};

static Letter miniGreekTable[] =
{
    {"A"/*Alpha*/,   913}, {"B"/*Beta*/,    914},
    {"G"/*Gamma*/,   915}, {"D"/*Delta*/,   916},
	{"E"/*Epsilon*/, 917}, {"Z"/*Zeta*/,    918},
    {"H"/*Eta*/,     919}, {"Q"/*Theta*/,   920},
	{"I"/*Iota*/,    921}, {"K"/*Kappa*/,   922},
    {"L"/*Lambda*/,  923}, {"M"/*Mu*/,      924},
	{"N"/*Nu*/,      925}, {"C"/*Xi*/,      926},
    {"O"/*Omicron*/, 927}, {"P"/*Pi*/,      928},
	{"R"/*Rho*/,     929}, {"S"/*Sigma*/,   930},
    {"T"/*Tau*/,     931}, {"U"/*Upsilon*/, 932},
	{"F"/*Phi*/,     933}, {"X"/*Chi*/,     934},
    {"Y"/*Psi*/,     935}, {"W"/*Omega*/,   936},
	{"a"/*alpha*/,   945}, {"b"/*beta*/,    946},
    {"g"/*gamma*/,   947}, {"d"/*delta*/,   948},
	{"e"/*epsilon*/, 949}, {"z"/*zeta*/,    950},
    {"h"/*eta*/,     951}, {"q"/*theta*/,   952},
	{"i"/*iota*/,    953}, {"k"/*kappa*/,   954},
    {"l"/*lambda*/,  955}, {"m"/*mu*/,      956},
	{"n"/*nu*/,      957}, {"c"/*xi*/,      958},
    {"o"/*omicron*/, 959}, {"p"/*pi*/,      960},
	{"r"/*rho*/,     961}, {"s"/*sigma*/,   963},
    {"t"/*tau*/,     964}, {"u"/*upsilon*/, 965},
	{"f"/*phi*/,     966}, {"x"/*chi*/,     967},
    {"y"/*psi*/,     968}, {"w"/*omega*/,   969},
    {NULL,           NULL}
};

static const SubFunction subfunctions[] = {
    /* ??? */
    {"\\frac",      Handlers::rastFrac,    FRAC_NORMAL},
    {"\\over",      Handlers::rastFrac,    FRAC_OVER},
    {"\\atop",      Handlers::rastFrac,    FRAC_ATOP},
    {"\\choose",    Handlers::rastFrac,    FRAC_CHOOSE},
    {"\\not",       Handlers::rastOverlay, NONE}, // (crossed with /)
    {"\\Not",       Handlers::rastOverlay, NONE}, // (crossed with 45 deg line)
    {"\\widenot",   Handlers::rastOverlay, NONE}, // (crossed with 45 deg line)
    {"\\sout",      Handlers::rastOverlay, NONE}, // (crossed with horizontal line)
    {"\\strikeout", Handlers::rastOverlay, NONE}, // (crossed with horizontal line)
    {"\\compose",   Handlers::rastOverlay, NONE}, // (overlays one expression onto another)
    {"\\sqrt",      Handlers::rastSqrt,    NONE}, //rastsqrt
    {"\\sum",       nullptr,               NONE},
    {"\\prod",      nullptr,               NONE},
    /* Arrays */
    {"\\begin",      nullptr,               NONE}, //rastbegin
    {"\\array",      Handlers::rastArray,   ARR_NORMAL},
    {"\\matrix",     Handlers::rastArray,   ARR_MATRIX},
    {"\\tabular",    Handlers::rastArray,   ARR_TABULAR},
    {"\\picture",    Handlers::rastPicture, NONE},
    {"\\line",       nullptr,               NONE}, //rastline
    {"\\rule",       nullptr,               NONE}, //rastrule
    {"\\circle",     nullptr,               NONE}, //rastcircle
    {"\\bezier",     nullptr,               NONE}, //rastbezier
    {"\\qbezier",    nullptr,               NONE}, //rastbezier
	{"\\raisebox",   Handlers::rastRaise,   NONE},
    {"\\rotatebox",  Handlers::rastRotate,  NONE},
    {"\\magnify",    nullptr,               NONE}, //rastmagnify
    {"\\magbox",     nullptr,               NONE}, //rastmagnify
    {"\\reflectbox", nullptr,               NONE}, //rastreflect
    {"\\fbox",       nullptr,               NONE}, //rastfbox
    {"\\boxed",      nullptr,               NONE}, //rastfbox
    {"\\eval",       Handlers::rastEval,    NONE},
    {"\\evaluate",   Handlers::rastEval,    NONE},
    {"\\today",      Handlers::rastToday,   NONE},
    {"\\calendar",   nullptr,               NONE}, //rastcalendar (month calendar)
    /* Spaces (maybe) */
    /* Newlines */
    {"\\\\", Handlers::rastNewline, NONE},
    /* Arrows */
    {"\\longrightarrow",     nullptr, NONE}, //rastarrow
    {"\\Longrightarrow",     nullptr, NONE}, //rastarrow
    {"\\longleftarrow",      nullptr, NONE}, //rastarrow
    {"\\Longleftarrow",      nullptr, NONE}, //rastarrow
    {"\\longleftrightarrow", nullptr, NONE}, //rastarrow
    {"\\Longleftrightarrow", nullptr, NONE}, //rastarrow
    {"\\longuparrow",        nullptr, NONE}, //rastuparrow
    {"\\Longuparrow",        nullptr, NONE}, //rastuparrow
    {"\\longdownarrow",      nullptr, NONE}, //rastuparrow
    {"\\Longdownarrow",      nullptr, NONE}, //rastuparrow
    {"\\longupdownarrow",    nullptr, NONE}, //rastuparrow
    {"\\Longupdownarrow",    nullptr, NONE}, //rastuparrow
    /* Flags (rastflags) (includes font, fontsize, color) */
    /* Text */
    {"\\cyr",   Handlers::rastText, TEXT_CYR},
    {"\\greek", Handlers::rastText, TEXT_GREEK},
    /* Weight */
    {"\\it",     Handlers::rastSetWeight, FONT_ITALIC},
    {"\\bold",   Handlers::rastSetWeight, FONT_BOLD},
    {"\\boldit", Handlers::rastSetWeight, FONT_BOLDITALIC},
    /* Colors */
    {"\\color",     Handlers::rastColor, COLOR_CUSTOM},
    {"\\red",       Handlers::rastColor, COLOR_RED},
    {"\\green",     Handlers::rastColor, COLOR_GREEN},
    {"\\blue",      Handlers::rastColor, COLOR_BLUE},
    {"\\black",     Handlers::rastColor, COLOR_BLACK},
    {"\\white",     Handlers::rastColor, COLOR_WHITE},
    {"\\gradient",  Handlers::rastColor, COLOR_GRADIENT},
    {"\\reverse",   nullptr,             NONE},
    {"\\reversefg", nullptr,             NONE},
    {"\\reversebg", nullptr,             NONE},
    /* Font sizes (rastSetSize) */
    {"\\tiny",         nullptr, NONE},
    {"\\scriptsize",   nullptr, NONE},
    {"\\footnotesize", nullptr, NONE},
    {"\\small",        nullptr, NONE},
    {"\\normalsize",   nullptr, NONE},
    {"\\large",        nullptr, NONE},
    {"\\Large",        nullptr, NONE},
    {"\\LARGE",        nullptr, NONE},
    {"\\huge",         nullptr, NONE},
    {"\\Huge",         nullptr, NONE},
    {"\\HUGE",         nullptr, NONE},
    /* Accents (rastaccent) */
    {"\\overbrace",           Handlers::rastAccent, NONE},
    {"\\underbrace",          Handlers::rastAccent, NONE},
    {"\\overline",            Handlers::rastAccent, NONE},
    {"\\underline",           Handlers::rastAccent, NONE},
    {"\\vec",                 Handlers::rastAccent, NONE},
    {"\\widevec",             Handlers::rastAccent, NONE},
    {"\\overarrow",           Handlers::rastAccent, NONE},
    {"\\overrightarrow",      Handlers::rastAccent, NONE},
    {"\\Overrightarrow",      Handlers::rastAccent, NONE},
    {"\\overleftarrow",       Handlers::rastAccent, NONE},
    {"\\Overleftarrow",       Handlers::rastAccent, NONE},
    {"\\underarrow",          Handlers::rastAccent, NONE},
    {"\\underrightarrow",     Handlers::rastAccent, NONE},
    {"\\Underrightarrow",     Handlers::rastAccent, NONE},
    {"\\underleftarrow",      Handlers::rastAccent, NONE},
    {"\\Underleftarrow",      Handlers::rastAccent, NONE},
    {"\\overleftrightarrow",  Handlers::rastAccent, NONE},
    {"\\Overleftrightarrow",  Handlers::rastAccent, NONE},
    {"\\underleftrightarrow", Handlers::rastAccent, NONE},
    {"\\Underleftrightarrow", Handlers::rastAccent, NONE},
    {"\\bar",                 Handlers::rastAccent, NONE},
    {"\\widebar",             Handlers::rastAccent, NONE},
    {"\\hat",                 Handlers::rastAccent, NONE},
    {"\\widehat",             Handlers::rastAccent, NONE},
    {"\\tilde",               Handlers::rastAccent, NONE},
    {"\\widetilde",           Handlers::rastAccent, NONE},
    {"\\dot",                 Handlers::rastAccent, NONE},
    {"\\widedot",             Handlers::rastAccent, NONE},
    {"\\ddot",                Handlers::rastAccent, NONE},
    {"\\wideddot",            Handlers::rastAccent, NONE},
    /* Math functions (rastmathfunc) */
    {"\\arccos", Handlers::rastMathFunc, MATH_ARCCOS},
    {"\\arcsin", Handlers::rastMathFunc, MATH_ARCSIN},
    {"\\arctan", Handlers::rastMathFunc, MATH_ARCTAN},
    {"\\arg",    Handlers::rastMathFunc, MATH_ARG},
    {"\\cos",    Handlers::rastMathFunc, MATH_COS}, //cosine
    {"\\cosh",   Handlers::rastMathFunc, MATH_COSH},
    {"\\sin",    Handlers::rastMathFunc, MATH_SIN}, //sine
    {"\\sinh",   Handlers::rastMathFunc, MATH_SINH},
    {"\\tan",    Handlers::rastMathFunc, MATH_TAN}, //tangent
    {"\\tanh",   Handlers::rastMathFunc, MATH_TANH},
    {"\\cot",    Handlers::rastMathFunc, MATH_COT}, //cotangent
    {"\\coth",   Handlers::rastMathFunc, MATH_COTH},
    {"\\csc",    Handlers::rastMathFunc, MATH_CSC},
    {"\\deg",    Handlers::rastMathFunc, MATH_DEG},
    {"\\det",    Handlers::rastMathFunc, MATH_DET},
    {"\\dim",    Handlers::rastMathFunc, MATH_DIM},
    {"\\exp",    Handlers::rastMathFunc, MATH_EXP},
    {"\\gcd",    Handlers::rastMathFunc, MATH_GCD},
    {"\\hom",    Handlers::rastMathFunc, MATH_HOM},
    {"\\inf",    Handlers::rastMathFunc, MATH_INF},
    {"\\ker",    Handlers::rastMathFunc, MATH_KER},
    {"\\lg",     Handlers::rastMathFunc, MATH_LG},
    {"\\lim",    Handlers::rastMathFunc, MATH_LIM},
    {"\\liminf", Handlers::rastMathFunc, MATH_LIMINF},
    {"\\limsup", Handlers::rastMathFunc, MATH_LIMSUP},
    {"\\ln",     Handlers::rastMathFunc, MATH_LN},
    {"\\log",    Handlers::rastMathFunc, MATH_LOG},
    {"\\max",    Handlers::rastMathFunc, MATH_MAX},
    {"\\min",    Handlers::rastMathFunc, MATH_MIN},
    {"\\Pr",     Handlers::rastMathFunc, MATH_PR},
    {"\\sec",    Handlers::rastMathFunc, MATH_SEC},
    {"\\}", nullptr, NONE},
    {NULL, nullptr, NONE}
};