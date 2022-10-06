#include "Latex.hpp"

Latex::Latex(WarningBehavior behavior): p_WarningBehavior(behavior), p_SelectedFont(&m_NormalFont) { };

Latex::~Latex() { };

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

	// Font font(p_NormalFont, 50); //Should be initialized when rasterization is needed

	Image finalImage;
	size_t i, j;
	static bool isFirstIteration = false;
	SubFunction subFunction;

	#ifdef DEBUG
		printf("[Latex::toImage] start expression: %s\n", expression.c_str());
	#endif

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
				tempImage.rasterizeCharacter(expression[i], *p_SelectedFont, p_Color);
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

	const char* comment = "%%";
	std::stack<char> brackets;
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
	for (size_t i = 0; i < expression.length(); i++)
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

	//Erase spaces

	//Convert \\left( to \\( and \\right) to \\)
	// while ( )
};

Scripts Latex::texScripts(std::string& expression, ScriptType which)
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

SubFunction Latex::getSubFunction(const std::string& expression, size_t at)
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

void Latex::setFont(FontType type, const char* pathToFont, uint16_t size) 
{ 
	switch(type)
	{
		case FontType::Normal:
			m_NormalFont.setFont(pathToFont);
			m_NormalFont.type = FontType::Normal;
			m_NormalFont.setSize(size);
			break;
		case FontType::Italic:
			m_ItalicFont.setFont(pathToFont);
			m_ItalicFont.type = FontType::Italic;
			m_ItalicFont.setSize(size);
			break;
		case FontType::Bold:
			m_BoldFont.setFont(pathToFont);
			m_BoldFont.type = FontType::Bold;
			m_BoldFont.setSize(size);
			break;
		case FontType::BoldItalic:
			m_BoldItalicFont.setFont(pathToFont);
			m_BoldItalicFont.type = FontType::BoldItalic;
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

void Handlers::rastNewline(Latex& latex, std::string& expression, Image& image, SubFunctionType arg1)
{
	if (image.isEmpty()) return;

	image.concat(latex.toImage(expression), ImagePosition::BOTTOM);
};

void Handlers::rastColor(Latex& latex, std::string& expression, Image& image, SubFunctionType colorType)
{
	PROFILE_SCOPE("Handlers::rastColor");

	Image tempImage;

	//?if called, should set color temporarily or permanently, if 1st char is "~"
	//!create temporary color handler

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
		
		switch(colorType)
		{
			case COLOR_CUSTOM:
				hex = Latex::getSubExpression(expression, 0, '{', '}', false);
				if (hex.length() == 0) return;
				break;
			case COLOR_RED:
				hex = "ff0000";
				break;
			case COLOR_GREEN:
				hex = "00ff00";
				break;
			case COLOR_BLUE:
				hex = "0000ff";
				break;
			case COLOR_BLACK:
				hex = "000000";
				break;
			case COLOR_WHITE:
				hex = "ffffff";
				break;
			default:
				break;
		}
		
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

	if (!tempImage.isEmpty())
	{
		if (!image.isEmpty())
			image.concat(tempImage);
		else
			image = tempImage;
	}
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
		if (lift_num < 0) tempImage.m_AdvanceHeight -= lift_num;

		if (!image.isEmpty())
			image.concat(tempImage);
		else
			image = tempImage;
	}
};

void Handlers::rastText(Latex& latex, std::string& expression, Image& image, SubFunctionType textType)
{
	//? Maybe add another array of subfunction-like letters (i.e. {"\\alpha", NULL, letterCode})

	auto findLetter = [](Letter* table, std::string& expr) -> Letter*
	{
		size_t i, match;
		Letter* letter;

		for (i = 0; table[i].character != NULL; ++i)
		{
			match = expr.find(table[i].character, 0);

			if (match != std::string::npos && match == 0)
				letter = &table[i];
		};

		return letter;
	};

	Image tempImage;
	Letter* letter;
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
				letter = findLetter(miniGreekTable, subexpression);
				break;
			default:
				break;
		}

		subexpression.erase(0, strlen(letter->character));
		tempImage.rasterizeCharacter(letter->charCode, latex.getSelectedFont(), latex.getFontColor());
	}

	if (expression[0] == '_' || expression[0] == '^')
		Handlers::rastScripts(latex, expression, tempImage, NONE);

	if (!image.isEmpty())
		image.concat(tempImage);
	else
		image = tempImage;
};

void Handlers::rastSetWeight(Latex& latex, std::string& expression, Image& image, SubFunctionType fontType)
{
	PROFILE_SCOPE("Handlers::rastSetWeight");

	Image tempImage;
	FontType type;
	std::string subexpression;

	type = latex.getSelectedFont().type;

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

	if (!tempImage.isEmpty())
	{
		if (!image.isEmpty())
			image.concat(tempImage);
		else
			image = tempImage;
	}
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

	scripts = latex.texScripts(expression, ScriptType::BOTH);
	

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

	if (!image.isEmpty())
		image.concat(tempImg);
	else
		image = tempImg;
};

void Handlers::rastArray(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	PROFILE_SCOPE("Handlers::rastArray");

	// search for subexpression between brackets
	// rasterize it
	// rasterize left bracket the same height as rasterized expression
	//? create bracket rasterizer with some height/width parameters
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

			double tempval = latex.getSelectedFont().m_SFT.yScale;
			int ttt = exprImg.m_Height;

			sft_char(&(latex.getSelectedFont().m_SFT), '{', &c);

			while (c.height < ttt) 
			{
				latex.getSelectedFont().m_SFT.yScale = (ttt / (c.height / latex.getSelectedFont().m_SFT.yScale));
				sft_char(&(latex.getSelectedFont().m_SFT), '{', &c);
			}

			lbImg.rasterizeCharacter(type == ARR_NORMAL ? '{' : '[', latex.getSelectedFont(), latex.getFontColor());
			rbImg.rasterizeCharacter(type == ARR_NORMAL ? '}' : ']', latex.getSelectedFont(), latex.getFontColor());

			latex.getSelectedFont().setSize(tempval);

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

	if (!tempImg.isEmpty())
	{
		if (!image.isEmpty())
			image.concat(tempImg);
		else
			image = tempImg;
	}
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

	//add space between images
	tempImage = Image::concat(numerImg, denomImg, ImagePosition::BOTTOM);

	//get y coordinate to draw a horizontal line on that line
	// if type == FRAC_NORMAL or type == FRAC_OVER
	//draw a horizontal line
	// if type == FRAC_ATOP
	//leave as is
	// if type == FRAC_CHOOSE
	//leave as is and put parenthesis around expression
	switch(type)
	{
		case FRAC_NORMAL:
		case FRAC_OVER:
			tempImage.drawLine(0, numerImg.m_Height - 1, tempImage.m_Width, numerImg.m_Height - 1, latex.getFontColor());
			break;
		case FRAC_ATOP:
			break;
		case FRAC_CHOOSE:
			break;
		default:
			break;
	}

	if (!tempImage.isEmpty())
	{
		if (!image.isEmpty())
			image.concat(tempImage);
		else
			image = tempImage;
	}
}

void Handlers::rastOverlay(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	// get subexpr1 and subexpr2
	// rasterize
	// overlay subexpr2Img onto subexpr1Img
	// concat result to image
	return;
}

void Handlers::rastSqrt(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
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
	time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	localtime_s(&tmstruct, &time);

	sprintf(text, "%s, %s %d, %d", dayNames[tmstruct.tm_wday], monthNames[tmstruct.tm_mon], tmstruct.tm_mday, tmstruct.tm_year + 1900);
	subexpr = text;

	tempImg = latex.toImage(subexpr);

	if (!tempImg.isEmpty())
	{
		if (!image.isEmpty())
			image.concat(tempImg);
		else
			image = tempImg;
	}
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

	if (!tempImg.isEmpty())
	{
		if (!image.isEmpty())
			image.concat(tempImg);
		else
			image = tempImg;
	}
}

void Handlers::rastAccent(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	return;
}

void Handlers::rastMathFunc(Latex& latex, std::string& expression, Image& image, SubFunctionType funcType)
{
	expression.insert(0, mathFuncNames[funcType - 19]);
}