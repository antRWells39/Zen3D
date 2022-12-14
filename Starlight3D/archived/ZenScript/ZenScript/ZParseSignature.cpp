#include "ZParseSignature.h"
#include "ZParseValue.h"
#include "ZParseSigParam.h"
#include "ZSigParamNode.h"
#include "ZSignatureNode.h"
#include <assert.h>

ZParseSignature::ZParseSignature(ZTokenStream* stream) : ZParseNode(stream) {



}


ZScriptNode* ZParseSignature::Parse() {

	auto check = mStream->NextToken();

	if (check.mType != TokenType::TokenLeftPara)
	{
		///*ERROR*
		assert(false);
	}

	auto sig_node = new ZSignatureNode;

	while (!mStream->EOS()) {

		auto toke = mStream->NextToken();

		ZParseSigParam* parse_par = nullptr;// new ZParseSigParam(mStream);
		ZSigParamNode* par_node = nullptr;

		switch (toke.mType)
		{
		case TokenType::TokenRightPara:
			return sig_node;
			break;
		case TokenType::TokenComma:
			continue;
			break;
		case TokenType::TokenInt:

			parse_par = new ZParseSigParam(mStream);
			mStream->Back();
			par_node = (ZSigParamNode*)parse_par->Parse();

			sig_node->AddParameter(par_node);

			break;
		case TokenType::TokenFloat:

			break;
		case TokenType::TokenString:

			break;
		default:
			assert(false);
			break;
		}
		
		int aa = 5;


	}

	//int aa = 5;


	return nullptr;
}