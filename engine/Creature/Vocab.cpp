#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Vocab.h"
#include "../Maths.h"	// for Rnd()
#include "CreatureConstants.h"

#include <vector>
std::vector<std::string> Vocab::ourWordSections;
int Vocab::ourNumWordChunks = 0;

const float PRONUNTHRES1 = 0.31f;    // thresholds: how well wd must be learned b4
const float PRONUNTHRES2 = 0.8f;    // using a more sophisticated pronunciation
const float CONFIRM = 0.14f; // how much to add to weight when i/p matches myVocab
const float DENY = 0.1f;    // how much to subtract if text is new


Vocab::Vocab() { 
	learnedStrength=0.0f; 
}


bool Vocab::SetupCatalogueVector()
{
	ourNumWordChunks = theCatalogue.GetArrayCountForTag("vocab_convert_chunks");
	for (int i=0; i<ourNumWordChunks; i++)
	{
		ourWordSections.push_back(theCatalogue.Get("vocab_convert_chunks",i));
	}	
	return true;
}

// fill both in & out slots with this text
void Vocab::InitBabyWord(const std::string& text)
{
	platonicWord = text;
	outWord = text;
	learnedStrength = 0.0f;
}



void Vocab::InitWord(const std::string& text, int age, bool perfect)
{
	platonicWord = text;
		
	if (perfect)
	{
		learnedStrength = 1.0f;
		outWord = text;
	}
	else
	{
		//  Baby speak doesn't get unlearned instantly!:
		learnedStrength = age == AGE_BABY ? 0 : age == AGE_CHILD ? 
			PRONUNTHRES1-CONFIRM : PRONUNTHRES2-CONFIRM;

		if (learnedStrength>=PRONUNTHRES2)
			outWord = text;
		else if (learnedStrength>=PRONUNTHRES1)
			DoReasonablyGoodInfantSpeak();
    	else if( learnedStrength > 0)
			DoReallyTerribleInfantSpeak();
	}
}




// Convert a word from myVocab[].platonicWord form to a very babyish mispronunciation
// (sort of "Da-Da")
void Vocab::DoReallyTerribleInfantSpeak()
{
    std::string srcString;
	std::string dstString;
	std::string endString;
    static std::string vowels(theCatalogue.Get("vocab_vowels"));
    static std::string consonants(theCatalogue.Get("vocab_consonants"));
	std::string tempString;


    // rule is:
    // keep the 1st part (if wd starts with vowel, keep up to 1st cons;
    // or vice versa). If word is long (prob multisyllabic, repeat that bit.
    // eg. "lift" -> "li"; "elevator" -> "el-el":
	outWord = platonicWord;
	int firstConsonantId = platonicWord.find_first_of(consonants);
	int firstVowelId = platonicWord.find_first_of(vowels);

	int amountOfInWordToUse = firstVowelId==0 ?
		firstConsonantId+1 : firstVowelId+1;

	outWord.resize(amountOfInWordToUse);			  // "el"

    if  (((platonicWord.length()>5 && outWord.length()<5) ||// if word is polysyllabic
         (outWord.length()<3))) {                     // or baby form is short
		outWord.append(platonicWord);						  // duplicate it ("-el")
		outWord.resize(amountOfInWordToUse*2);
    }
}




// Convert a word to a moderately mispronounced form
// (phonic mistakes - "this"->"dis")

void Vocab::DoReasonablyGoodInfantSpeak() {	
	// Rule is: look for any letter groups that can be converted to
    // mispronounced/babyish forms, eg. "th"->"d"
    outWord = platonicWord;								// start perfectly
	// If you look at "vocab constructs.catalogue", the following should become clear.
	for (int i=0; i<ourNumWordChunks; i=i+2) {				// for each letter group:
		std::string syllableToFind(ourWordSections[i]);
		std::string syllableToReplaceWith(ourWordSections[i+1]);
        int syllableId = outWord.find(syllableToFind);	// find it if poss

        if (syllableId>=0) {							// if exists:
			outWord.erase(syllableId, syllableToFind.length());
			outWord.insert(syllableId, syllableToReplaceWith);
            return;										// no further changes
        }
    }													// repeat for all groups
}



// You've HEARD a word on a blackboard. Update your myVocab as follows:
// Determine which myVocab entry is involved (depends on which picture
// the blackboard is showing). If this word's spinal dendrite's weight
// is <=0, then copy the (new) text into the myVocabulary. Otherwise, if
// the text matches the myVocab entry, strengthen the weighting, or if
// not, weaken it.
// 18/4/94: HearWord() is also called when a creature speaks, so that creatures
// can learn language from each other
void Vocab::HearWord(std::string wordHeard) {
    float prevStrength = learnedStrength;

    // Does the text match that stored in the myVocab?
    // If so, increase the strength of the spinal dendrite
    if  (!wordHeard.compare(platonicWord)) {
        learnedStrength = BoundedAdd(learnedStrength, CONFIRM);

        // if learned word better than 1st threshold, change from saying it
        // in "da-da" form to phonic mispronunciation;
        // if better now than 2nd threshold, become able to speak it accurately
        if  (learnedStrength>=PRONUNTHRES2)
            outWord = platonicWord;
		else if  (learnedStrength>=PRONUNTHRES1)
			DoReasonablyGoodInfantSpeak();
 
    } else {
        // If not, decrease the strength (unlearn):

		// this should only be possible in c3
		// if a user changes words in the catalogue and
		// imports a previouly taught creature
		
		learnedStrength = BoundedSub(learnedStrength, DENY);
    }


    // If strength of word has dropped to zero, replace the myVocab entry
    // with the new text
    if  (learnedStrength<=0.0f) {
        platonicWord = wordHeard;
		// o/p word is badly mispronounced:
		DoReallyTerribleInfantSpeak();
        learnedStrength = CONFIRM;	// initial strength
    }
}

