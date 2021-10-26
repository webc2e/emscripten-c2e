// GtkAdapter - make using GTK from C++ easier

#include <vector>

template<class RECEIVER>
class GtkAdapterData
{
public:
	typedef void (RECEIVER::* MemberFunction) ();

	GtkAdapterData(RECEIVER* window, MemberFunction memberFunction)
		: myWindow(window), myMemberFunction(memberFunction)
	{
	}

	static void ActualFunction(GtkWidget *widget, gpointer data)
	{
		GtkAdapterData* AdapterData = (GtkAdapterData*)data;
		((AdapterData->myWindow)->*(AdapterData->myMemberFunction))();

	}

private:
	RECEIVER* myWindow;
	MemberFunction myMemberFunction;
};

template<class RECEIVER>
class GtkAdapter
{
public:
	gpointer GtkAdapt(GtkAdapterData<RECEIVER>::MemberFunction memberFunction)
	{
		GtkAdapterData<RECEIVER>* adapterData = new GtkAdapterData<RECEIVER>(static_cast<RECEIVER*>(this), memberFunction);
		myAdapterDatas.push_back(adapterData);
		return (gpointer)adapterData;
	}

	GtkSignalFunc GtkSignal()
	{
		return GTK_SIGNAL_FUNC(&GtkAdapterData<RECEIVER>::ActualFunction);
	}

	~GtkAdapter()
	{
		for (int i=0; i<myAdapterDatas.size(); ++i)
		{
			delete myAdapterDatas[i];
		}
	}

private:
	vector<GtkAdapterData<RECEIVER>*> myAdapterDatas;
};

