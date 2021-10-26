#include "CDRomDevice.h"
#include "..\display\ErrorMessageHandler.h"
#ifdef _WIN32
#include <mmsystem.h>
#include "Win95SpecificMCIDeviceHandles.h"
#endif


CDRomDevice::CDRomDevice()
{	
	myCDOpenFlag = false;
	mySpecificMCIDeviceHandles = NULL;
	myInitialisedProperlyFlag = false;

}

void CDRomDevice::Initialise()
{
	if(mySpecificMCIDeviceHandles)
		return;
	
	if(mySpecificMCIDeviceHandles = new Win95SpecificMCIDeviceHandles)
	{
		mySpecificMCIDeviceHandles->OpenCD(NULL);

		mySpecificMCIDeviceHandles->
			OpenSoundBufferForRecording(Win95SpecificMCIDeviceHandles::SourceCD,
										SampleFrequency,
										SampleSize,
										NULL,
										NULL);
		SymbolID state = NoCD;
		int track, frames;
		mySpecificMCIDeviceHandles->GetStatus(track,frames,state);

		myInitialisedProperlyFlag = true;

	}
}

CDRomDevice::~CDRomDevice()
{
	if(mySpecificMCIDeviceHandles)
	{
		delete mySpecificMCIDeviceHandles;
		mySpecificMCIDeviceHandles = NULL;
	}
}




void CDRomDevice::ShutDown()
{
	delete mySpecificMCIDeviceHandles;
	mySpecificMCIDeviceHandles = NULL;
}

int CDRomDevice::GetNextFragment(void)
{
		if(!mySpecificMCIDeviceHandles)
			return -1;

	return mySpecificMCIDeviceHandles->GetNextFragment();

  return 0;
}

void CDRomDevice::SetVolume(double loudness) 
{
	if(!mySpecificMCIDeviceHandles)
			return;

	mySpecificMCIDeviceHandles->SetVolume(loudness);

 
}


bool CDRomDevice::OpenCD(char* cdromName)
{
// tbd: get correct CDrom names in config screen, specify the correct one in
// cdromName, use that one in devicetype.

	if(!mySpecificMCIDeviceHandles)
			return false;

	if(myCDOpenFlag)
		return true;

	 return myCDOpenFlag = mySpecificMCIDeviceHandles->OpenCD(cdromName);



}


void CDRomDevice::CloseCD(void)
{
	if(!mySpecificMCIDeviceHandles)
		return;

	mySpecificMCIDeviceHandles->CloseCD();


}

bool CDRomDevice::GetTrackInfo(void)
{
	if(!mySpecificMCIDeviceHandles)
			return false;

	return mySpecificMCIDeviceHandles->GetTrackInfo();

}


int CDRomDevice::GetTrackCount(void)
{
	if(!mySpecificMCIDeviceHandles)
		return false;

	return mySpecificMCIDeviceHandles->GetTrackCount();

}


int CDRomDevice::GetTrackFrame(int track)
{
	if(!mySpecificMCIDeviceHandles)
		return -1;

	return mySpecificMCIDeviceHandles->GetTrackFrame( track);
}


bool CDRomDevice::PlayCD(int frame, int endFrame)
{
	// this kick starts the whole CD Device do not bother allocating 
	// resources until asked to play something
	if(!mySpecificMCIDeviceHandles)
	{
		Initialise();
	}

	if(mySpecificMCIDeviceHandles)
		return mySpecificMCIDeviceHandles->PlayCD(frame,endFrame);
 
	return false;
}

bool CDRomDevice::GetStatus(int &track, int &frames)
{
	SymbolID state;

	return mySpecificMCIDeviceHandles->GetStatus(track, frames, state);
}


bool CDRomDevice::StopCD(void)
{
	if(mySpecificMCIDeviceHandles)
		return mySpecificMCIDeviceHandles->StopCD();
 
	return false;
}


bool CDRomDevice::PauseCD()
{
	if(mySpecificMCIDeviceHandles)
		return mySpecificMCIDeviceHandles->PauseCD();

	return false;
}


bool CDRomDevice::ResumeCD()
{
	if(mySpecificMCIDeviceHandles)
		return mySpecificMCIDeviceHandles->ResumeCD();

	return false;
}


bool  CDRomDevice::EjectCD()
{
	if(mySpecificMCIDeviceHandles)
	return mySpecificMCIDeviceHandles->EjectCD();

	return false;
}


bool CDRomDevice::CloseCDTray()
{
	if(mySpecificMCIDeviceHandles)
	return mySpecificMCIDeviceHandles->CloseCDTray();

	return false;
}


// The code below is thanks to Thad Froggley and the net
void CDRomDevice::GetCurrentCDFreqencies(int& low, int& mid, int& high)
{
	low = mid = high = 0;
	
	if(mySpecificMCIDeviceHandles)
	{
		mySpecificMCIDeviceHandles->GetNextFragment();

		float x[SampleSize], y[SampleSize];
		float a[SampleSize];

		 int i;


		 // get the two channels of data
		 for(i=0;i<SampleSize;i++) 
		 {
			x[i] = mySpecificMCIDeviceHandles->GetData()[i*2];
			y[i] = mySpecificMCIDeviceHandles->GetData()[i*2+1];
		 }

		 // don't bother with stereo
		 // use y array to recive transformed x data
		FastFourierTransfer(SampleSize,0,x,NULL,y,a);

		// Take the average values of low mid and high frequencies
		// Thanks to Thad for the averaging out code
		high = 0;
		for(i =0; i < 128; i++)
		{
			high += y[i];
		}

		high = high/(2*256*128);


		// mid
		mid = 0;
		for(i =64; i < 192; i++)
		{
			mid += y[i];
		}
		  
		mid = mid/(2*256*(192-64));


		low = 0;
		for(i =128; i < 256; i++)
		{
			low += y[i];
		}

		low = low/(2*256*(256-128));
		
	}


}

int CDRomDevice::IsPowerOfTwo ( unsigned x )
{
    if ( x < 2 )
        return false;

    if ( x & (x-1) )        // Thanks to 'byang' for this cute trick!
        return false;

    return true;
}


unsigned CDRomDevice::NumberOfBitsNeeded ( unsigned PowerOfTwo )
{
    unsigned i;

    if ( PowerOfTwo < 2 )
    {
        fprintf (
            stderr,
            ">>> Error in fftmisc.c: argument %d to NumberOfBitsNeeded is too small.\n",
            PowerOfTwo );

        return 0;
    }

    for ( i=0; ; i++ )
    {
        if ( PowerOfTwo & (1 << i) )
            return i;
    }
}



unsigned CDRomDevice::ReverseBits ( unsigned index, unsigned NumBits )
{
    unsigned i, rev;

    for ( i=rev=0; i < NumBits; i++ )
    {
        rev = (rev << 1) | (index & 1);
        index >>= 1;
    }

    return rev;
}


double CDRomDevice::Index_to_frequency ( unsigned NumSamples, unsigned Index )
{
    if ( Index >= NumSamples )
        return 0.0;
    else if ( Index <= NumSamples/2 )
        return (double)Index / (double)NumSamples;

    return -(double)(NumSamples-Index) / (double)NumSamples;
}


void CDRomDevice::FastFourierTransfer (
    unsigned  NumSamples,
    int       InverseTransform,
    float    *RealIn,
    float    *ImagIn,
    float    *RealOut,
    float    *ImagOut )
{
    unsigned NumBits;    /* Number of bits needed to store indices */
    unsigned i, j, k, alima;
    unsigned BlockSize, BlockEnd;

    double angle_numerator = 2.0 * DDC_PI;
    double tr, ti;     /* temp real, temp imaginary */

    if ( !IsPowerOfTwo(NumSamples) )
    {
        fprintf (
            stderr,
            "Error in fft():  NumSamples=%u is not power of two\n",
            NumSamples );

        exit(1);
    }

    if ( InverseTransform )
        angle_numerator = -angle_numerator;

    if (RealIn == NULL || RealOut == NULL || ImagOut == NULL)
		return;

    NumBits = NumberOfBitsNeeded ( NumSamples );

	if(NumBits == 0)
		return;

    /*
    **   Do simultaneous data copy and bit-reversal ordering into outputs...
    */

    for ( i=0; i < NumSamples; i++ )
    {
        j = ReverseBits ( i, NumBits );
        RealOut[j] = RealIn[i];
        ImagOut[j] = (ImagIn == NULL) ? 0.0 : ImagIn[i];
    }

    /*
    **   Do the FFT itself...
    */

    BlockEnd = 1;
    for ( BlockSize = 2; BlockSize <= NumSamples; BlockSize <<= 1 )
    {
        double delta_angle = angle_numerator / (double)BlockSize;
        double sm2 = sin ( -2 * delta_angle );
        double sm1 = sin ( -delta_angle );
        double cm2 = cos ( -2 * delta_angle );
        double cm1 = cos ( -delta_angle );
        double w = 2 * cm1;
        double ar[3], ai[3];


        for ( i=0; i < NumSamples; i += BlockSize )
        {
            ar[2] = cm2;
            ar[1] = cm1;

            ai[2] = sm2;
            ai[1] = sm1;

            for ( j=i, alima=0; alima < BlockEnd; j++, alima++ )
            {
                ar[0] = w*ar[1] - ar[2];
                ar[2] = ar[1];
                ar[1] = ar[0];

                ai[0] = w*ai[1] - ai[2];
                ai[2] = ai[1];
                ai[1] = ai[0];

                k = j + BlockEnd;
                tr = ar[0]*RealOut[k] - ai[0]*ImagOut[k];
                ti = ar[0]*ImagOut[k] + ai[0]*RealOut[k];

                RealOut[k] = RealOut[j] - tr;
                ImagOut[k] = ImagOut[j] - ti;

                RealOut[j] += tr;
                ImagOut[j] += ti;
            }
        }

        BlockEnd = BlockSize;
    }

    /*
    **   Need to normalize if inverse transform...
    */

    if ( InverseTransform )
    {
        double denom = (double)NumSamples;

        for ( i=0; i < NumSamples; i++ )
        {
            RealOut[i] /= denom;
            ImagOut[i] /= denom;
        }
    }
}

