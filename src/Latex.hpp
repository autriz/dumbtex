#pragma once

#include "Image.hpp"

#include <stdexcept>
#include <cstring>
#include <stack>
#include <iostream>

#if !defined(DEBUG) || !defined(PROFILER)
#include <algorithm>
#include <chrono>
#endif

struct Scripts;

enum ScriptType { SUBSCRIPT = 1, SUPSCRIPT, BOTH };

class Latex {

	public:

		/*
			@brief Constructs a Latex instance.
		*/
		Latex();

		Latex(const Latex& other) = delete;

		Latex(Latex& other) = delete;

		Latex(Latex&& other) = delete;

		~Latex();

		/*
			@brief Renders image and returns it.
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
	/* rastOverlay types */
	OVERLAY_SLASH,
	OVERLAY_DIAG_LINE,
	OVERLAY_HOR_LINE,
	OVERLAY_COMPOSE,
	/* rastArray types */
	ARR_NORMAL,
	ARR_MATRIX,
	ARR_TABULAR,
	/* rastBezier types */
	BEZIER_CUBIC,
	BEZIER_QUADRATIC,
	/* rastArrow types */
	ARROW_RIGHT,
	ARROW_BRIGHT,
	ARROW_LEFT,
	ARROW_BLEFT,
	ARROW_LEFTRIGHT,
	ARROW_BLEFTRIGHT,
	ARROW_UP,
	ARROW_BUP,
	ARROW_DOWN,
	ARROW_BDOWN,
	ARROW_UPDOWN,
	ARROW_BUPDOWN,
	/* rastMathFunc types */
	MATH_ARCCOS = 400,
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
	SubFunctionType type;
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
			@brief \\ handler
			@details Rasterizes left-hand expression on top of right-hand expression
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
			@details
			@example
		*/
		static void rastText(Latex&, std::string&, Image&, SubFunctionType);

		/*
			@brief Subscript / superscript handler
			@details
			@example
		*/
		static void rastScripts(Latex&, std::string&, Image&, SubFunctionType);

		/*
			@brief \begin \end handler
			@details
			@example
		*/
		static void rastBegin(Latex&, std::string&, Image&, SubFunctionType);

		/*
			@brief \array \matrix \tabular handler
			@details
			@example "\array{a b c\\a b c\\a b c}" - matrix with curly braces
			@example "\matrix{a b c d e f}" - array with square brackets
			@example "\tabular{a b c\\a b c\\a b c}" - matrix without braces/brackets
		*/
		static void rastArray(Latex&, std::string&, Image&, SubFunctionType);

		/*
			@brief \rotatebox handler
			@details Rotates image by an arbitrary angle
			@example "\rotate{90}{a}" - rotate "a" by 90 degrees clockwise
			@example "\rotate{-90}~abc" - rotate everything after subfunction by 90 degrees anticlockwise
		*/
		static void rastRotate(Latex&, std::string&, Image&, SubFunctionType);

		/*
			@brief \frac \over \atop \choose handler
			@details
			@example "\frac{a}{b}" - a on top of b and line between
			@example "\atop{a}{b}" - a on top of b without line
			@example "\over{a}{b}" - a on top of b and line between
			@example "\choose{a}{b}" - a on top of b without line and parenthesis around them
		*/
		static void rastFrac(Latex&, std::string&, Image&, SubFunctionType);

		/*
			@brief \not \Not \widenot \sout \strikeout \compose handler
			@details
			@example
		*/
		static void rastOverlay(Latex&, std::string&, Image&, SubFunctionType);

		/*
			@brief \sqrt handler
			@details
			@example
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

		/*
			@brief
			@details
			@example
		*/
	   static void rastGRChar(Latex&, std::string&, Image&, SubFunctionType);

		/*
			@brief \bezier \qbezier handler
			@details
			@example
		*/
	   static void rastBezier(Latex&, std::string&, Image&, SubFunctionType);

		/*
			@brief \magnify \magbox handler
			@details Magnifies subexpression
			@example "\magnify{2}{hello}" - word hello magnified by 2 times
			@example "\magbox{2}{hello}" - same as \magnify
		*/
	   static void rastMagnify(Latex&, std::string&, Image&, SubFunctionType);

		/*
			@brief \fbox \boxed handler
			@details Wraps subexpression in a "box"
			@example "\fbox[200,200]{hello}" - creates 200x200 box with border, centers hello in it
		*/
	   static void rastFBox(Latex&, std::string&, Image&, SubFunctionType);

		/*
			@brief \longleftarrow \longrightarrow \etc. handler
			@details Rasterizes arrow
			@example "a\longleftarrowb" - "a⟶b"
		*/
	   static void rastArrow(Latex&, std::string&, Image&, SubFunctionType);

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

static const Letter cyrTable[] = 
{
	{"A",         1040}, {"B",        1041},
	{"V",         1042}, {"D",        1043},
	{"E",        1044}, {"YO",        1025},
	{"ZH",        1046}, {"Z",        1047},
	{"I",         1048}, {"J",        1049},
	{"K",         1050}, {"L",        1051},
	{"M",         1052}, {"N",        1053},
	{"O",         1054}, {"P",        1055},
	{"R",         1056}, {"S",        1057},
	{"T",         1058}, {"U",        1059},
	{"F",         1060}, {"KH",       1061},
	{"TS",        1062}, {"CH",       1063},
	{"SH",        1064}, {"SHCH",     1065},
	{"\\Cdprime", 1066}, {"Y",        1067},
	{"\\Yeta",    1067}, {"\\Cprime", 1068},
	{"`E",        1069}, {"YU",       1070},
	{"YA",        1071}, {"a",        1072},
	{"b",         1073}, {"v",        1074},
	{"g",         1075}, {"d",        1076},
	{"e",         1077}, {"yo",       1105},
	{"zh",        1078}, {"z",        1079},
	{"i",         1080}, {"j",        1081},
	{"k",         1082}, {"l",        1083},
	{"m",         1084}, {"n",        1085},
	{"o",         1086}, {"p",        1087},
	{"r",         1088}, {"s",        1089},
	{"t",         1090}, {"u",        1091},
	{"f",         1092}, {"kh",       1093},
	{"ts",        1094}, {"ch",       1095},
	{"sh",        1096}, {"shch",     1097},
	{"\\cdprime", 1098}, {"y",        1099},
	{"\\yeta",    1099}, {"\\cprime", 1100},
	{"`e",        1101}, {"yu",       1102},
	{"ya",        1103}, {NULL,       0}
};

static const Letter greekTable[] =
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
	{NULL,           0}
};

static const SubFunction subfunctions[] = {
	/* ??? */
	{"\\frac",      Handlers::rastFrac,    FRAC_NORMAL},
	{"\\over",      Handlers::rastFrac,    FRAC_OVER},
	{"\\atop",      Handlers::rastFrac,    FRAC_ATOP},
	{"\\choose",    Handlers::rastFrac,    FRAC_CHOOSE},
	{"\\not",       Handlers::rastOverlay, OVERLAY_SLASH}, // (crossed with /)
	{"\\Not",       Handlers::rastOverlay, OVERLAY_DIAG_LINE}, // (crossed with 45 deg line)
	{"\\widenot",   Handlers::rastOverlay, OVERLAY_DIAG_LINE}, // (crossed with 45 deg line)
	{"\\sout",      Handlers::rastOverlay, OVERLAY_HOR_LINE}, // (crossed with horizontal line)
	{"\\strikeout", Handlers::rastOverlay, OVERLAY_HOR_LINE}, // (crossed with horizontal line)
	{"\\compose",   Handlers::rastOverlay, OVERLAY_COMPOSE},
	{"\\sqrt",      Handlers::rastSqrt,    NONE}, //rastsqrt
	{"\\sum",       nullptr,               NONE},
	{"\\prod",      nullptr,               NONE},
	/* Arrays */
	{"\\begin",      Handlers::rastBegin,   NONE}, //rastbegin
	{"\\array",      Handlers::rastArray,   ARR_NORMAL},
	{"\\matrix",     Handlers::rastArray,   ARR_MATRIX},
	{"\\tabular",    Handlers::rastArray,   ARR_TABULAR},
	{"\\picture",    Handlers::rastPicture, NONE},
	{"\\line",       nullptr,               NONE}, //rastline
	{"\\rule",       nullptr,               NONE}, //rastrule
	{"\\circle",     nullptr,               NONE}, //rastcircle
	{"\\bezier",     Handlers::rastBezier, 	BEZIER_CUBIC},
	{"\\qbezier",    Handlers::rastBezier,  BEZIER_QUADRATIC},
	{"\\raisebox",   Handlers::rastRaise,   NONE},
	{"\\rotatebox",  Handlers::rastRotate,  NONE},
	{"\\magnify",    Handlers::rastMagnify, NONE},
	{"\\magbox",     Handlers::rastMagnify, NONE},
	{"\\reflectbox", nullptr,               NONE}, //rastreflect
	{"\\fbox",       Handlers::rastFBox,    NONE},
	{"\\boxed",      Handlers::rastFBox,    NONE},
	{"\\eval",       Handlers::rastEval,    NONE},
	{"\\evaluate",   Handlers::rastEval,    NONE},
	{"\\today",      Handlers::rastToday,   NONE},
	{"\\calendar",   nullptr,               NONE}, //rastcalendar (month calendar)
	/* Spaces (maybe) */
	/* Newlines */
	{"\\n",  Handlers::rastNewline, NONE},
	{"\\\\", Handlers::rastNewline, NONE},
	/* Arrows */
	{"\\longrightarrow",     Handlers::rastArrow, ARROW_RIGHT}, //rastarrow
	{"\\Longrightarrow",     Handlers::rastArrow, ARROW_BRIGHT}, //rastarrow
	{"\\longleftarrow",      Handlers::rastArrow, ARROW_LEFT}, //rastarrow
	{"\\Longleftarrow",      Handlers::rastArrow, ARROW_BLEFT}, //rastarrow
	{"\\longleftrightarrow", Handlers::rastArrow, ARROW_LEFTRIGHT}, //rastarrow
	{"\\Longleftrightarrow", Handlers::rastArrow, ARROW_BLEFTRIGHT}, //rastarrow
	{"\\longuparrow",        Handlers::rastArrow, ARROW_UP}, //rastuparrow
	{"\\Longuparrow",        Handlers::rastArrow, ARROW_BUP}, //rastuparrow
	{"\\longdownarrow",      Handlers::rastArrow, ARROW_DOWN}, //rastuparrow
	{"\\Longdownarrow",      Handlers::rastArrow, ARROW_BDOWN}, //rastuparrow
	{"\\longupdownarrow",    Handlers::rastArrow, ARROW_UPDOWN}, //rastuparrow
	{"\\Longupdownarrow",    Handlers::rastArrow, ARROW_BUPDOWN}, //rastuparrow
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
	{"\\reverse",   nullptr,             NONE}, // huh
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
	{"\\{", nullptr, NONE},
	{"\\}", nullptr, NONE},
	/* Greek characters */
	{"\\Alpha",   Handlers::rastGRChar, (SubFunctionType)913},
	{"\\Beta",    Handlers::rastGRChar, (SubFunctionType)914},
	{"\\Gamma",   Handlers::rastGRChar, (SubFunctionType)915},
	{"\\Delta",   Handlers::rastGRChar, (SubFunctionType)916},
	{"\\Epsilon", Handlers::rastGRChar, (SubFunctionType)917},
	{"\\Zeta",    Handlers::rastGRChar, (SubFunctionType)918},
	{"\\Eta",     Handlers::rastGRChar, (SubFunctionType)919},
	{"\\Theta",   Handlers::rastGRChar, (SubFunctionType)920},
	{"\\Iota",    Handlers::rastGRChar, (SubFunctionType)921},
	{"\\Kappa",   Handlers::rastGRChar, (SubFunctionType)922},
	{"\\Lambda",  Handlers::rastGRChar, (SubFunctionType)923},
	{"\\Mu",      Handlers::rastGRChar, (SubFunctionType)924},
	{"\\Nu",      Handlers::rastGRChar, (SubFunctionType)925},
	{"\\Xi",      Handlers::rastGRChar, (SubFunctionType)926},
	{"\\Omicron", Handlers::rastGRChar, (SubFunctionType)927},
	{"\\Pi",      Handlers::rastGRChar, (SubFunctionType)928},
	{"\\Rho",     Handlers::rastGRChar, (SubFunctionType)929},
	{"\\Sigma",   Handlers::rastGRChar, (SubFunctionType)930},
	{"\\Tau",     Handlers::rastGRChar, (SubFunctionType)931},
	{"\\Upsilon", Handlers::rastGRChar, (SubFunctionType)932},
	{"\\Phi",     Handlers::rastGRChar, (SubFunctionType)933},
	{"\\Chi",     Handlers::rastGRChar, (SubFunctionType)934},
	{"\\Psi",     Handlers::rastGRChar, (SubFunctionType)935},
	{"\\Omega",   Handlers::rastGRChar, (SubFunctionType)936},
	{"\\alpha",   Handlers::rastGRChar, (SubFunctionType)945},
	{"\\beta",    Handlers::rastGRChar, (SubFunctionType)946},
	{"\\gamma",   Handlers::rastGRChar, (SubFunctionType)947},
	{"\\delta",   Handlers::rastGRChar, (SubFunctionType)948},
	{"\\epsilon", Handlers::rastGRChar, (SubFunctionType)949},
	{"\\zeta",    Handlers::rastGRChar, (SubFunctionType)950},
	{"\\eta",     Handlers::rastGRChar, (SubFunctionType)951},
	{"\\theta",   Handlers::rastGRChar, (SubFunctionType)952},
	{"\\iota",    Handlers::rastGRChar, (SubFunctionType)953},
	{"\\kappa",   Handlers::rastGRChar, (SubFunctionType)954},
	{"\\lambda",  Handlers::rastGRChar, (SubFunctionType)955},
	{"\\mu",      Handlers::rastGRChar, (SubFunctionType)956},
	{"\\nu",      Handlers::rastGRChar, (SubFunctionType)957},
	{"\\xi",      Handlers::rastGRChar, (SubFunctionType)958},
	{"\\omicron", Handlers::rastGRChar, (SubFunctionType)959},
	{"\\pi",      Handlers::rastGRChar, (SubFunctionType)960},
	{"\\rho",     Handlers::rastGRChar, (SubFunctionType)961},
	{"\\sigma",   Handlers::rastGRChar, (SubFunctionType)963},
	{"\\tau",     Handlers::rastGRChar, (SubFunctionType)964},
	{"\\upsilon", Handlers::rastGRChar, (SubFunctionType)965},
	{"\\phi",     Handlers::rastGRChar, (SubFunctionType)966},
	{"\\chi",     Handlers::rastGRChar, (SubFunctionType)967},
	{"\\psi",     Handlers::rastGRChar, (SubFunctionType)968},
	{"\\omega",   Handlers::rastGRChar, (SubFunctionType)969},
	{NULL, nullptr, NONE}
};