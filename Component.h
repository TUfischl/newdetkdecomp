#pragma once

#if !defined(CLS_COMPONENT)
#define CLS_COMPONENT

#include "Globals.h"

#pragma once
class Component
{
private:
	uint MyId{ 0 };
	string MyName{ "" };
	int MyLabel{ 0 };
	uint MyWeight{ 0 };

public:
	Component(uint id, string name);
	virtual~Component();

	uint getId() { return MyId; }
	string getName() { return MyName; }

	int getLabel() { return MyLabel; }
	void setLabel(int label = 0) { MyLabel = label; }

	void incLabel() { MyLabel++; }
	void decLabel() { MyLabel--; }

	uint getWeidght() { return MyWeight; }
	void setWeight(uint weight) { MyWeight = weight;  }
};

#endif

