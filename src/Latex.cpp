#include "Latex.hpp"

/*
	DumbTeX
	Rasterization of LaTeX-like expressions to image
	
	Image's 0,0 at top left corner, meaning that Y coordinate is downwards

*/

Latex::Latex(): p_SelectedFont(&m_NormalFont) { };

Latex::~Latex() { };

Latex& Latex::Get()
{
	static Latex instance;
	return instance;
}

Image Latex::toImage(std::string& expression)
{
	PROFILE_SCOPE("Latex::toImage"); 

   /*
		toImage()
		|__rastlimits()
		   |__rastscripts()
			  |__texscripts()
   
   */

	Image finalImage;
	size_t i, j;
	static bool isFirstIteration = false;
	SubFunction subFunction;

	#ifdef DEBUG
		printf("[Latex::toImage] start expression: %s\n", expression.c_str());
	#endif

	if (expression.length() == 0)
		throw std::runtime_error("There's nothing to rasterize.");

	if (isFirstIteration == false) 
	{
		prepExpression(expression); //prepare expression to find unsupported subfunctions and remove unnecessary braces, if found
		isFirstIteration = true;
	}

	#ifdef DEBUG
		printf("[Latex::toImage] prepared start expression: %s\n", expression.c_str());
	#endif

	while (expression.length() > 0)
	{
		for (i = 0; i < expression.length(); ++i)
		{
			Image tempImage;

			#ifdef DEBUG
				if (i != 0) printf("[Latex::toImage] expression: %s\n", expression.c_str());
			#endif

			if (expression[i] == '_' || expression[i] == '^')
			{
				Handlers::rastScripts(*this, expression, finalImage, NONE);
				break;
			}
			else if (expression[i] == '{')
			{
				std::string subexpression = Latex::getSubExpression(expression, 0, '{', '}', false);

				if (expression.length() <= 0)
					expression = subexpression;
				else
					finalImage.concat(toImage(subexpression));
				break;
			}
			else if (expression[i] == '\\')
			{
				if ((subFunction = getSubFunction(expression, i)).expression != NULL)
				{
					//?Esaped delimeters like "\}" or "\{" are processed after everything between them is rasterized
					//?Because escaped delims depend on size of all expression

					//!Take inspiration from MimeTeX's texsubexpr() function, quite a gold mine
					expression.erase(i, strlen(subFunction.expression));

					if (subFunction.handler) 
						subFunction.handler(*this, expression, finalImage, subFunction.type);
						
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
			}
			else
			{
				tempImage.rasterizeCharacter(getSelectedFont(), expression[i], p_Color);
				expression.erase(i, 1);
				if (expression[i] == '_' || expression[i] == '^')
					Handlers::rastScripts(*this, expression, tempImage, NONE);
			}

			if (!tempImage.isEmpty()) 
				finalImage.concat(tempImage);
			break;
		}

		i = 0;
	}

	return finalImage;
};

void Latex::prepExpression(std::string& expression)
{
	PROFILE_SCOPE("Latex::prepExpression");

	if (expression.length() == 0) return;

	auto getFunc = [](const std::string& expression, size_t at) -> userFunction
	{
		size_t i, match;

		for (i = 0; userDefFunctions[i].expression != NULL; i++)
		{
			match = expression.find(userDefFunctions[i].expression, at);

			if (match != std::string::npos && match == at)
				return userDefFunctions[i];
		};

		return userDefFunctions[i];
	};

	const char* comment = "%%";
	std::stack<char> brackets;
	userFunction userFunc;
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
	for (size_t i = 0; i < expression.length(); ++i)
	{
		if (expression[i] == '{') // found open bracket
			brackets.push(expression[i]); // push it to stack

		if (expression[i] == '}') // found close bracket
		{
			if (brackets.size() > 0) // if stack isn't empty
				brackets.pop(); // remove open bracket from stack
			else
				expression.erase(i, 1); // erase unfixed close bracket
		}
	}

	if (!brackets.empty())
		for (size_t i = 0; i < brackets.size(); i++)
			expression.append("}"); //add close bracket for every unfixed open bracket

	//replace user-defined functions with its equivalent
	for (size_t i = 0; i < expression.length(); ++i)
	{
		if (expression[i] == '\\')
		{
			if ((userFunc = getFunc(expression, i)).expression != NULL)
			{
				expression.erase(i, strlen(userFunc.expression));
				expression.insert(i, userFunc.equivalent);
				i = 0;
			}
		}
	}

	//Erase spaces (optional)

	//Convert \\left( to \\( and \\right) to \\)
	// while ( )
};

void Latex::setFont(FontType type, const char* pathToFont, uint16_t size) 
{ 
	switch(type)
	{
		case FontType::Normal:
			m_NormalFont.setFont(pathToFont);
			m_NormalFont.m_Type = FontType::Normal;
			m_NormalFont.setSize(size);
			break;
		case FontType::Italic:
			m_ItalicFont.setFont(pathToFont);
			m_ItalicFont.m_Type = FontType::Italic;
			m_ItalicFont.setSize(size);
			break;
		case FontType::Bold:
			m_BoldFont.setFont(pathToFont);
			m_BoldFont.m_Type = FontType::Bold;
			m_BoldFont.setSize(size);
			break;
		case FontType::BoldItalic:
			m_BoldItalicFont.setFont(pathToFont);
			m_BoldItalicFont.m_Type = FontType::BoldItalic;
			m_BoldItalicFont.setSize(size);
			break;
	}
};

void Latex::setFonts(const char* normal, const char* italic, const char* bold, const char* boldItalic) 
{ 
	PROFILE_SCOPE("Latex::setFonts");

	if (strlen(normal) > 0) this->setFont(FontType::Normal, normal); 
	if (strlen(italic) > 0) this->setFont(FontType::Italic, italic); 
	if (strlen(bold) > 0) this->setFont(FontType::Bold, bold);
	if (strlen(boldItalic) > 0) this->setFont(FontType::BoldItalic, boldItalic);
};

Font& Latex::getSelectedFont()
{
	return *p_SelectedFont;
};

void Latex::setSelectedFont(FontType type)
{
	switch(type)
	{
		case FontType::Normal:
			p_SelectedFont = &m_NormalFont;
			break;
		case FontType::Italic:
			p_SelectedFont = &m_ItalicFont;
			break;
		case FontType::Bold:
			p_SelectedFont = &m_BoldFont;
			break;
		case FontType::BoldItalic:
			p_SelectedFont = &m_BoldItalicFont;
			break;
	}
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

std::string Latex::getSubExpression(std::string& expression, unsigned from, const char left, const char right, bool returnDelims)
{
	PROFILE_SCOPE("Latex::getSubExpression");

	unsigned first, last;
	std::string result;
	std::stack<char> brackets;

	if (expression[from] != left) // if expression not starts from left delimeter
	{
		result = expression[from];
		expression.erase(from, 1);
		return result; //just return first char
	}

	first = expression.find(left, from); //find index of left delimeter
	if (first > from) return NULL;

	for (size_t i = 0; i < expression.length(); i++) //find index of right delimeter
	{
		if (expression[i] == left)
			brackets.push(expression[i]);

		if (expression[i] == right)
		{
			if (brackets.size() > 1)
				brackets.pop();
			else
			{
				last = i;
				break;
			}
		}
	}

	while (expression[last - 1] == '\\') //if delimeter is escaped
		last = expression.find(right, last+1); //find right delimeter again

	result = returnDelims ? expression.substr(first, last - first + 1) : expression.substr(first + 1, last - first - 1);
	expression.erase(0, returnDelims ? result.length() : result.length() + 2);

	return result;
};

const struct SubFunction Latex::getSubFunction(const std::string& expression, size_t at)
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

const struct Scripts Latex::texScripts(std::string& expression, ScriptType which)
{
	PROFILE_SCOPE("Latex::texScripts");

	bool gotSub = false, gotSup = false;
	std::string subScript, supScript;

	while (expression.length() > 0)
	{
		if (expression[0] == '_' && (which == 1 || which == 3) && !gotSub)
		{
			expression.erase(0, 1);
			gotSub = true;
			subScript = Latex::getSubExpression(expression, 0, '{', '}', false);
		}
		else if (expression[0] == '^' && (which == 2 || which == 3) && !gotSup)
		{
			expression.erase(0, 1);
			gotSup = true;
			supScript = Latex::getSubExpression(expression, 0, '{', '}', false);
		}
		else
			return {subScript, supScript};
	}

	return {subScript, supScript};
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

void Handlers::rastNewline(Latex& latex, std::string& expression, Image& image, SubFunctionType arg1)
{
	PROFILE_SCOPE("Handlers::rastNewline");

	int space = 0;
	std::string arg;

	if (image.isEmpty() || expression.length() == 0) return;

	//optional parameter
	if (expression[0] == '[')
		arg = Latex::getSubExpression(expression, 0, '[', ']', false);

	if (arg.length() != 0 && arg.find_first_not_of("0123456789") == std::string::npos)
		space = std::stoi(arg);

	image.concat(latex.toImage(expression), ImagePosition::BOTTOM, space);
};

void Handlers::rastColor(Latex& latex, std::string& expression, Image& image, SubFunctionType colorType)
{
	PROFILE_SCOPE("Handlers::rastColor");

	Image tempImage;

	if (colorType == COLOR_GRADIENT) //color with gradient
	{
		std::string hex1, hex2, arg;

		hex1 = Latex::getSubExpression(expression, 0, '{', '}', false);
		if (hex1.length() == 0) return;

		hex2 = Latex::getSubExpression(expression, 0, '{', '}', false);
		if (hex2.length() == 0) return;

		if (expression[0] == '~')
		{
			arg = expression.substr(1);
			expression.erase(0);
		}
		else
		{
			arg = Latex::getSubExpression(expression, 0, '{', '}', false);
			if (arg.length() == 0) return;
		}

		tempImage = latex.toImage(arg);
		tempImage.gradient(hexToRGBA(hex1), hexToRGBA(hex2));
	}
	else //color with certain color
	{
		Color tempColor = latex.getFontColor();
		std::string hex, arg;
		
		if (colorType == COLOR_CUSTOM)
		{
			hex = Latex::getSubExpression(expression, 0, '{', '}', false);
			if (hex.length() == 0) return;
		}
		else
			hex = 
				colorType == COLOR_RED ? "ff0000" : 
				colorType == COLOR_GREEN ? "00ff00" : 
				colorType == COLOR_BLUE ? "0000ff" : 
				colorType == COLOR_WHITE ? "ffffff" : "000000";
		
		latex.setFontColor(hexToRGBA(hex));

		if (expression[0] == '~') //color all text after subfunction 
		{
			arg = expression.substr(1);
			if (arg.length() == 0) return;

			expression.erase(0);
			tempImage = latex.toImage(arg);
		}
		else //color only text in brackets
		{
			arg = Latex::getSubExpression(expression, 0, '{', '}', false);
			if (arg.length() == 0) return;

			tempImage = latex.toImage(arg);
			latex.setFontColor(tempColor);
		}
	}

	image.concat(tempImage);
};

void Handlers::rastRaise(Latex& latex, std::string& expression, Image& image, SubFunctionType arg1)
{
	PROFILE_SCOPE("Handlers::rastRaise");

	Image tempImage;
	int lift_num;
	std::string lift, arg;

	lift = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (lift.length() == 0) return;

	arg = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (arg.length() == 0) return;

	if (lift.find_first_not_of("-0123456789") != std::string::npos) return;

	lift_num = std::stoi(lift);
	tempImage = latex.toImage(arg);

	if (!tempImage.isEmpty())
	{
		tempImage.m_Baseline += lift_num;
		tempImage.m_AdvanceHeight -= lift_num > 0 ? lift_num : -lift_num;

		image.concat(tempImage);
	}
};

void Handlers::rastText(Latex& latex, std::string& expression, Image& image, SubFunctionType textType)
{
	PROFILE_SCOPE("Handlers::rastText");

	auto findLetter = [](const Letter* table, std::string& expr) -> const Letter*
	{
		size_t i, match;
		const Letter* letter;

		for (i = 0; table[i].character != NULL; ++i)
		{
			match = expr.find(table[i].character, 0);

			if (match != std::string::npos && match == 0)
				letter = &table[i];
		};

		return expr.find(letter->character, 0) == 0 ? letter : &table[i];
	};

	Image tempImage;
	const Letter* letter;
	std::string subexpression;

	subexpression = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (subexpression.length() == 0) return;

	while (subexpression.length() > 0)
	{
		#ifdef DEBUG
			printf("[Handlers::rastText] subexpression = %s\n", subexpression.c_str());
		#endif
		switch (textType)
		{
			case TEXT_CYR:
				letter = findLetter(cyrTable, subexpression);
				break;
			case TEXT_GREEK:
				letter = findLetter(greekTable, subexpression);
				break;
			default:
				break;
		}

		if (letter->character != NULL)
		{
			subexpression.erase(0, strlen(letter->character));
			tempImage.rasterizeCharacter(latex.getSelectedFont(), letter->charCode, latex.getFontColor());
		}
		else
			subexpression.erase(0, 1);
	}

	image.concat(tempImage);
};

void Handlers::rastSetWeight(Latex& latex, std::string& expression, Image& image, SubFunctionType fontType)
{
	PROFILE_SCOPE("Handlers::rastSetWeight");

	Image tempImage;
	FontType type = latex.getSelectedFont().m_Type;
	std::string subexpression;

	subexpression = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (subexpression.length() == 0) return;

	latex.setSelectedFont(
		fontType == FONT_REGULAR ? FontType::Normal : 
		fontType == FONT_ITALIC ? FontType::Italic : 
		fontType == FONT_BOLD ? FontType::Bold : 
		fontType == FONT_BOLDITALIC ? FontType::BoldItalic : FontType::Normal
	);

	tempImage = latex.toImage(subexpression);

	latex.setSelectedFont(type);

	image.concat(tempImage);
};

void Handlers::rastScripts(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastScripts");

	Image subImg, supImg;
	Details subDetails = {0, 0, 0, 0, 0}, supDetails = {0, 0, 0, 0, 0};
	int newWidth, newHeight;
	bool isSub, isSup;
	float sizeOff = 0.6f;
	Scripts scripts;
	Font& font = latex.getSelectedFont();

	if (expression.length() == 0) return;

	scripts = Latex::texScripts(expression, ScriptType::BOTH);

	font.setSize(font.m_SFT.xScale * sizeOff);

	if (scripts.subScript.length() != 0)
	{
		isSub = true;
		subImg = latex.toImage(scripts.subScript); // implement proper size changer
	}

	if (scripts.supScript.length() != 0)
	{
		isSup = true;
		supImg = latex.toImage(scripts.supScript);
	}

	font.setSize(font.m_SFT.xScale / sizeOff);

	if (!isSub && !isSup) return;

	if (isSub)
		subDetails = subImg.getDetails();

	if (isSup)
		supDetails = supImg.getDetails();

	#ifdef DEBUG
		printf("[Handlers::rastScripts] Subscript details: height = %d, width = %d\n", subDetails.height, subDetails.width);
		printf("[Handlers::rastScripts] Superscript details: height = %d, width = %d\n", supDetails.height, supDetails.width);
	#endif

	newWidth = subDetails.width > supDetails.width ? subDetails.width : supDetails.width;
	newHeight = (image.m_Height - image.m_AdvanceHeight) + subDetails.height + supDetails.height;

	#ifdef DEBUG
		printf("[Handlers::rastScripts] Combined height = %d, width = %d\n", newHeight, newWidth);
	#endif

	Image tempImg(newWidth, newHeight, 4);
	tempImg.m_Baseline = newHeight - subDetails.height;
	if (subDetails.height > 0) tempImg.m_AdvanceHeight += subDetails.height;

	tempImg.overlay(supImg, 0, 0);
	tempImg.overlay(subImg, 0, newHeight - subDetails.height);

	image.concat(tempImg);
};

void Handlers::rastBegin(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastBegin");

	return; //!!!!
	auto findEnv = [](const char* table[], std::string& expr) -> int
	{
		size_t i, match;
		const char* env;

		for (i = 0; table[i] != NULL; ++i)
		{
			match = expr.find(table[i], 0);

			if (match != std::string::npos && match == 0)
				return i;
		};

		return i;
	};

	std::string subexpression, environment;
	const char* end = "\\end";

	const char* environments[] =
	{
		"eqnarray", "array", "matrix", "tabular", 
		"pmatrix", "bmatrix", "Bmatrix", "vmatrix", 
		"Vmatrix", "gather", "align", "verbatim",
		"picture", "cases", "equation", NULL
	};

	// get used environment
	environment = Latex::getSubExpression(expression, 0, '{', '}', false);

	// eh...
	switch (findEnv(environments, environment))
	{
		case 0: // \eqnarray
			break;
		case 1: // \array
			break;
		case 2: // \matrix
			break;
		case 3: // \tabular
			break;
		case 4: // \pmatrix ( ... )
			break;
		case 5: // \bmatrix [ ... ]
			break;
		case 6: // \Bmatrix { ... }
			break;
		case 7: // \vmatrix | ... |
			break;
		case 8: // \Vmatrix || ... ||
			break;
		case 9: // \gather
			break;
		case 10: // \align
			break;
		case 11: // \verbatim
			break;
		case 12: // \picture
			break;
		case 13: // \cases
			break;
		case 14: // \equation
			break;
		default:
			return;
	}

	// find first \end{...} token
	// if token is found, and environment isn't matching, check for another 
	// \begin between them, or check for another \end after first one

	/* 
		get all thingys between \begin{...} and \end{...} (everything before 
		\end{...}, to be exact, 'cause \begin is chopped before handler 
		calling, and {...} is chopped after Latex::getSubExpression)
	*/

}

void Handlers::rastArray(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastArray");

	// search for subexpression between brackets
	// rasterize it
	// rasterize left bracket the same height as rasterized expression
	// create bracket rasterizer with some height/width parameters
	// do the same with right bracket
	// lbracket + subexpr + rbracket
	// concat result with image

	Image exprImg, tempImg; 
	std::string subexpr;

	subexpr = Latex::getSubExpression(expression, 0, '{', '}', false);
	exprImg = latex.toImage(subexpr);

	switch(type)
	{
		case ARR_NORMAL:
		case ARR_MATRIX:
		{
			Image lbImg, rbImg;
			SFT_Char c;
			Font font = latex.getSelectedFont();
			Color color = latex.getFontColor();

			double tempval = font.m_SFT.yScale;
			int ttt = exprImg.m_Height;

			sft_char(&(font.m_SFT), '{', &c);

			while (c.height < ttt) 
			{
				// uint_fast32_t outline;
				// outline_offset(font.m_SFT.font, '{', &outline);
				// y1 = geti16(sft->font, outline + 4);
				// y2 = geti16(sft->font, outline + 8);
				//font.m_SFT.yScale = exprImg.m_Height / (y2 + 1 - y1);
				font.m_SFT.yScale = (ttt / (c.height / (font.m_SFT.yScale / font.m_SFT.font->unitsPerEm)));
				sft_char(&(font.m_SFT), '{', &c);
			}

			lbImg.rasterizeCharacter(font, type == ARR_NORMAL ? '{' : '[', color);
			rbImg.rasterizeCharacter(font, type == ARR_NORMAL ? '}' : ']', color);

			font.setSize(tempval);

			tempImg = Image::concat(lbImg, exprImg);
			tempImg.concat(rbImg);
			break;
		}
		case ARR_TABULAR:
		{
			tempImg = exprImg;
			break;
		}
		default:
			break;
	}

	image.concat(tempImg);
};

void Handlers::rastRotate(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastRotate");

	Image tempImage;
	int degrees_num;
	std::string subexpr, degrees;

	degrees = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (degrees.length() == 0) return;

	subexpr = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (subexpr.length() == 0) return;

	if (degrees.find_first_not_of("-0123456789") != std::string::npos) return;

	degrees_num = std::stoi(degrees);

	tempImage = latex.toImage(subexpr);

	if (!tempImage.isEmpty())
	{
		tempImage.rotate((double)(degrees_num % 360));
		image.concat(tempImage);
	}
};

void Handlers::rastFrac(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastFrac");

	Image tempImage, numerImg, denomImg;
	std::string numer, denom;
	
	numer = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (numer.length() == 0) return;

	denom = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (denom.length() == 0) return;

	//lower size
	numerImg = latex.toImage(numer);
	denomImg = latex.toImage(denom);

	if (numerImg.isEmpty() || denomImg.isEmpty()) return;

	tempImage = Image::concat(numerImg, denomImg, ImagePosition::BOTTOM, 1);

	switch(type)
	{
		case FRAC_NORMAL:
		case FRAC_OVER:
			tempImage.drawLine(0, numerImg.m_Height - 1, tempImage.m_Width, numerImg.m_Height - 1, latex.getFontColor());
			break;
		case FRAC_ATOP:
			break;
		case FRAC_CHOOSE:
			// Add parenthesis around rasterized text
			break;
		default:
			break;
	}

	image.concat(tempImage);
}

void Handlers::rastOverlay(Latex& latex, std::string& expression, Image& image, SubFunctionType overlayType)
{
	PROFILE_SCOPE("Handlers::rastOverlay");

	auto overlayLine = [&latex, &expression, &image, &overlayType]() -> void
	{
		Image subImage1, subImage2;
		std::string subexpr;

		subexpr = Latex::getSubExpression(expression, 0, '{', '}', false);
		if (subexpr.length() == 0) return;

		subImage1 = latex.toImage(subexpr);

		switch (overlayType)
		{
			case OVERLAY_DIAG_LINE:
				subImage1.drawLine(0, subImage1.m_Height - 1, subImage1.m_Width - 1, 0, latex.getFontColor());
				break;
			case OVERLAY_HOR_LINE:
				subImage1.drawLine(0, subImage1.m_Height / 2, subImage1.m_Width, subImage1.m_Height / 2, latex.getFontColor());
				break;
			case OVERLAY_SLASH:
				break;
			default:
				break;
		}

		image.concat(subImage1);
	};

	auto overlayCompose = [&latex, &expression, &image]() mutable -> void
	{
		Image subImage1, subImage2;
		std::string subexpr1, subexpr2;

		subexpr1 = Latex::getSubExpression(expression, 0, '{', '}', false);
		if (subexpr1.length() == 0) return;

		subexpr2 = Latex::getSubExpression(expression, 0, '{', '}', false);
		if (subexpr2.length() == 0) return;

		subImage1 = latex.toImage(subexpr1);
		subImage2 = latex.toImage(subexpr2);

		Details subDetails1 = subImage1.getDetails();
		Details subDetails2 = subImage2.getDetails();

		Image tempImage(
			subDetails1.width > subDetails2.width ? subDetails1.width : subDetails2.width, 
			subDetails1.height > subDetails2.height ? subDetails1.height : subDetails2.height, 
			subDetails1.channels > subDetails2.channels ? subDetails1.channels : subDetails2.channels
		);

		int baseline = subDetails1.baseline > subDetails2.baseline ? subDetails1.baseline : subDetails2.baseline;

		tempImage.overlay(subImage1, 0, baseline > 0 ? baseline - subDetails1.baseline : 0);
		tempImage.overlay(subImage2, 0, baseline > 0 ? baseline - subDetails2.baseline : 0);

		image.concat(tempImage);
	};

	switch (overlayType)
	{
		case OVERLAY_SLASH:
		case OVERLAY_DIAG_LINE:
		case OVERLAY_HOR_LINE:
			overlayLine();
			break;
		case OVERLAY_COMPOSE:
			overlayCompose();
			break;
		default:
			break;
	}
}

void Handlers::rastSqrt(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastSqrt");
	//! how.
	return;
}

void Handlers::rastEval(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastEval");

	std::string subexpr;
	int subexpr_int;
	std::function<char(void)> get, peek;
	std::function<int(void)> expr, term, factor, number;

	peek = [&subexpr]() -> char 
	{ 
		return subexpr[0]; 
	};

	get = [&subexpr]() -> char 
	{ 
		char result = subexpr[0]; 
		subexpr.erase(0, 1); 
		return result; 
	};

	number = [&get, &peek]() -> int 
	{
		int result = get() - '0';
		while (peek() >= '0' && peek() <= '9') 
			result = 10*result + get() - '0'; 
		return result; 
	};

	factor = [&get, &peek, &number, &factor, &expr]() -> int 
	{
		if (peek() >= '0' && peek() <= '9')
        	return number();
		else if (peek() == '(')
		{
			get(); // '('
			int result = expr();
			get(); // ')'
			return result;
		}
		else if (peek() == '-')
		{
			get();
			return -factor();
		}
		return 0; // error
	};

	term = [&peek, &get, &factor]() -> int
	{
		int result = factor();
		char op;
		while (peek() == '*' || peek() == '/' || peek() == '^' || peek() == '%')
		{
			op = get();
			if (op == '*')
				result *= factor();
			else if (op == '/')
				result /= factor();
			else if (op == '^')
				result = (int)std::pow((double)result, factor());
			else
				result = result % factor();
		}
		return result;
	};

	expr = [&peek, &get, &term]() -> int
	{
		int result = term();
		while (peek() == '+' || peek() == '-')
			if (get() == '+')
				result += term();
			else
				result -= term();
		return result;
	};

	subexpr = Latex::getSubExpression(expression, 0, '(', ')', false);
	if (subexpr.length() == 0)
		return;
	
	subexpr_int = expr();
	expression.insert(0, std::to_string(subexpr_int));
}

void Handlers::rastToday(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastToday");
	
	std::string subexpr;
	char text[128];
	tm tmstruct;
	Image tempImg;

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

	time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	localtime_s(&tmstruct, &time);

	sprintf(text, "%s, %s %d, %d", dayNames[tmstruct.tm_wday], monthNames[tmstruct.tm_mon], tmstruct.tm_mday, tmstruct.tm_year + 1900);
	subexpr = text;

	tempImg = latex.toImage(subexpr);

	image.concat(tempImg);
}

void Handlers::rastPicture(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastPicture");

	int width, height;
	size_t i;
	std::string subexpr, temp;

	subexpr = Latex::getSubExpression(expression, 0, '(', ')', false);
	if (subexpr.find_first_not_of("-0123456789,") != std::string::npos) return;
	width = std::stoi(subexpr.substr(0, subexpr.find(",")));
	height = std::stoi(subexpr.substr(subexpr.find(",") + 1));

	Image tempImg = Image(width, height, 4);
	subexpr = Latex::getSubExpression(expression, 0, '{', '}', false);

	while (subexpr.length() != 0)
	{
		for (i = 0; i < subexpr.length(); ++i)
		{
			if (subexpr[i] != '(') 
			{
				i++;
				continue;
			};

			temp = Latex::getSubExpression(subexpr, 0, '(', ')', false);

			if (temp.find_first_not_of("-0123456789,") != std::string::npos) return;
			
			width = std::stoi(temp.substr(0, temp.find(",")));
			height = std::stoi(temp.substr(temp.find(",") + 1));

			temp = Latex::getSubExpression(subexpr, 0, '{', '}', false);
			tempImg.overlay(latex.toImage(temp), width, height);

			break;
		}
		if (i > 0)
			subexpr.erase(0, i + 1);
	}

	image.concat(tempImg);
}

void Handlers::rastAccent(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastAccent");

	return;
}

void Handlers::rastMathFunc(Latex& latex, std::string& expression, Image& image, SubFunctionType funcType)
{
	PROFILE_SCOPE("Handlers::rastMathFunc");

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

	expression.insert(0, mathFuncNames[funcType - 399]);
}

void Handlers::rastGRChar(Latex& latex, std::string& expression, Image& image, SubFunctionType charCode)
{
	PROFILE_SCOPE("Handlers::rastGRChar");

	Image tempImage;

	if (charCode != 0)
	{
		tempImage.rasterizeCharacter(latex.getSelectedFont(), charCode, latex.getFontColor());

		if (expression[0] == '_' || expression[0] == '^')
			Handlers::rastScripts(latex, expression, tempImage, NONE);

		image.concat(tempImage);
	}
}

void Handlers::rastBezier(Latex& latex, std::string& expression, Image& image, SubFunctionType bezierType)
{
	PROFILE_SCOPE("Handlers::rastBezier");

	auto getPoint = [](int n1, int n2, float perc) -> int
	{
		int diff = n2 - n1;

		return n1 + (diff * perc);
	};

	// can't understand why quadractic has 3 points, and cubic 4 points, like wth
	Image tempImage;
	uint8_t* dstPx;
	Color color = latex.getFontColor();

	switch (bezierType)
	{
		case BEZIER_QUADRATIC:
		{
			std::string coord1, coord2, coord3;
			int x1, x2, x3, y1, y2, y3, xa, ya, xb, yb, x, y;
			float i;

			coord1 = Latex::getSubExpression(expression, 0, '(', ')', false);
			if (coord1.length() == 0 || coord1.find_first_not_of("-0123456789,") != std::string::npos) return;

			coord2 = Latex::getSubExpression(expression, 0, '(', ')', false);
			if (coord2.length() == 0 || coord2.find_first_not_of("-0123456789,") != std::string::npos) return;

			coord3 = Latex::getSubExpression(expression, 0, '(', ')', false);
			if (coord3.length() == 0 || coord3.find_first_not_of("-0123456789,") != std::string::npos) return;

			x1 = std::stoi(coord1.substr(0, coord1.find(",")));
			y1 = std::stoi(coord1.substr(coord1.find(",") + 1));

			x2 = std::stoi(coord2.substr(0, coord2.find(",")));
			y2 = std::stoi(coord2.substr(coord2.find(",") + 1));

			x3 = std::stoi(coord3.substr(0, coord3.find(",")));
			y3 = std::stoi(coord3.substr(coord3.find(",") + 1));

			tempImage = Image(std::max<int>(x1, std::max<int>(x2, x3)), std::max<int>(y1, std::max<int>(y2, y3)), 4);

			for (i = 0; i < 1; i += 0.01)
			{
				xa = getPoint(x1, x2, i);
				ya = getPoint(y1, y2, i);
				xb = getPoint(x2, x3, i);
				yb = getPoint(y2, y3, i);

				x = getPoint(xa, xb, i);
				y = getPoint(ya, yb, i);

				dstPx = &tempImage.m_Data[(x + y * tempImage.m_Width) * tempImage.m_Channels];
				
				for (int j = 0; j < tempImage.m_Channels; ++j)
					dstPx[j] = color[j];
			}

			break;
		}
		case BEZIER_CUBIC:
		{
			std::string coord1, coord2, coord3, coord4;
			int x1, x2, x3, x4, y1, y2, y3, y4, xa, ya, xb, yb, xc, yc, xm, ym, xn, yn, x, y;
			float i;

			coord1 = Latex::getSubExpression(expression, 0, '(', ')', false);
			if (coord1.length() == 0 || coord1.find_first_not_of("0123456789,") != std::string::npos) return;

			coord2 = Latex::getSubExpression(expression, 0, '(', ')', false);
			if (coord2.length() == 0 || coord2.find_first_not_of("0123456789,") != std::string::npos) return;

			coord3 = Latex::getSubExpression(expression, 0, '(', ')', false);
			if (coord3.length() == 0 || coord3.find_first_not_of("0123456789,") != std::string::npos) return;

			coord4 = Latex::getSubExpression(expression, 0, '(', ')', false);
			if (coord4.length() == 0 || coord4.find_first_not_of("0123456789,") != std::string::npos) return;

			x1 = std::stoi(coord1.substr(0, coord1.find(",")));
			y1 = std::stoi(coord1.substr(coord1.find(",") + 1));

			x2 = std::stoi(coord2.substr(0, coord2.find(",")));
			y2 = std::stoi(coord2.substr(coord2.find(",") + 1));

			x3 = std::stoi(coord3.substr(0, coord3.find(",")));
			y3 = std::stoi(coord3.substr(coord3.find(",") + 1));

			x4 = std::stoi(coord4.substr(0, coord4.find(",")));
			y4 = std::stoi(coord4.substr(coord4.find(",") + 1));

			tempImage = Image(std::max<int>(x1, std::max<int>(x2, std::max<int>(x3, x4))), std::max<int>(y1, std::max<int>(y2, std::max<int>(y3, y4))), 4);

			for (i = 0; i < 1; i += 0.01)
			{
				xa = getPoint(x1, x2, i);
				ya = getPoint(y1, y2, i);
				xb = getPoint(x2, x3, i);
				yb = getPoint(y2, y3, i);
				xc = getPoint(x3, x4, i);
				yc = getPoint(y3, y4, i);

				xm = getPoint(xa, xb, i);
				ym = getPoint(ya, yb, i);
				xn = getPoint(xb, xc, i);
				yn = getPoint(yb, yc, i);

				x = getPoint(xm, xn, i);
				y = getPoint(ym, yn, i);

				dstPx = &tempImage.m_Data[(x + y * tempImage.m_Width) * tempImage.m_Channels];

				for (int j = 0; j < tempImage.m_Channels; ++j)
					dstPx[j] = color[j];
			}

			break;
		}
		default:
			break;
	}

	image.concat(tempImage);
}

void Handlers::rastMagnify(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastMagnify");

	Image tempImage;
	std::string magnifier, subexpression;
	int magnifier_num;

	magnifier = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (magnifier.length() == 0 || magnifier.find_first_not_of("0123456789") != std::string::npos) return;

	subexpression = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (subexpression.length() == 0) return;

	magnifier_num = std::stoi(magnifier);

	tempImage = latex.toImage(subexpression);

	tempImage.scaleUp(magnifier_num);
	
	image.concat(tempImage);
}

void Handlers::rastFBox(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastFBox");

	return; //!!!
	// get width and height
	// parse to int
	// get subexpression
	// create box
	// draw border
	// rasterize subexpression
}

void Handlers::rastArrow(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastArrow");

	return;
}

void Handlers::rastLine(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastLine");

	Image tempImage;
	int x1, x0, y1, y0;
	Color color = latex.getFontColor();

	std::string pos1 = Latex::getSubExpression(expression, 0, '(', ')', false);
	if (pos1.length() == 0 || pos1.find_first_not_of(",0123456789") != std::string::npos) return;

	std::string pos2 = Latex::getSubExpression(expression, 0, '(', ')', false);
	if (pos2.length() == 0 || pos2.find_first_not_of(",0123456789") != std::string::npos) return;

	x0 = std::stoi(pos1.substr(0,pos1.find(",")));
	y0 = std::stoi(pos1.substr(pos1.find(",") + 1));

	x1 = std::stoi(pos2.substr(0,pos2.find(",")));
	y1 = std::stoi(pos2.substr(pos2.find(",") + 1));

	tempImage = Image(std::max<int>(x0, x1) + 1, std::max<int>(y0, y1) + 1, 4);

	tempImage.drawLine(x0, y0, x1, y1, color);

	image.concat(tempImage);
}

void Handlers::rastReflect(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastReflect");

	Image tempImage;
	std::string axis, subexpr;

	axis = Latex::getSubExpression(expression, 0, '[', ']', false);
	if (axis.length() == 0 && axis.length() > 1) return;

	subexpr = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (subexpr.length() == 0) return;

	tempImage = latex.toImage(subexpr);

	switch (axis[0])
	{
		case 'x':
			tempImage.flip(AXIS::X);
			break;
		case 'y':
			tempImage.flip(AXIS::Y);
			break;
		default:
			break;
	}

	image.concat(tempImage);
}