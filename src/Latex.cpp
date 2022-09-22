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

	int i, j;
	static bool isFirstIteration = false;
	SubFunction subFunction;

	if (isFirstIteration == false) 
	{
		prepExpression(expression); //prepare expression to find unsupported subfunctions and remove unnecessary braces, if found
		isFirstIteration = true;
	}

	#ifdef DEBUG
		printf("[Latex::toImage] start expression: %s\n", expression.c_str());
	#endif

	while (expression.length() > 0)
	{
		for (i = 0; i < expression.length(); ++i)
		{
			//if expression[i] == '{' and expression[i-1] != '\\' then
			//find '}', check if '}' not escaped
			//if '}' not found, add it to the end of expression
			//cut expression between '{' and '}'
			//remove '{' and '}'
			//parse it to toImage() again
			//concat with finalImage
			if (expression[i] == '_' || expression[i] == '^')
			{
				Handlers::rastScripts(*this, expression, finalImage, NONE);
				break;
			}
			else if (expression[i] == '{')
			{
				size_t rightBracketIndex;
				std::string subexpression;

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
				break;
			}
			else
			{
				finalImage.rasterizeCharacter(expression[i], *p_SelectedFont, p_Color);
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
	PROFILE_SCOPE("Latex::prepExpression");

	if (expression.length() == 0)
		return;

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
	for (int i = 0; i < expression.length(); i++)
	{
		if (expression[i] == '{') // found open bracket
			brackets.push(expression[i]); // push it to stack

		if (expression[i] == '}') // found close bracket
			if (brackets.size() > 0) // if stack isn't empty
				brackets.pop(); // remove open bracket from stack
			else
				expression.erase(i, 1); // erase unfixed close bracket
	}

	if (!brackets.empty())
		for (int i = 0; i < brackets.size(); i++)
			expression.append("}"); //add close bracket for every unfixed open bracket

	//Erase spaces

	//Convert \\left( to \\( and \\right) to \\)
	// while ( )
};

Scripts Latex::texScripts(std::string& expression, ScriptType which)
{
	bool gotSub = false, gotSup = false;
	std::string subScript, supScript;

	while (expression.length() > 0)
	{
		printf("%d\n", expression.length());
		if (expression[0] == '_' && (which == 1 || which == 3))
		{
			if (!gotSub)
			{
				expression.erase(0, 1);
				gotSub = true;
				subScript = Latex::getSubExpression(expression, 0, '{', '}', false);
			}
		}
		else if (expression[0] == '^' && (which == 2 || which == 3))
		{
			if (!gotSup)
			{
				expression.erase(0, 1);
				gotSup = true;
				supScript = Latex::getSubExpression(expression, 0, '{', '}', false);
			}
		}
		else
			return {subScript, supScript};
	}
	return {subScript, supScript};
};

std::string Latex::getSubExpression(std::string& expression, int from, const char left, const char right, bool returnDelims)
{
	PROFILE_SCOPE("Latex::getSubExpression");

	unsigned first, last;
	std::string result;
	std::stack<char> brackets;

	if (expression[from] != left) // if expression not starts from left delimeter
	{
		result = expression[0];
		expression.erase(0, 1);
		return result; //just return first char
	}

	first = expression.find(left, from); //find index of left delimeter
	if (first > from) return NULL;

	for (int i = 0; i < expression.length(); i++) //find index of right delimeter
	{
		if (expression[i] == left)
			brackets.push(expression[i]);
		if (expression[i] == right)
			if (brackets.size() > 1)
				brackets.pop();
			else
			{
				last = i;
				break;
			}
	}

	while (expression[last - 1] == '\\') //if delimeter is escaped
		last = expression.find(right, last+1); //find right delimeter again

	result = returnDelims ? expression.substr(first, last - first + 1) : expression.substr(first + 1, last - first - 1);
	expression.erase(0, returnDelims ? result.length() : result.length() + 2);

	return result;
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

void Latex::setFont(FontType type, const char* pathToFont, uint16_t size) 
{ 
	switch(type)
	{
		case FontType::Normal:
			m_NormalFont.setFont(pathToFont);
			m_NormalFont.setSize(size);
			break;
		case FontType::Italic:
			m_ItalicFont.setFont(pathToFont);
			m_ItalicFont.setSize(size);
			break;
		case FontType::Bold:
			m_BoldFont.setFont(pathToFont);
			m_BoldFont.setSize(size);
			break;
		case FontType::BoldItalic:
			m_BoldItalicFont.setFont(pathToFont);
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
	image.concat(latex.toImage(expression), ImagePosition::BOTTOM);
};

void Handlers::rastColor(Latex& latex, std::string& expression, Image& image, SubFunctionType colorType)
{

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

			expression.erase(0, expression[0] == '{' ? arg.length() + 2 : arg.length());
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
		}
		
		latex.setFontColor(hexToRGBA(hex));

		if (expression[0] == '~') //color all text after subfunction 
		{
			arg = expression.substr(1);
			expression.erase(0);
			if (arg.length() == 0) return;

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
		image.concat(tempImage);
};

void Handlers::rastRaise(Latex& latex, std::string& expression, Image& image, SubFunctionType arg1)
{
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
		image.concat(tempImage);
	}
};

void Handlers::rastText(Latex& latex, std::string& expression, Image& image, SubFunctionType textType)
{
	//? Maybe add another array of subfunction-like letters (i.e. {"\\alpha", NULL, letterCode})
	
	return;
	Image tempImage;
	Letter* letter;
	std::string arg;

	arg = Latex::getSubExpression(expression, 0, '{', '}', false);
	if (arg.length() == 0)
		return;

	for (int i = 0; i < arg.length(); ++i){
		switch (textType)
		{
			case TEXT_CYR:
				// Search for letter
				// Assign letter from table to letter pointer
				break;
			case TEXT_GREEK:
				// Search for letter
				// Assign letter from table to letter pointer
				break;
		}
		tempImage.rasterizeCharacter(letter->charCode, latex.getSelectedFont(), latex.getFontColor());
	}

	image.concat(tempImage);
};

void Handlers::rastSetFont(Latex& latex, std::string& expression, Image& image, SubFunctionType fontType)
{
	// get font type
	// get subexpression
	// set font type
	// if open brace after font type is not present ('~' is present)
		// rasterize the rest of expression
	// else
		// get subexpression between braces
		// rasterize subexpression
		// set previous font type
	// if rasterized image is not empty
		// concat with passed image
};

void Handlers::rastScripts(Latex& latex, std::string& expression, Image& image, SubFunctionType type)
{
	Image subImg, supImg;
	Details subDetails, supDetails;
	int newWidth, newHeight, newBaseline;
	bool isSub, isSup;
	Scripts scripts;
	Font& font = latex.getSelectedFont();

	if (expression.length() == 0)
		return;

	scripts = latex.texScripts(expression, ScriptType::BOTH);
	
	if (scripts.subScript.length() != 0)
	{
		isSub = true;
		font.setSize(font.m_SFT.xScale - 5);
		subImg = latex.toImage(scripts.subScript); // implement proper size changer
		font.setSize(font.m_SFT.xScale + 5);
	}
	if (scripts.supScript.length() != 0)
	{
		isSup = true;
		font.setSize(font.m_SFT.xScale - 5);
		supImg = latex.toImage(scripts.supScript);
		font.setSize(font.m_SFT.xScale + 5);
	}

	if (!isSub && !isSup)
		return;

	if (isSub)
		subDetails = subImg.getDetails();

	if (isSup)
		supDetails = supImg.getDetails();

	newWidth = subDetails.width > supDetails.width ? subDetails.width : supDetails.width;
	newHeight = (image.m_Height - image.m_AdvanceHeight) + subDetails.height + supDetails.height;

	Image tempImg(newWidth, newHeight, 4);
	tempImg.m_Baseline = newHeight - subDetails.height;
	if (subDetails.height > 0) tempImg.m_AdvanceHeight += subDetails.height;

	tempImg.overlay(supImg, 0, 0);
	tempImg.overlay(subImg, 0, newHeight - subDetails.height);

	image.concat(tempImg);
};