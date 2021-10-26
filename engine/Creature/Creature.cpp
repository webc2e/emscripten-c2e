#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Creature.h"
#include "Genome.h"
#include "../World.h"		// for pointer!
#include "../App.h"
#include "../Display/ErrorMessageHandler.h"
#include "../Agents/AgentHandle.h"

#include "Brain/Brain.h"
#include "Biochemistry/Biochemistry.h"
#include "LinguisticFaculty.h"
#include "MotorFaculty.h"
#include "SensoryFaculty.h"
#include "ReproductiveFaculty.h"
#include "ExpressiveFaculty.h"
#include "LifeFaculty.h"
#include "MusicFaculty.h"

#ifndef _WIN32
// VK_* defs
#include "../unix/KeyScan.h"
#endif


//CREATURES_IMPLEMENT_SERIAL( Creature )




bool Creature::PrepareForAgeing(int age)
{
	CAOSVar varNextAge;
	varNextAge.SetInteger(age);
	CallScript(SCRIPT_CALLBACK_AGEING, varNextAge, INTEGERZERO);
	return true;
}

	
	// Keep track of the position of the currently required hotspot on IT
// (when obj is attempting to Activate etc. IT
void Creature::TrackIt() {
	if (myIt.IsValid()) {				// if an IT is defined
        myItPosition.x = myIt.GetAgentReference().WhereShouldCreaturesPressMe().x;
        myItPosition.y = myIt.GetAgentReference().WhereShouldCreaturesPressMe().y;
    }
}

// blame Alima for these...
// when a creature ages it gets removed as the tracking object
// but if this flag is set then it will be reset
void Creature::RememberThatYouAreBeingTracked(bool amibeingtracked)
{
	myBeingTrackedFlag = amibeingtracked;
}




int theCreatureCount = 0;

int GetCreatureCount()
{
	return theCreatureCount;
}

int Creature::ourNextUpdateTickOffsetToUse = 0;

void Creature::HandlePickup(Message* Msg)
{
	// Simple tests..
	if (Msg->GetFrom().GetType() == CAOSVar::typeAgent)
	{
		if (Msg->GetFrom().GetAgent().IsPointerAgent())
		{
			// Pickup is from Pointer....
			if ((theApp.GetCreaturePickupStatus() == 1) ||
				((theApp.GetCreaturePickupStatus() == 2) && (!theApp.GetInputManager().IsKeyDown(VK_SHIFT))) ||
				((theApp.GetCreaturePickupStatus() == 3) && ((!theApp.GetInputManager().IsKeyDown(VK_SHIFT)) || !Music()->SelectableByUser())))
			{
				// Convert pickup message to hold hands message
				HandleStartHoldHands(Msg);
				return;
				
			}
		}
	}

	base::HandlePickup(Msg);
}




void Creature::SpeakSentence(std::string& thisSentence)
{
	myLinguisticFaculty->Say( thisSentence );
}





void Creature::OwnTrash()
{
	_ASSERT(!myGarbaged);

	for(int i = 0; i < noOfFaculties; i++)
	{
		myFaculties[i]->Trash();
	}

	for(int j = 0; j < noOfFaculties; j++)
	{
		delete myFaculties[j];
		myFaculties[j] = NULL;
	}
}


void Creature::Trash()
{
	_ASSERT(!myGarbaged);

	OwnTrash();

	// This must be the last line in the function
	base::Trash();
}



void Creature::OwnUpdate()
{
	_ASSERT(!myGarbaged);

	// not our tick?
	if ((myUpdateTick%4)!=myUpdateTickOffset)
		return;

	if (!Life()->GetWhetherDead()) {
		TrackIt();
		for (int i=0; i<noOfFaculties; i++) 
		{
			if (myFaculties[i]->IsActive())
			{
				myFaculties[i]->Update();
			}
		}

		// update crowdedness:
		myCrowdedLocus = 0.0f;
		float value;
		if (theApp.GetWorld().GetMap().GetRoomPropertyMinusMyContribution(AgentHandle(*this), value))
			myCrowdedLocus = value;
	}
}


void Creature::Update() 
{    
	_ASSERT(!myGarbaged);

	base::Update();
	OwnUpdate();
}



float Creature::GetDriveLevel(int i) 
{
	_ASSERT(!myGarbaged);
	if (i<0 || i>=NUMDRIVES)
		return -1.0f;
	return myDriveLoci[i];
}

int Creature::GetHighestDrive() 
{
	_ASSERT(!myGarbaged);

	int d = 0;
	for	(int i=0; i<NUMDRIVES; i++)
	{
		if(myDriveLoci[i] > myDriveLoci[d])
			d = i;
	}
	return d;
}


bool Creature::OwnWrite(CreaturesArchive &archive) const
{
	_ASSERT(!myGarbaged);

	// these Creature:: members get serialised out by skeletal creature
	// so if we're one of those really don't do them here:
	if ((myAgentType & AgentHandle::agentSkeletalCreature) != AgentHandle::agentSkeletalCreature )
	{
		archive << myMotherMoniker;		// to identify my mother
		archive << myFatherMoniker;		// to identify my father

		archive << myBeingTrackedFlag;

		archive << myIt;				// object of this creature's attention, NULL if none
		archive << myIntrospectiveFlag;	// for extraspective actions like GO EAST and GO WEST
		archive << myItPosition;
	}

	archive.WriteFloatRefTarget( myInvalidLocus );
	int o;
	for ( o=0; o<NUMDRIVES; o++) {
		archive.WriteFloatRefTarget( myDriveLoci[o] );
	}

	archive.WriteFloatRefTarget( myConstantLocus );
	archive.WriteFloatRefTarget( myAirQualityLocus );
	archive.WriteFloatRefTarget( myCrowdedLocus );

	int k;
	for ( k=0; k<NUM_FLOATING_LOCI; k++) {
		archive.WriteFloatRefTarget( myFloatingLoci[k] );
	}

	//Save 4 (Biochemistry for last)
	int i;
	for ( i=0; i<noOfFaculties; i++)
		if( i != 4 )
			archive << myFaculties[i];
	archive << myFaculties[4];

	return true;
}

bool Creature::OwnRead(CreaturesArchive &archive)
{
	_ASSERT(!myGarbaged);

	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		// these Creature:: members get serialised out by skeletal creature
		// but if we're not a skeletal creature we should do them now:
		if ((myAgentType & AgentHandle::agentSkeletalCreature) != AgentHandle::agentSkeletalCreature )
		{
			archive >> myMotherMoniker;		// to identify my mother
			archive >> myFatherMoniker;		// to identify my father

			archive >> myBeingTrackedFlag;

			archive >> myIt;				// object of this creature's attention, NULL if none
			archive >> myIntrospectiveFlag;	// for extraspective actions like GO EAST and GO WEST
			archive >> myItPosition;
		}

		archive.ReadFloatRefTarget( myInvalidLocus );

		int o;
		for ( o=0; o<NUMDRIVES; o++) {
			archive.ReadFloatRefTarget( myDriveLoci[o] );
		}
		archive.ReadFloatRefTarget( myConstantLocus );
		archive.ReadFloatRefTarget( myAirQualityLocus );
		archive.ReadFloatRefTarget( myCrowdedLocus );

		int k;
		for ( k=0; k<NUM_FLOATING_LOCI; k++) {
			archive.ReadFloatRefTarget( myFloatingLoci[k] );
		}

		//Save 4 (Biochemistry for last)
		int i;
		for ( i=0; i<noOfFaculties; i++)
			if( i != 4 )
				archive >> myFaculties[i];
		archive >> myFaculties[4];

		mySensoryFaculty		= (SensoryFaculty*)		(myFaculties[0]);
		myBrain					= (Brain*)				(myFaculties[1]);
		myMotorFaculty			= (MotorFaculty*)		(myFaculties[2]);
		myLinguisticFaculty		= (LinguisticFaculty*)	(myFaculties[3]);
		myBiochemistry			= (Biochemistry*)		(myFaculties[4]);
		myReproductiveFaculty	= (ReproductiveFaculty*)(myFaculties[5]);
		myExpressiveFaculty		= (ExpressiveFaculty*)	(myFaculties[6]);
		myMusicFaculty			= (MusicFaculty*)		(myFaculties[7]);
		myLifeFaculty			= (LifeFaculty*)		(myFaculties[8]);

		// set up creature callback in faculties:
		for (i=0; i<noOfFaculties; i++) 
		{
			myFaculties[i]->SetCallback(&(mySelf.GetAgentFacultyInterfaceReference()), &(mySelf.GetCreatureFacultyInterfaceReference()));
		}

		// resolve known agents in this world and time stamp (must be done after life
		// cos it is called within)
		mySensoryFaculty->ResolveFriendAndFoe();

		// success
		myBrain->RegisterBiochemistry(myBiochemistry->GetChemicalConcs());
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}








bool Creature::Write(CreaturesArchive &archive) const
{
	_ASSERT(!myGarbaged);

	base::Write(archive);

	OwnWrite(archive);

	return true;
}

bool Creature::Read(CreaturesArchive &archive) 
{
	_ASSERT(!myGarbaged);

	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		if(!base::Read(archive))
			return false;

		OwnRead(archive);
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}


void Creature::SetupDefaults()
{
	int i;

	myMotherMoniker = myFatherMoniker = "";	// Mum & Dad will be read from genes
	myIt = NULLHANDLE;

	myConstantLocus = 1.0f;						// constant output emitter locus
	for	(i=0; i<NUM_FLOATING_LOCI; i++)		// recep/emitter loci
		myFloatingLoci[i] = 0.0f;
	for	(i=0; i<NUMDRIVES; i++)
		myDriveLoci[i] = 0.0f;
	myAirQualityLocus = 1.0f;
	myCrowdedLocus = 0.0f;
	myInvalidLocus = 0.0f;

	myUpdateTickOffset = ourNextUpdateTickOffsetToUse;
	ourNextUpdateTickOffsetToUse = (ourNextUpdateTickOffsetToUse+1)%4;

	myMovementStatus=AUTONOMOUS;

	myAgentType |= AgentHandle::agentCreature;
}


///////////////////////////////////////////////////////////////////////////////
//  Initialisation:

// General Initialisation called from the superclasses only
// (when they have their V-table set up right):
void Creature::Init() 
{
	try
	{
		for (int i=0; i<noOfFaculties; i++) 
		{
			myFaculties[i]->SetCallback(&(mySelf.GetAgentFacultyInterfaceReference()), &(mySelf.GetCreatureFacultyInterfaceReference()));
			myFaculties[i]->Init();
		}

		if (!ReadFromGenome())
			myFailedConstructionException = ErrorMessageHandler::Format("creature_error", 0, "Creature::Creature", GetMoniker().c_str());

		Sensory()->PostInit();
	}
	catch (BasicException& e)
	{
		myFailedConstructionException = e.what();
	}
	catch (...)
	{
		myFailedConstructionException = "NLE0012: Unknown exception caught in creature constructor";
	}
}



// Serialisation constructor
Creature::Creature() : base() {
	++theCreatureCount;

	SetupDefaults();									// set defaults
}


// Destructor.
Creature::~Creature()


{
	--theCreatureCount;
}

void Creature::Construct(int family, uint32 id, AgentHandle gene, int gene_index, int sex, int variant)
{
	ASSERT(sex==1 || sex==2);
	ASSERT(variant>=1 && variant<=8);

	++theCreatureCount;
	SetupDefaults();

	myID = id;

	myFaculties[0] = (Faculty*)(mySensoryFaculty		= new SensoryFaculty());
	myFaculties[1] = (Faculty*)(myBrain					= new Brain());
	myFaculties[2] = (Faculty*)(myMotorFaculty			= new MotorFaculty());
	myFaculties[3] = (Faculty*)(myLinguisticFaculty		= new LinguisticFaculty());
	myFaculties[4] = (Faculty*)(myBiochemistry			= new Biochemistry());
	myFaculties[5] = (Faculty*)(myReproductiveFaculty	= new ReproductiveFaculty());
	myFaculties[6] = (Faculty*)(myExpressiveFaculty		= new ExpressiveFaculty());
	myFaculties[7] = (Faculty*)(myMusicFaculty			= new MusicFaculty());
	myFaculties[8] = (Faculty*)(myLifeFaculty			= new LifeFaculty());

	myBrain->RegisterBiochemistry(myBiochemistry->GetChemicalConcs());

	Life()->SetSex(sex);
	Life()->SetVariant(variant);


	// get genome from AgentHandle given:
	GetGenomeStore().MoveSlotFrom(0, gene.GetAgentReference().GetGenomeStore(), gene_index);

	Genome g(GetGenomeStore(), 0, Life()->GetSex(), Life()->GetAge(), Life()->GetVariant());

	// make classifier
	Classifier c(family, g.GetGenus() + 1, g.GetSex());
	myClassifier = c;

	// put stuff in history
	std::string moniker = g.GetMoniker();
	HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
	CreatureHistory& history = historyStore.GetCreatureHistory(moniker);

	history.myGender = g.GetSex();
	history.myVariant = variant;

	ASSERT(g.GetSex() == sex);
}


Creature::Creature(int family, uint32 id, AgentHandle gene, int gene_index, int sex, int variant)
	: base()
{
	Construct(family, id, gene, gene_index, sex, variant);
}


Creature::Creature(int family, uint32 id, AgentHandle gene, int gene_index, int sex, int variant,
	FilePath const& gallery, int numimages, int baseimage, int plane)
	: base(family, -1, -1, id, gallery, numimages, baseimage, plane)
{
	Construct(family, id, gene, gene_index, sex, variant);
}









// Express any genes relevant to current age:
bool Creature::ReadFromGenome()
{
	_ASSERT(!myGarbaged);
	try
	{
		Genome g(GetGenomeStore(), 0, Life()->GetSex(), Life()->GetAge(), Life()->GetVariant());

		for (int i=0; i<noOfFaculties; i++) {
			myFaculties[i]->ReadFromGenome(g);
		}

		// (was in skeleton):
		// Read the (only) GENUS (Header) gene and determine creature's genus
		// and his mum's & dad's names
		g.Reset();
		while ((g.GetGeneType(CREATUREGENE,G_GENUS,NUMCREATURESUBTYPES,SWITCH_AGE))!=FALSE) {
			//Genus = 
			g.GetCodon(0,3);

			char charArray[64];

			int i;
			for ( i = 0; i < 64; ++i)
			{
				char c = g.GetChar();
				charArray[i] = c;
			}

			// used for detecting family members with senses -
			// and no doubt generally useful!
			myMotherMoniker = std::string(charArray, 32).c_str(); 
			myFatherMoniker = std::string(charArray + 32, 32).c_str();
			// c_str() in these lines is to force stripping to trailing zero -
			// remember that std::strings have a length, and can actually contain
			// zero characters in the middle of them.
		}
	}
	catch(Genome::GenomeException& e)
	{
		ErrorMessageHandler::Show(e, std::string("Creature::ReadFromGenome"));
		return false;
	}
	return true;
}




///////////////////////////////////////////////////////////////////////////////
// LOCI FACULTY:

// respond to an enquiry by a biochemical Receptor/Emitter object for its locus address
float* Creature::GetLocusAddress(int type,			// ID relates to RECEPTOR or EMITTER locus?
							 int organ,
							 int tissue,		// tissue containing locus
							 int locus) {		// locus on object
	_ASSERT(!myGarbaged);

	for (int i=0; i<noOfFaculties; i++)
	{
		float* locusPointer = myFaculties[i]->GetLocusAddress(type, organ, tissue, locus);
		if (locusPointer)
			return locusPointer;
	}

	if (organ==ORGAN_CREATURE)
	{
		if (type==RECEPTOR)
		{
			// Receptor loci (locus is modulated according to chemical concentration)
			switch (tissue)
			{
				case TISSUE_CIRCULATORY:
					if (locus>=LOC_FLOATING_FIRST && locus<=LOC_FLOATING_LAST)
						return &myFloatingLoci[locus-LOC_FLOATING_FIRST];
					break;

				case TISSUE_DRIVES:
					if (locus>=LOC_DRIVE0 && locus<LOC_DRIVE0+NUMDRIVES)
						return &myDriveLoci[locus-LOC_DRIVE0];
					break;
			}
		}
		else
		{
 			// Emitter loci (locus is a source of chemical emission)
			switch (tissue)
			{							// according to tissue...
				case TISSUE_CIRCULATORY:
					if (locus>=LOC_FLOATING_FIRST && locus<=LOC_FLOATING_LAST)
						return &myFloatingLoci[locus-LOC_FLOATING_FIRST];
					break;

				case TISSUE_SENSORIMOTOR:
					switch (locus)
					{
						case LOC_CONST:
							return &myConstantLocus;
						case LOC_AIRQUALITY:
							return &myAirQualityLocus;
						case LOC_CROWDEDNESS:
							return &myCrowdedLocus;
					}
					break;

				case TISSUE_DRIVES:
					if (locus>=LOC_DRIVE0 && locus<LOC_DRIVE0+NUMDRIVES)
						return &myDriveLoci[locus-LOC_DRIVE0];
					break;
			}
		}
	}

    // Unrecognised ID.
	return &myInvalidLocus;
}

// this is a helper for CAV at shut down time our norns need to
// fall asleep.  These tiredness and sleepiness values were recommended
// by Helen.  Call this 5 minutes before shutdown.
void Creature::MakeYourselfTired()
{
	_ASSERT(!myGarbaged);

	myBiochemistry->SetChemical(CHEM_TIREDNESS,float(0.8));
	myBiochemistry->SetChemical(CHEM_SLEEPINESS,float(0.6));
}



