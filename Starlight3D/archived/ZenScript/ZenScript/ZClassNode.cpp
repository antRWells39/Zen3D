#include "ZClassNode.h"
#include "ZMethodNode.h"
#include "ZContextScope.h"
#include "ZVarsNode.h"
#include <assert.h>
#include "ZExpressionNode.h"
#include "ZScriptContext.h"
#include "ZSignatureNode.h"
#include "ZSigParamNode.h"
void ZClassNode::SetName(std::string name) {

	mClassName = name;

}

void ZClassNode::AddMethod(ZMethodNode* node)
{

	mMethods.push_back(node);

}

void ZClassNode::SetMethods(std::vector<ZMethodNode*> methods) {

	mMethods = methods;

}

std::vector<ZMethodNode*> ZClassNode::GetMethods() {

	return mMethods;

}

std::string ZClassNode::GetName() {

	return mClassName;

}

void ZClassNode::AddVars(ZVarsNode* node)
{

	mVars.push_back(node);

}

void ZClassNode::CreateScope() {

	mInstanceScope = new ZContextScope("Class:" + mClassName + " Instance");

}

void ZClassNode::PopulateScope() {

	for (int i = 0; i < mVars.size(); i++) {

		auto vars = mVars[i];

		auto names = vars->GetVars();
		auto type = vars->GetType();

		for (int j = 0; j < names.size(); j++)
		{

			ZContextVar* new_var = new ZContextVar(names[j]->name, type);
			auto def = names[j]->def;
			auto def_exp = def->GetExpression();

			if (def_exp.mElements.size() > 0)
			{


				new_var->SetInt(names[j]->def->Exec(std::vector<ZContextVar*>())->GetIntVal());
				//TODO
			}

			mInstanceScope->RegisterVar(new_var);

		}

	}

}

void ZClassNode::SetVars(std::vector<ZVarsNode*> vars)
{
	mVars = vars;
}

ZClassNode* ZClassNode::CreateInstance(std::string name) {

	ZClassNode* new_cls = new ZClassNode;

	new_cls->SetName(name);
	new_cls->SetMethods(mMethods);
	new_cls->SetVars(mVars);
	new_cls->CreateScope();
	new_cls->PopulateScope();
	new_cls->SetBaseName(this->mClassName);


	return new_cls;

}

ZMethodNode* ZClassNode::FindMethod(std::string name) {

	for (int i = 0; i < mMethods.size(); i++) {

		if (mMethods[i]->GetName() == name)
		{
			return mMethods[i];
		}

	}
	return nullptr;
	assert(false);
	return nullptr;

}

ZContextVar* ZClassNode::CallMethod(std::string name, const std::vector<ZContextVar*>& params)
{
	auto method = FindMethod(name);
	
	auto new_scope = ZScriptContext::CurrentContext->GetScope()->Clone();

	auto sig = method->GetSignature();

	auto pars = sig->GetParams();

	for (int i = 0; i < pars.size(); i++) {

		auto pa = pars[i];
		ZContextVar* v1 = new ZContextVar(pa->GetName(), pa->GetType());
		switch (pa->GetType())
		{
		case VarInt:
			v1->SetInt(params[i]->GetIntVal());
			break;
		case VarFloat:
			v1->SetFloat(params[i]->GetFloatVal());
			break;
		}
		new_scope->RegisterVar(v1);

	}

	//method->SetScope(new_scope);
	//ZScriptContext::CurrentScope = method->GetScope();




	ZScriptContext::CurrentContext->PushScope(new_scope);
	method->SetClassOwner(this);
	auto res = method->Exec({});
	ZScriptContext::CurrentContext->PopScope();
	return res;
}


void ZClassNode::SetBaseName(std::string name) {

	mBaseClassName = name;

}