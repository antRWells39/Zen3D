#pragma once
#include <vector>
#include <initializer_list>

class ZContextVar;
class ZClassNode;

class ZScriptNode
{
public:

	virtual ZContextVar* Exec(const std::vector<ZContextVar*>& params);
	ZClassNode* GetClassOwner() {
		return mClass;
	}
	void SetClassOwner(ZClassNode* cls)
	{
		mClass = cls;
	}


public:
	
	std::vector<ZScriptNode*> mNodes;
	ZClassNode* mClass;

};

