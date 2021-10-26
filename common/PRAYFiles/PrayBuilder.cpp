#include "PrayBuilder.h"

#include "../SimpleLexer.h"
#include "../FileFuncs.h"

#include "StringIntGroup.h"
#include "PrayManager.h"
#include "PrayChunk.h"
#include "PrayException.h"

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


void PrayBuilder::munchComment(int& currentTokenType, std::string& currentToken, SimpleLexer& inputLexer)
{
	bool eat;
	while (currentTokenType == SimpleLexer::typeSymbol && currentToken == "(-")
	{
		// Let's eat comments :)
		eat = true;
		while (eat)
		{
			currentTokenType = inputLexer.GetToken(currentToken);
			if (currentTokenType == SimpleLexer::typeFinished)
				eat = false;
			if (currentTokenType == SimpleLexer::typeSymbol)
				if (currentToken == "-)")
					eat = false;
		}
		// Eaten comment - ready to carry on :)
		if (currentTokenType == SimpleLexer::typeFinished)
		{
			myOutputStream << "Unexpected end of file processing comment :(" << std::endl;
			// TODO: yell at Daniel ;-)
			exit(1);
		}
		currentTokenType = inputLexer.GetToken(currentToken);
	}
}

#define EXPECT(a) expect(currentTokenType,currentToken,SimpleLexer::a,inputLexer.GetLineNum())
bool PrayBuilder::expect(int thisType, std::string& thisToken, int expectedType, int linenumber)
{
	if (thisType == expectedType)
		return true;
	std::string wantType, gotType;

	decodeType(thisType,gotType);
	decodeType(expectedType,wantType);
	myOutputStream << "Error on line " << linenumber
		<< ". Expecting " << wantType << ", got " << gotType
		<< " (" << thisToken << ")" << std::endl;
	return false;
}


void PrayBuilder::decodeType(int thisType,std::string& into)
{
	switch(thisType)
	{
	case SimpleLexer::typeFinished:
		into = "End of File";
		break;
	case SimpleLexer::typeError:
		into = "***LEXER ERROR***";
		break;
	case SimpleLexer::typeString:
		into = "Quoted String";
		break;
	case SimpleLexer::typeNumber:
		into = "Numerical Value";
		break;
	case SimpleLexer::typeSymbol:
		into = "Symbol or keyword";
		break;
	default:
		into = "Unknown type";
	}
}


PrayBuilder::PrayBuilder(std::string sourceFile, std::string destFile)
{
	mySuccessfulBuild = false;

	myOutputStream.clear();

	myOutputStream << "Embedded PRAYBuilder now processing..." << std::endl;

	std::ifstream sourceFileStream(sourceFile.c_str());
	if (!sourceFileStream.good())
	{
		myOutputStream << "Oh dear, couldn't open input file " << sourceFile << std::endl;
		myOutput = myOutputStream.str();
		myOutput.resize(myOutputStream.tellp());
		return;
	}

	SimpleLexer inputLexer(sourceFileStream);

	std::string currentChunkName;
	std::string currentChunkType;

	std::string currentToken;
	StringIntGroup sg;

	enum
	{
		lookingChunk,
		lookingTag
	} myState;

	int currentTokenType = inputLexer.GetToken(currentToken);

	myState = lookingChunk;

	munchComment(currentTokenType,currentToken,inputLexer);

	if (!EXPECT(typeString))
	{ 
		myOutput = myOutputStream.str();
		myOutput.resize(myOutputStream.tellp());
		return;
	}
	PrayManager pm;
	pm.SetLanguage(currentToken);

	currentTokenType = inputLexer.GetToken(currentToken);

	std::string chunkName,chunkType;

	while (currentTokenType != SimpleLexer::typeFinished)
	{
		munchComment(currentTokenType,currentToken,inputLexer);
		if (myState == lookingChunk)
		{
			// We are looking for a chunk identifier (Symbol/Token)
			if (!EXPECT(typeSymbol))
			{ 
				myOutput = myOutputStream.str();
				myOutput.resize(myOutputStream.tellp());
				return;
			}
			// Right then, let's think... If "inline" do file chunk, if "chunk" do prep for stringintgroup.

			if (currentToken == "inline")
			{
				myOutputStream << "Parsing inline chunk..." << std::endl;
				// Deal with inline chunk...
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);
				// Expecting a nice Symbol...
				if (!EXPECT(typeSymbol))
				{ 
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				// The symbol should be 4 chars exactly
				if (currentToken.size() != 4)
				{
					myOutputStream << "Expecting chunk symbol type. It was not four chars. " <<
						"PRAYBuilder aborting." << std::endl << "Offending token was " << 
						currentToken << " on line " << inputLexer.GetLineNum() << std::endl;
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				// The symbol is indeed 4 chars.
				chunkType = currentToken;
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);
				if (!EXPECT(typeString))
				{ 
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				// The string is the name of the chunk.
				chunkName = currentToken;
				MakeFilenameSafe(chunkName); // paranoia (in case there's a way of making these files by their names)
				myOutputStream << "Chunk is of type: " << chunkType <<
					" and is called " << chunkName << std::endl;
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);
				if (!EXPECT(typeString))
				{ 
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				// The string is the name of the file.
				std::string filename = currentToken;
				MakeFilenameSafe(filename);
				myOutputStream << "The data for the inline chunk comes from " << filename << std::endl;
				FILE* inlineFile;
				inlineFile = fopen(filename.c_str(),"rb");
				if (inlineFile == NULL)
				{
					myOutputStream << "Oh dear, couldn't open the file " << filename << " :(" << std::endl;
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				// File opened, let's munch in the data...
				fseek(inlineFile,0,SEEK_END);
				int bytes = ftell(inlineFile);
				fseek(inlineFile,0,SEEK_SET);
				uint8* filedata = new uint8[bytes];
				fread(filedata,bytes,1,inlineFile);
				fclose(inlineFile);
				// Let's dump that good old chunk into the file (nice and compressed too :)
				try
				{
					pm.AddChunkToFile(chunkName,chunkType,destFile,bytes,filedata,CompressionFlag);
				}
				catch (PrayException& pe)
				{
					myOutputStream << "Erkleroo, excepted during chunk add :( (" << pe.GetMessage() << ")" << std::endl;
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				catch (...)
				{
					myOutputStream << "Erkleroo, excepted during chunk add :( (Some day I'll put some better error routines in :)" << std::endl;
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				delete [] filedata;
				currentTokenType = inputLexer.GetToken(currentToken);
				continue;
			} // inline
			if (currentToken == "group")
			{
				myOutputStream << "Please wait, preparing StringIntGroup chunk..." << std::endl;
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);
				// Expecting a nice Symbol...
				if (!EXPECT(typeSymbol))
				{ 
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				// The symbol should be 4 chars exactly
				if (currentToken.size() != 4)
				{
					myOutputStream << "Expecting chunk symbol type. It was not four chars. "
						<< "PRAYBuilder aborting." << std::endl << "Offending token was " <<
						currentToken << " on line " << inputLexer.GetLineNum() << std::endl;
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				chunkType = currentToken;
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);
				if (!EXPECT(typeString))
				{ 
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				// The string is the name of the chunk.
				chunkName = currentToken;
				myOutputStream << "Chunk is of type: " << chunkType <<
					" and is called " << chunkName << std::endl;
				currentTokenType = inputLexer.GetToken(currentToken);
				myState = lookingTag;
				continue;
			}

		}
		else if (myState == lookingTag)
		{
			if (currentTokenType == SimpleLexer::typeSymbol)
			{
				// Hit end of chunk, so write it out, and reset state
				std::ostringstream os;
				sg.SaveToStream(os);
				std::string tempStr = os.str();
				const char* str = tempStr.c_str();
				int leng = os.tellp();
				try
				{
					pm.AddChunkToFile(chunkName,chunkType,destFile,leng,(unsigned char *)str,CompressionFlag);
				}
				catch (PrayException& pe)
				{
					myOutputStream << "Erkleroo, excepted during group add :( (" << pe.GetMessage() << ")" << std::endl;
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				catch (...)
				{
					myOutputStream << "Erkleroo, excepted during group add :( (Some day I'll put some better error routines in :)" << std::endl;
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				sg.Clear();
				myState = lookingChunk;
				continue;
			}
			if (currentTokenType == SimpleLexer::typeString)
			{
				// Right then, we have a tag. Format is...
				// "tag" <value>
				// <value> is either a number, a string, or a symbol.
				std::string thisTag = currentToken;
				currentTokenType = inputLexer.GetToken(currentToken);
				munchComment(currentTokenType,currentToken,inputLexer);

				std::string tempString;
				int bytes;
				char* filedata;

				std::string t;
				std::string is;
				int i;
				switch(currentTokenType)
				{
				case SimpleLexer::typeString:
					// We have a string->string mapping
					if (sg.FindString(thisTag,t))
						myOutputStream << "Warning, redefinition of " << thisTag << " from " << t << " to " << currentToken << std::endl;
					sg.AddString(thisTag,currentToken);
					break;
				case SimpleLexer::typeNumber:
					// We have a string->int mapping
					if (sg.FindInt(thisTag,i))
						myOutputStream << "Warning, redefinition of " << thisTag << " from " << i << " to " << currentToken << std::endl;
					sg.AddInt(thisTag,atoi(currentToken.c_str()));
					break;
				case SimpleLexer::typeSymbol:
					{
						// We have a string->file (I.E. string) mapping
						if (sg.FindString(thisTag,is))
							myOutputStream << "Warning redefinition of file tag " << thisTag << std::endl;
						if (currentToken != "@")
						{
							myOutputStream << "The only valid symbol in this context is \"@\". Error on line " << inputLexer.GetLineNum() << std::endl;
							myOutput = myOutputStream.str();
							myOutput.resize(myOutputStream.tellp());
							return;
						}
						currentTokenType = inputLexer.GetToken(currentToken);
						munchComment(currentTokenType,currentToken,inputLexer);
						if (!EXPECT(typeString))
						{ 
							myOutput = myOutputStream.str();
							myOutput.resize(myOutputStream.tellp());
							return;
						}
						std::string filename = currentToken;
						MakeFilenameSafe(filename);
						myOutputStream << "Trying to add a file mapping to " << filename << std::endl;
						FILE* inlineFile;
						inlineFile = fopen(filename.c_str(),"rb");
						if (inlineFile == NULL)
						{
							myOutputStream << "Oh dear, couldn't open the file " << filename << " :(" << std::endl;
							myOutput = myOutputStream.str();
							myOutput.resize(myOutputStream.tellp());
							return;
						}
						// File opened, let's munch in the data...
						fseek(inlineFile,0,SEEK_END);
						bytes = ftell(inlineFile);
						fseek(inlineFile,0,SEEK_SET);
						filedata = new char[bytes];
						fread(filedata,bytes,1,inlineFile);
						fclose(inlineFile);

						tempString.assign(filedata,bytes);
						delete [] filedata;
						sg.AddString(thisTag,tempString);
						break;
					}
				default:
					myOutputStream << "Expecting number,symbol or string. Error on line " << inputLexer.GetLineNum() << std::endl;
					myOutput = myOutputStream.str();
					myOutput.resize(myOutputStream.tellp());
					return;
				}
				currentTokenType = inputLexer.GetToken(currentToken);
				continue;
			}
		}
		else
		{
			myOutputStream << "Unknown lookahead state :(" << std::endl;
			myOutput = myOutputStream.str();
			myOutput.resize(myOutputStream.tellp());
			return;
		}
	}

	if (myState == lookingTag)
	{
		
		std::ostringstream os;
		sg.SaveToStream(os);
		std::string tempStr = os.str();
		const char* str = tempStr.c_str();
		int leng = os.tellp();
		try
		{
			pm.AddChunkToFile(chunkName,chunkType,destFile,leng,(unsigned char *)str,CompressionFlag);
		}
		catch (PrayException& pe)
		{
			myOutputStream << "Erkleroo, excepted during group add :( (" << pe.GetMessage() << ")" << std::endl;
			myOutput = myOutputStream.str();
			myOutput.resize(myOutputStream.tellp());
			return;
		}
		catch (...)
		{
			myOutputStream << "Erkleroo, excepted during group add :( (Some day I'll put some better error routines in :)" << std::endl;
			myOutput = myOutputStream.str();
			myOutput.resize(myOutputStream.tellp());
			return;
		}
		sg.Clear();
		os.clear();
		myState = lookingChunk;
	}
	mySuccessfulBuild = true;
	myOutputStream << std::endl << std::endl << "***END***" << std::endl << "\0";
	
	myOutput = myOutputStream.str();
	myOutput.resize(myOutputStream.tellp());
	myOutputStream.clear();
	pm.GarbageCollect(true);

}

bool PrayBuilder::Successful()
{
	return mySuccessfulBuild;
}

std::string PrayBuilder::Output()
{
	return myOutput;
}

