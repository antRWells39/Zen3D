#include "ZTokenizer.h"
#include <map>

ZTokenizer::ZTokenizer(ZSource* source)
{

	auto code = source->GetCode();

	for (int i = 0; i < code.size(); i++) {

		//std::cout << "Source:" << code[i] << std::endl;

	}

	mCode = code;

}

bool vec_contains(std::vector<char> vec, char look)
{

	for (int i = 0; i < vec.size(); i++) {

		if (vec[i] == look) {
			return true;
		}

	}
	return false;

}

ZTokenStream* ZTokenizer::Tokenize() {

	std::vector<char> is_text;
	std::vector<char> is_op;
	std::vector<char> is_num;

	is_text.push_back("a"[0]);
	is_text.push_back("b"[0]);
	is_text.push_back("c"[0]);
	is_text.push_back("d"[0]);
	is_text.push_back("e"[0]);
	is_text.push_back("f"[0]);
	is_text.push_back("g"[0]);
	is_text.push_back("h"[0]);
	is_text.push_back("i"[0]);
	is_text.push_back("j"[0]);
	is_text.push_back("k"[0]);
	is_text.push_back("l"[0]);
	is_text.push_back("m"[0]);
	is_text.push_back("n"[0]);
	is_text.push_back("o"[0]);
	is_text.push_back("p"[0]);
	is_text.push_back("q"[0]);
	is_text.push_back("r"[0]);
	is_text.push_back("s"[0]);
	is_text.push_back("t"[0]);
	is_text.push_back("u"[0]);
	is_text.push_back("v"[0]);
	is_text.push_back("w"[0]);
	is_text.push_back("x"[0]);
	is_text.push_back("y"[0]);
	is_text.push_back("z"[0]);

	is_text.push_back("A"[0]);
	is_text.push_back("B"[0]);
	is_text.push_back("C"[0]);
	is_text.push_back("D"[0]);
	is_text.push_back("E"[0]);
	is_text.push_back("F"[0]);
	is_text.push_back("G"[0]);
	is_text.push_back("H"[0]);
	is_text.push_back("I"[0]);
	is_text.push_back("J"[0]);
	is_text.push_back("K"[0]);
	is_text.push_back("L"[0]);
	is_text.push_back("M"[0]);
	is_text.push_back("N"[0]);
	is_text.push_back("O"[0]);
	is_text.push_back("P"[0]);
	is_text.push_back("Q"[0]);
	is_text.push_back("R"[0]);
	is_text.push_back("S"[0]);
	is_text.push_back("T"[0]);
	is_text.push_back("U"[0]);
	is_text.push_back("V"[0]);
	is_text.push_back("W"[0]);
	is_text.push_back("X"[0]);
	is_text.push_back("Y"[0]);
	is_text.push_back("Z"[0]);
	
	//Operators
	is_op.push_back("("[0]);
	is_op.push_back(")"[0]);
	is_op.push_back(","[0]);
	is_op.push_back("."[0]);
	is_op.push_back("+"[0]);
	is_op.push_back("-"[0]);
	is_op.push_back("*"[0]);
	is_op.push_back("/"[0]);
	is_op.push_back("="[0]);




	is_num.push_back("0"[0]);
	is_num.push_back("1"[0]);
	is_num.push_back("2"[0]);
	is_num.push_back("3"[0]);
	is_num.push_back("4"[0]);
	is_num.push_back("5"[0]);
	is_num.push_back("6"[0]);
	is_num.push_back("7"[0]);
	is_num.push_back("8"[0]);
	is_num.push_back("9"[0]);

	std::vector<Token> tokens;

	int ctoks = 0;

	for (int i = 0; i < mCode.size(); i++) {

		auto line = mCode[i];

		std::string cur_token = "";
		bool is_string = false;
		bool is_number = false;
		bool is_float = false;

		int s_tok = 0;

		for (int c = 0; c < line.size(); c++) {

			auto ch = line[c];

			if (is_number) {

				if (vec_contains(is_num, ch)) {
					cur_token = cur_token + ch;
					continue;
				}
				else if (ch == "."[0])
				{
					cur_token = cur_token + ch;
					is_float = true;
					continue;
				}
				else {
					if (is_float) {
						tokens.push_back(Token(TokenType::TokenFloat, cur_token));
						s_tok++;
					}
					else {
						tokens.push_back(Token(TokenType::TokenInt, cur_token));
						s_tok++;
					}
					cur_token = "";
					is_number = false;
					is_float = false;
				}

			}

			if (is_string) {

				if (ch == "\""[0])
				{
					is_string = false;
					tokens.push_back(Token(TokenType::TokenString, cur_token));
					cur_token = "";
					s_tok++;
					continue;
				}
				else {
					cur_token = cur_token + ch;
					continue;
				}


			}

			if (vec_contains(is_text, ch))
			{
				cur_token = cur_token + ch;
			}
			else {

				if (cur_token.size() > 0)
				{

					if (vec_contains(is_num, ch)) {
						cur_token = cur_token + ch;
						continue;
					}
					tokens.push_back(Token(TokenType::TokenIdent, cur_token));
					s_tok++;
					cur_token = "";
				}

				if (vec_contains(is_op, ch))
				{

					std::string op = "";
					op = op + ch;
					tokens.push_back(Token(TokenType::TokenOperator,op));
					s_tok++;

				}
				else if (ch == "\""[0])
				{
					is_string = true;

				}
				else if (vec_contains(is_num, ch)) {
					is_number = true;
					cur_token = ch;
					continue;
				}

			}


		}

		if (cur_token.size() > 0) {

			if (is_string) {
				tokens.push_back(Token(TokenType::TokenString, cur_token));
				s_tok++;
			}
			else if (is_number) {
				if (is_float) {
					tokens.push_back(Token(TokenType::TokenFloat, cur_token));
					s_tok++;
				}
				else {
					tokens.push_back(Token(TokenType::TokenInt, cur_token));
					s_tok++;
				}
				}
			else {
				tokens.push_back(Token(TokenType::TokenIdent, cur_token));
				s_tok++;
			}

		}

		if (s_tok > 0) {
			tokens.push_back(Token(TokenType::TokenEndOfLine, ";"));
		}
	}



	std::cout << "Token Stream output." << std::endl;

	for (int i = 0; i < tokens.size(); i++)
	{

		std::cout << "Token:" << TokenToString(tokens[i].mType) << "  Text:" << tokens[i].mText << std::endl;

	    

	}

	std::map<std::string, TokenType> token_map;

	token_map.insert(std::make_pair("class", TokenType::TokenClass));
	token_map.insert(std::make_pair("method", TokenType::TokenMethod));
	token_map.insert(std::make_pair("int", TokenType::TokenInt));
	token_map.insert(std::make_pair("float", TokenType::TokenFloat));
	token_map.insert(std::make_pair("string", TokenType::TokenString));
	token_map.insert(std::make_pair("function", TokenType::TokenFunction));
	token_map.insert(std::make_pair("end", TokenType::TokenEnd));
	token_map.insert(std::make_pair("new", TokenType::TokenNew));
	token_map.insert(std::make_pair("void", TokenType::TokenVoid));
	token_map.insert(std::make_pair(".", TokenType::TokenPeriod));
	token_map.insert(std::make_pair(",", TokenType::TokenComma));
	token_map.insert(std::make_pair("(", TokenType::TokenLeftPara));
	token_map.insert(std::make_pair(")", TokenType::TokenRightPara));
	token_map.insert(std::make_pair("+", TokenType::TokenPlus));;
	token_map.insert(std::make_pair("-", TokenType::TokenMinus));;
	token_map.insert(std::make_pair("/", TokenType::TokenDivide));;
	token_map.insert(std::make_pair("*", TokenType::TokenMultiply));;
	token_map.insert(std::make_pair("=", TokenType::TokenEquals));;
	std::vector<Token> new_tokens;

	for (int i = 0; i < tokens.size(); i++) {

		auto old_token = tokens[i];

		Token new_token = Token(old_token.mType, old_token.mText);

		if (token_map.find(old_token.mText) != token_map.end()) {
			new_token.mType = token_map[old_token.mText];
		}
		else {
			
		}

		new_tokens.push_back(new_token);

	}

	auto tok_stream = new ZTokenStream();
	tok_stream->SetTokens(new_tokens);

	return tok_stream;

}