// Parser.cpp: implementation of the Parser class.
//
//////////////////////////////////////////////////////////////////////

#define _CRT_SECURE_NO_DEPRECATE

#include <cstdlib>
#include <cstring>

#include "Parser.h"
#include "Globals.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


Parser::Parser(bool bDef)
{
	cMyBuffer[0] = '\0';
	iMyBufferPos = 0;
	iMyLineNumber = 1;
	iMyColumnNumber = 0;
	iMyEndOfAtoms = 0;
	iMyNbrOfWarnings = 0;
	bMyEOF = false;
	bMyDef = bDef;
}


Parser::~Parser()
{
	int iDefAtom = 0, iDefVar = 0;
	vector<set<int> *>::iterator sVectorIter;
	vector<bool>::iterator bVectorIter;

	for (sVectorIter=MyAtomVars.begin(); sVectorIter != MyAtomVars.end(); sVectorIter++)
		delete *sVectorIter;

	for (sVectorIter=MyAtomNeighbours.begin(); sVectorIter != MyAtomNeighbours.end(); sVectorIter++)
		delete *sVectorIter;

	for (sVectorIter=MyVarAtoms.begin(); sVectorIter != MyVarAtoms.end(); sVectorIter++)
		delete *sVectorIter;

	for (sVectorIter=MyVarNeighbours.begin(); sVectorIter != MyVarNeighbours.end(); sVectorIter++)
		delete *sVectorIter;

	for (bVectorIter=MyDefAtomUsed.begin(); bVectorIter != MyDefAtomUsed.end(); bVectorIter++, iDefAtom++)
		if (!*bVectorIter)
			delete MyDefAtoms[iDefAtom];

	for (bVectorIter=MyDefVarUsed.begin(); bVectorIter != MyDefVarUsed.end(); bVectorIter++, iDefVar++)
		if (!*bVectorIter)
			delete MyDefVariables[iDefVar];
}



//////////////////////////////////////////////////////////////////////
// Class methods
//////////////////////////////////////////////////////////////////////


void Parser::SyntaxError(const char *cMsg, bool bLineNbr)
{
	if (bLineNbr)
		cerr << "Error in Ln " << iMyLineNumber << ", Col " << iMyColumnNumber << ": " << cMsg << endl;
	else
		cerr << "Error: " << cMsg << endl;
	exit(EXIT_FAILURE);
}


void Parser::SyntaxWarning(const char *cMsg, bool bLineNbr)
{
	if (bLineNbr)
		cerr << "Warning in Ln " << iMyLineNumber << ", Col " << iMyColumnNumber << ": " << cMsg << endl;
	else
		cerr << "Warning: " << cMsg << endl;
	++iMyNbrOfWarnings;
}


void Parser::fillBuffer()
{
	if (!MyFile.eof()) {
		// Reinitialize buffer; this is necessary because of the 'read'-method
		for(int i=0; i < IF_BUFFER_SIZE; i++)
			cMyBuffer[i] = '\0';

		MyFile.read(cMyBuffer, IF_BUFFER_SIZE);
		iMyBufferPos = 0;
	}
	else {
		bMyEOF = true;

		// If end-token was not found yet
		if (iMyEndOfAtoms == 0)
			SyntaxError("Unexpected end of file.", true);
	}
}


void Parser::readIgnoreText()
{
	bool bEndIgnore = false;

	do {
		switch (cMyBuffer[iMyBufferPos]) {
			case '\0':	// End of buffer reached -> refill
				fillBuffer();
				break;
			case ' ':	// Ignore spaces
				++iMyBufferPos;
				++iMyColumnNumber;
				break;
			case 9:		// Ignore tabulators
				++iMyBufferPos;
				++iMyColumnNumber;
				while(iMyColumnNumber%8 != 1)
					++iMyColumnNumber;
				break;
			case '\n':	// Ignore newlines
				++iMyBufferPos;
				++iMyLineNumber;
				iMyColumnNumber = 1;
				break;
			case '%':	// Ignore comments
				do {
					++iMyBufferPos;
					++iMyColumnNumber;
					if (cMyBuffer[iMyBufferPos] == '\0')  // End of buffer reached -> refill
						fillBuffer();
				} while ((cMyBuffer[iMyBufferPos] != '\n') && !bMyEOF);
				// Until newline or end of file
				break;
			default:
				bEndIgnore = true;
		}
	} while (!bEndIgnore && !bMyEOF);
}


char *Parser::readIdentifier()
{
	bool bEndIdentifier = false;
	char cIdentBuffer[MAX_IDENTIFIER_SIZE + 1], *cIdentifier = NULL;
	int iIdentifierSize = 0;

	readIgnoreText();
	do {
		if (cMyBuffer[iMyBufferPos] == '\0')  // End of buffer reached -> refill
				fillBuffer();
		else {
			if (((cMyBuffer[iMyBufferPos] >= 'A') && (cMyBuffer[iMyBufferPos] <= 'Z')) ||
				((cMyBuffer[iMyBufferPos] >= 'a') && (cMyBuffer[iMyBufferPos] <= 'z')) ||
				((cMyBuffer[iMyBufferPos] >= '0') && (cMyBuffer[iMyBufferPos] <= '9')) ||
				(cMyBuffer[iMyBufferPos] == '_') || (cMyBuffer[iMyBufferPos] == ':')) {
				
				// Copy identifier from input file in temporary identifier buffer
				cIdentBuffer[iIdentifierSize++] = cMyBuffer[iMyBufferPos++];
				++iMyColumnNumber;

				if (iIdentifierSize > MAX_IDENTIFIER_SIZE)
					SyntaxError("Maximum length of identifier exceeded.", true);
			}
			else
				bEndIdentifier = true;
		}
	} while (!bEndIdentifier);
	// Until character was found that is not allowed in an identifier

	cIdentBuffer[iIdentifierSize] = '\0';
	if(iIdentifierSize == 0)
		SyntaxError("Illegal or missing identifier.", true);

	// Allocate character array for identifier and copy identifier from the identifier buffer in this array
	cIdentifier = new char[iIdentifierSize + 1];
	if (cIdentifier == NULL)
		writeErrorMsg("Error assigning memory.", "Parser::readIdentifier");

	return strcpy(cIdentifier, cIdentBuffer);
}


bool Parser::readIdentifier(const char *cIdent)
{
	int i = 0;

	readIgnoreText();
	while (cIdent[i] != '\0') {
		if (cMyBuffer[iMyBufferPos] == '\0')  // End of buffer reached -> refill
			fillBuffer();
		if (cMyBuffer[iMyBufferPos] != cIdent[i++])
			return false;
		++iMyBufferPos;
		++iMyColumnNumber;
	}

	return true;
}


int Parser::readArity()
{
	bool bEndArity = false;
	int iArity = 0, iNbrOfDigits = 0;

	readIgnoreText();
	do {
		if (cMyBuffer[iMyBufferPos] == '\0')  // End of buffer reached -> refill
				fillBuffer();
		else {
			if ((cMyBuffer[iMyBufferPos] >= '0') && (cMyBuffer[iMyBufferPos] <= '9')) {
				iArity *= 10;
				iArity += (int)cMyBuffer[iMyBufferPos]-48;
				++iMyBufferPos;
				++iMyColumnNumber;
				++iNbrOfDigits;

				// Maximum number of digits is restricted to 4 (Arity: 1 - 9999)
				if (iNbrOfDigits == 4)
					bEndArity = true;
			}
			else
				bEndArity = true;
		}
	} while (!bEndArity);
	// Until a non-numerical character was found or the maximum number of digits was exceeded

	if(iArity == 0)
		SyntaxError("Illegal arity number.", true);

	return iArity;
}


void Parser::readVarDefs()
{
	char *cIdentifier = NULL, cErrorMsg[MAX_IDENTIFIER_SIZE + 32];
	vector<char *>::iterator VectorIter;

	do {
		cIdentifier = readIdentifier();

		// Check whether the variable is already defined
		VectorIter = MyDefVariables.begin();
		while ((VectorIter != MyDefVariables.end()) && (strcmp(*VectorIter, cIdentifier) != 0))
			++VectorIter;

		// If the variable is not defined yet
		if (VectorIter == MyDefVariables.end()) {
			// Store the variable identifier
			MyDefVariables.push_back(cIdentifier);

			// Initialize usage of the variable
			MyDefVarUsed.push_back(false);
		}
		else {
			strcpy(cErrorMsg, "Variable \"");
			strcat(cErrorMsg, cIdentifier);
			strcat(cErrorMsg, "\" is already defined.");
			SyntaxError(cErrorMsg, true);
		}
	} while (readSeparator());
}


void Parser::readAtomDefs()
{
	char *cIdentifier = NULL, cErrorMsg[MAX_IDENTIFIER_SIZE + 32];
	vector<char *>::iterator VectorIter;

	do {
		cIdentifier = readIdentifier();

		// Check whether the atom is already defined
		VectorIter = MyDefAtoms.begin();
		while ((VectorIter != MyDefAtoms.end()) && (strcmp(*VectorIter, cIdentifier) != 0))
			++VectorIter;

		// If the atom is not defined yet
		if (VectorIter == MyDefAtoms.end())
			// Store the variable identifier
			MyDefAtoms.push_back(cIdentifier);
		else {
			strcpy(cErrorMsg, "Atom \"");
			strcat(cErrorMsg, cIdentifier);
			strcat(cErrorMsg, "\" is already defined.");
			SyntaxError(cErrorMsg, true);
		}

		// Read separator between atom identifier and arity
		readIgnoreText();
		if(!readIdentifier("/"))
			SyntaxError("Illegal character.", true);
		
		// Read arity of the actual atom
		MyArities.push_back(readArity());

		// Initialize usage of the atom
		MyDefAtomUsed.push_back(false);
	} while (readSeparator());
}


bool Parser::readOpenDef()
{
	return readIdentifier("<");
}


void Parser::readCloseDef()
{
	if(!readIdentifier(">"))
		SyntaxError("Illegal character.", true);
}


void Parser::readOpenBody()
{
	if(!readIdentifier("("))
		SyntaxError("Illegal character.", true);
}


void Parser::readCloseBody()
{
	if(!readIdentifier(")"))
		SyntaxError("Illegal character.", true);
}


bool Parser::readSeparator()
{
	return readIdentifier(",");
}


void Parser::readEnd()
{
	if(readIdentifier("."))
		// Remember the line number where the end-token was found for error messages
		iMyEndOfAtoms = iMyLineNumber;
	else
		SyntaxError("Illegal character.", true);
}


void Parser::readDefinitions()
{
	bool bVarDef, bAtomDef;

	// While a definition block '<...>' can be found
	while(readOpenDef()) {
		bVarDef = bAtomDef = false;

		if(readIdentifier("def")) {
			if(readIdentifier("Var"))  // Read "defVar" identifier
				bVarDef = true;
			else
				if(readIdentifier("Rel"))  // Read "defRel" identifier
					bAtomDef = true;
				else
					SyntaxError("Unknown identifier.", true);
		}
		else
			SyntaxError("Unknown identifier.", true);

		readIgnoreText();
		// Check whether the definition identifier is followed by a colon
		if(!readIdentifier(":")) 
			SyntaxError("Illegal character.", true);

		if(bVarDef)
			readVarDefs();  // Read variable definitions
		else
			if(bAtomDef)
				readAtomDefs();  // Read relation definitions

		readCloseDef();
	}
}


void Parser::readAtom()
{
    char *cIdentifier = NULL;
	vector<char *>::iterator VectorIter;
	set<int> *Set = NULL;
	char cErrorMsg[MAX_IDENTIFIER_SIZE + 64];
	int iDefAtom = 0, iArity = 0;

	cIdentifier = readIdentifier();

	// Check if the same atom identifier was already used before
	for (VectorIter=MyAtoms.begin(); VectorIter != MyAtoms.end(); VectorIter++)
		if (strcmp(*VectorIter, cIdentifier) == 0) {
			strcpy(cErrorMsg, "Atom identifier \"");
			strcat(cErrorMsg, cIdentifier);
			strcat(cErrorMsg, "\" occurs the second time.");
			SyntaxError(cErrorMsg, true);
		}

	if (bMyDef) {
		// Check whether the atom is defined
		VectorIter=MyDefAtoms.begin();
		while ((VectorIter != MyDefAtoms.end()) && (strcmp(*VectorIter, cIdentifier) != 0)) {
			++VectorIter;
			++iDefAtom;
		}

		// If the atom is not defined
		if (VectorIter == MyDefAtoms.end()) {
			strcpy(cErrorMsg, "Undefined atom \"");
			strcat(cErrorMsg, cIdentifier);
			strcat(cErrorMsg, "\".");
			SyntaxError(cErrorMsg, true);
		}

		// Update atom usage
		MyDefAtomUsed[iDefAtom] = true;

		// Store the atom identifier
		MyAtoms.push_back(MyDefAtoms[iDefAtom]);
		delete cIdentifier;
	}
	else
		// Store the atom identifier
		MyAtoms.push_back(cIdentifier);

	// Allocate a set of variables for the atom
	Set = new set<int>;
	if (Set == NULL)
		writeErrorMsg("Error assigning memory.", "Parser::readAtom");
	MyAtomVars.push_back(Set);

	// Allocate a set of neighbours for the atom
	Set = new set<int>;
	if (Set == NULL)
		writeErrorMsg("Error assigning memory.", "Parser::readAtom");
	MyAtomNeighbours.push_back(Set);

	// Read the atom body
	readOpenBody();
	do {
		readVariable();
		++iArity;
	} while (readSeparator());
	readCloseBody();

	// Check whether the number of arguments equals the defined arity
	if (bMyDef && (MyArities[iDefAtom] != iArity))
		SyntaxError("Illegal number of arguments.", true);
}


void Parser::readVariable()
{
	int iVar, iDefVar, iAtom;
	char *cIdentifier = NULL;
	vector<char *>::iterator VectorIter;
	set<int>::iterator SetIter;
	set<int> *Set = NULL;
	pair<set<int>::iterator, bool> SetInsertRet;
	char cErrorMsg[(2*MAX_IDENTIFIER_SIZE) + 64];

	iAtom = (int)MyAtoms.size()-1;
	cIdentifier = readIdentifier();

	// Check if the same variable was already used before
	iVar = iDefVar = 0;
	VectorIter = MyVariables.begin();
	while ((VectorIter != MyVariables.end()) && (strcmp(*VectorIter, cIdentifier) != 0)) {
		++VectorIter;
		++iVar;
	}

	// If the variable did not occur previously
    if (VectorIter == MyVariables.end()) {

		if (bMyDef) {
			// Check whether the variable is defined
			VectorIter = MyDefVariables.begin();
			while ((VectorIter != MyDefVariables.end()) && (strcmp(*VectorIter, cIdentifier) != 0)) {
				++VectorIter;
				++iDefVar;
			}

			// If the variable is not defined
			if (VectorIter == MyDefVariables.end()) {
				strcpy(cErrorMsg, "Undefined variable \"");
				strcat(cErrorMsg, cIdentifier);
				strcat(cErrorMsg, "\".");
				SyntaxError(cErrorMsg, true);
			}

			// Update variable usage
			MyDefVarUsed[iDefVar] = true;

			// Store the variable identifier
			MyVariables.push_back(MyDefVariables[iDefVar]);
			delete cIdentifier;
		}
		else
			// Store the variable identifier
			MyVariables.push_back(cIdentifier);

		// Allocate a set of atoms for the variable
		Set = new set<int>;
		if (Set == NULL)
			writeErrorMsg("Error assigning memory.", "Parser::readVariable");
		MyVarAtoms.push_back(Set);

		// Allocate a set of neighbours for the variable
		Set = new set<int>;
		if (Set == NULL)
			writeErrorMsg("Error assigning memory.", "Parser::readVariable");
		MyVarNeighbours.push_back(Set);
	}

	// Add variable to the set of variables of the actual atom
    SetInsertRet = MyAtomVars[iAtom]->insert(iVar);
	// Write syntax warning if the variable occurs multiple times in the actual atom
	if (SetInsertRet.second == false) {
		strcpy(cErrorMsg, "Multiple occurrences of variable \"");
		strcat(cErrorMsg, cIdentifier);
		strcat(cErrorMsg, "\" in atom \"");
		strcat(cErrorMsg, MyAtoms.back());
		strcat(cErrorMsg, "\".");
		SyntaxWarning(cErrorMsg, true);
	}

	// Update the neighbourhood relation between the actual atom and the atoms already stored before
	for (SetIter=MyVarAtoms[iVar]->begin(); SetIter != MyVarAtoms[iVar]->end(); SetIter++)
		if (*SetIter != iAtom) {
			MyAtomNeighbours[*SetIter]->insert(iAtom);
			MyAtomNeighbours[iAtom]->insert(*SetIter);
		}

	// Update the neighbourhood relation between the actual variable and the variables already stored before
	for (SetIter=MyAtomVars[iAtom]->begin(); SetIter != MyAtomVars[iAtom]->end(); SetIter++)
		if (*SetIter != iVar) {
			MyVarNeighbours[*SetIter]->insert(iVar);
			MyVarNeighbours[iVar]->insert(*SetIter);
		}

	// Add actual atom to the set of atoms of the variable
 	MyVarAtoms[iVar]->insert(iAtom);
}


void Parser::parseFile(const char *cNameOfFile)
{
	int iDefVar = 0, iDefAtom = 0;
	char cErrorMsg[96], cLineNumber[(sizeof(int)*8)+1];
	vector<bool>::iterator bVectorIter;
	vector<char *>::iterator cVectorIter;

	// Open input file stream
	MyFile.open(cNameOfFile, ios::in);

	// Check if file opening was successful
	if(!MyFile.is_open())
		SyntaxError("Error opening file.");

	// Read definitions of variables and relations
	readDefinitions();

	// Delete all definition informations
	if (!bMyDef) {
		for (cVectorIter=MyDefAtoms.begin(); cVectorIter != MyDefAtoms.end(); cVectorIter++)
			delete *cVectorIter;
		for (cVectorIter=MyDefVariables.begin(); cVectorIter != MyDefVariables.end(); cVectorIter++)
			delete *cVectorIter;
		MyDefAtoms.clear();
		MyDefVariables.clear();
		MyArities.clear();
		MyDefAtomUsed.clear();
		MyDefVarUsed.clear();
	}

	// Read atoms until the separator cannot be found
	do {
		readAtom();
	} while(readSeparator());
	// Read the end-token
	readEnd();
	// Read the text after the end-token
	readIgnoreText();

	// Close input file stream
	MyFile.close();

	// Write syntax warning if EOF does not appear during reading the ignore text, i.e., there appears
	// some text after the end-token was found
	if(!bMyEOF) {
		strcpy(cErrorMsg, "Illegal character after termination symbol \".\" in line ");
		strcat(cErrorMsg, uitoa(iMyEndOfAtoms, cLineNumber));
		strcat(cErrorMsg, ".");
		SyntaxWarning(cErrorMsg, true);
	}

	if (bMyDef) {
		// Write a syntax warning if a defined atom was not used
		for (bVectorIter = MyDefAtomUsed.begin(); bVectorIter != MyDefAtomUsed.end(); bVectorIter++, iDefAtom++)
			if (!*bVectorIter) {
				strcpy(cErrorMsg, "Atom \"");
				strcat(cErrorMsg, MyDefAtoms[iDefAtom]);
				strcat(cErrorMsg, "\" is not used.");
				SyntaxWarning(cErrorMsg);
			}

		// Write a syntax warning if a defined variable was not used
		for (bVectorIter = MyDefVarUsed.begin(); bVectorIter != MyDefVarUsed.end(); bVectorIter++, iDefVar++)
			if (!*bVectorIter) {
				strcpy(cErrorMsg, "Variable \"");
				strcat(cErrorMsg, MyDefVariables[iDefVar]);
				strcat(cErrorMsg, "\" is not used.");
				SyntaxWarning(cErrorMsg);
			}
	}

	
}


int Parser::getNbrOfAtoms()
{
	return (int)MyAtoms.size();
}


int Parser::getNbrOfVars()
{
	return (int)MyVariables.size();
}


int Parser::getNbrOfVars(int iAtom)
{
	if (iAtom >= (int)MyAtomVars.size()) {
		writeErrorMsg("Position not available.", "Parser::getNbrOfVars");
		return 0;
	}
	else
		return (int)MyAtomVars[iAtom]->size();
}


int Parser::getNbrOfAtoms(int iVar)
{
	if (iVar >= (int)MyVarAtoms.size()) {
		writeErrorMsg("Position not available.", "Parser::getNbrOfAtoms");
		return 0;
	}
	else
		return (int)MyVarAtoms[iVar]->size();
}


int Parser::getNbrOfAtomNeighbours(int iAtom)
{
	if (iAtom >= (int)MyAtomNeighbours.size()) {
		writeErrorMsg("Position not available.", "Parser::getNbrOfAtomNeighbours");
		return 0;
	}
	else
		return (int)MyAtomNeighbours[iAtom]->size();
}


int Parser::getNbrOfVarNeighbours(int iVar)
{
	if (iVar >= (int)MyVarNeighbours.size()) {
		writeErrorMsg("Position not available.", "Parser::getNbrOfVarNeighbours");
		return 0;
	}
	else
		return (int)MyVarNeighbours[iVar]->size();
}


int Parser::getNbrOfWarnings()
{
	return iMyNbrOfWarnings;
}


char *Parser::getAtom(int iAtom)
{
	if (iAtom >= (int)MyAtoms.size()) {
		writeErrorMsg("Position not available.", "Parser::getAtom");
		return NULL;
	}
	else
		return MyAtoms[iAtom];
}


char *Parser::getVariable(int iVar)
{
	if (iVar >= (int)MyVariables.size()) {
		writeErrorMsg("Position not available.", "Parser::getVariable");
		return NULL;
	}
	else
		return MyVariables[iVar];
}


int Parser::getNextAtomVar(bool bReset)
{
	static vector<set<int> *>::iterator VectorIter;
	static set<int>::iterator SetIter;
	static bool bFirstCall = true;

	// Initialize resp. reset the iterator
	if(bFirstCall || bReset) {
		VectorIter = MyAtomVars.begin();
		if(VectorIter == MyAtomVars.end())
			writeErrorMsg("Position not available.", "Parser::getNextAtomVar");
		SetIter = (*VectorIter)->begin();
		bFirstCall = false;
	}
	else
		++SetIter;  // Increment the iterator

	while(SetIter == (*VectorIter)->end()) {
		if(++VectorIter == MyAtomVars.end())
			writeErrorMsg("Position not available.", "Parser::getNextAtomVar");
		SetIter = (*VectorIter)->begin();
	}

	return *SetIter;  // Return the next neighbour index
}


int Parser::getNextVarAtom(bool bReset)
{
	static vector<set<int> *>::iterator VectorIter;
	static set<int>::iterator SetIter;
	static bool bFirstCall = true;

	// Initialize resp. reset the iterator
	if(bFirstCall || bReset) {
		VectorIter = MyVarAtoms.begin();
		if(VectorIter == MyVarAtoms.end())
			writeErrorMsg("Position not available.", "Parser::getNextVarAtom");
		SetIter = (*VectorIter)->begin();
		bFirstCall = false;
	}
	else
		++SetIter;  // Increment the iterator

	while(SetIter == (*VectorIter)->end()) {
		if(++VectorIter == MyVarAtoms.end())
			writeErrorMsg("Position not available.", "Parser::getNextVarAtom");
		SetIter = (*VectorIter)->begin();
	}

	return *SetIter;  // Return the next neighbour index
}


int Parser::getNextAtomNeighbour(bool bReset)
{
	static vector<set<int> *>::iterator VectorIter;
	static set<int>::iterator SetIter;
	static bool bFirstCall = true;

	// Initialize resp. reset the iterator
	if(bFirstCall || bReset) {
		VectorIter = MyAtomNeighbours.begin();
		if(VectorIter == MyAtomNeighbours.end())
			writeErrorMsg("Position not available.", "Parser::getNextAtomNeighbour");
		SetIter = (*VectorIter)->begin();
		bFirstCall = false;
	}
	else
		++SetIter;  // Increment the iterator

	while(SetIter == (*VectorIter)->end()) {
		if(++VectorIter == MyAtomNeighbours.end())
			writeErrorMsg("Position not available.", "Parser::getNextAtomNeighbour");
		SetIter = (*VectorIter)->begin();
	}

	return *SetIter;  // Return the next neighbour index
}


int Parser::getNextVarNeighbour(bool bReset)
{
	static vector<set<int> *>::iterator VectorIter;
	static set<int>::iterator SetIter;
	static bool bFirstCall = true;

	// Initialize resp. reset the iterator
	if(bFirstCall || bReset) {
		VectorIter = MyVarNeighbours.begin();
		if(VectorIter == MyVarNeighbours.end())
			writeErrorMsg("Position not available.", "Parser::getNextVarNeighbour");
		SetIter = (*VectorIter)->begin();
		bFirstCall = false;
	}
	else
		++SetIter;  // Increment the iterator

	while(SetIter == (*VectorIter)->end()) {
		if(++VectorIter == MyVarNeighbours.end())
			writeErrorMsg("Position not available.", "Parser::getNextVarNeighbour");
		SetIter = (*VectorIter)->begin();
	}

	return *SetIter;  // Return the next neighbour index
}

